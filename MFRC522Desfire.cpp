#include "MFRC522Desfire.h"
#include "crypto/sha256.h"
#include "crypto/hmac.h"
#include "crypto/crc32.h"
#include <vector>

extern "C" {
#include "crypto/aes.h"
}

void debugOutByteArray(byte *data, byte len, std::string text){
#ifdef DEBUG
	std::cout << text;
	for(int i = 0; i < len; i++){
		std::cout << "0x" << std::hex << (int) data[i] << " ";
	}
	std::cout << std::endl;
#endif
}

byte MFRC522Desfire::PCD_Decrypt(DesfireAesKey key, byte *encryptedData, byte encryptedDataLen, byte *initVector, byte *decryptedData, byte decryptedDataLen){
	AES128_CBC_decrypt_buffer(decryptedData, encryptedData, encryptedDataLen, key.keyByte, initVector);
	memcpy(initVector, &encryptedData[encryptedDataLen - AES_KEY_LENGTH], AES_KEY_LENGTH);
    return STATUS_OK;
}

byte MFRC522Desfire::PCD_Encrypt(DesfireAesKey key, byte *decryptedData, byte decryptedDataLen, byte *initVector, byte *encryptedData, byte encryptedDataLen){
	AES128_CBC_encrypt_buffer(encryptedData, decryptedData, decryptedDataLen, key.keyByte, initVector);
	memcpy(initVector, &encryptedData[encryptedDataLen - AES_KEY_LENGTH], AES_KEY_LENGTH);
	return STATUS_OK;
}

byte MFRC522Desfire::PCD_Xor(byte *inOutBuf, byte bufLen, byte *in2){
	for (int i=0; i<bufLen; i++){
		inOutBuf[i] = inOutBuf[i] ^ in2[i];
	}
	return STATUS_OK;
}

byte MFRC522Desfire::PCD_Cmac(DesfireAesKey key, byte *iv, byte *subkey1, byte *subkey2, byte *data, byte &dataLen){
	byte *buffer;
	byte bufferLen;
	if (dataLen % 16 != 0){
		bufferLen = dataLen - (dataLen % 16) + 16;
	}
	else{
		bufferLen = dataLen;
	}
	buffer = new byte[bufferLen];

	memcpy(buffer, data, dataLen);

	//Wenn Länge von Data 0 oder kein Vielfaches von 16 ist, dann gepaddete Daten setzen und Subkey2 anwenden
	if((dataLen == 0)||(dataLen % 16 != 0)){
		memset(&buffer[dataLen], 0x00, bufferLen - dataLen);
		buffer[dataLen] = (byte) 0x80;
		PCD_Xor(&buffer[bufferLen - 16], 16, subkey2);
	}
	else {
		//Sonst Subkey1 anwenden
		PCD_Xor(&buffer[bufferLen - 16 - 1], bufferLen - 1, subkey1);
	}

	byte *dummyBuf = new byte[bufferLen];
	PCD_Encrypt(key, buffer, bufferLen, iv, dummyBuf, bufferLen);

	delete[] dummyBuf;
	delete[] buffer;
	return STATUS_OK;
}

byte MFRC522Desfire::PCD_ShiftLeft(byte *data, byte dataLen){
    for (int n = 0; n < dataLen - 1; n++) {
        data[n] = ((data[n] << 1) | ((data[n+1] >> 7)&0x01));
    }
    data[dataLen - 1] <<= 1;
    return STATUS_OK;
}

byte MFRC522Desfire::PCD_Pbkdf2(byte* password, byte passLen, byte* salt, byte saltLen, int count, DesfireAesKey &key){
	byte derivedKey[AES_KEY_LENGTH];
	int hLen = 32;

	byte U[32];
	byte T[32];
	byte *block1 = new byte[saltLen + 4];

	int l = 1;
	int r = 16;

	//System.arraycopy(Salt, 0, block1, 0, Salt.length);
	memcpy(block1, salt, saltLen);

	for (int i = 1; i <= l; i++) {
		block1[saltLen + 0] = (byte) (i >> 24 & 0xff);
		block1[saltLen+ 1] = (byte) (i >> 16 & 0xff);
		block1[saltLen+ 2] = (byte) (i >> 8  & 0xff);
		block1[saltLen+ 3] = (byte) (i >> 0  & 0xff);

		std::string mac = hmac<SHA256>(block1, saltLen + 4, password, passLen);
		for (unsigned int i = 0; i < mac.length(); i += 2) {
			std::string byteString = mac.substr(i, 2);
			byte b = (byte) strtol(byteString.c_str(), NULL, 16);
			U[i/2] = b;
		}
		memcpy(T, U, hLen);

		for (int j = 1; j < count; j++) {
			std::string mac = hmac<SHA256>(U, 32, password, passLen);
			for (unsigned int i = 0; i < mac.length(); i += 2) {
				std::string byteString = mac.substr(i, 2);
				byte b = (byte) strtol(byteString.c_str(), NULL, 16);
				U[i/2] = b;
			}

			for (int k = 0; k < hLen; k++) {
				T[k] ^= U[k];
			}
		}

		memcpy(&derivedKey[(i-1)*hLen], T, (i == l ? r : hLen));
	}
	memcpy(key.keyByte, derivedKey, AES_KEY_LENGTH);
	delete[] block1;
	return STATUS_OK;
}

byte MFRC522Desfire::PICC_Rats(byte *ats){
	byte commandBuf[4];
	byte status;
	byte responseBuf[10];
	byte responseBufLen;

	commandBuf[0] = 0xe0;
	commandBuf[1] = 0x50;
	status = MFRC522::PCD_CalculateCRC(commandBuf, 2, &commandBuf[2]);
	if (status != STATUS_OK){
		return status;
	}
	responseBufLen = 10;
	status = MFRC522::PCD_TransceiveData(commandBuf, 4, responseBuf, &responseBufLen);
	if (status != STATUS_OK){
		return status;
	}
	//Expect 6 byte ATS + 2 byte CRC
	if (responseBufLen != 8){
		return STATUS_ERROR;
	}
	//TODO: Check CRC in answer

	//Copy ATS bytes
	for (int i = 0; i < 6; i++){
		ats[i] = responseBuf[i];
	}
	return STATUS_OK;
}

byte MFRC522Desfire::PICC_SendApduCommand(byte command, byte *data, byte dataLen, byte &answerStatus, byte *answer, byte *answerLen){
	byte status;
	byte *commandBuf;
	byte responseBuf[128];
	byte responseBufLen = 128;
	static byte blockNo = 1;

	blockNo = !blockNo;

	commandBuf = new byte[8 + dataLen + 2];

	commandBuf[0] = 0x0a | (blockNo & 0x01);
	commandBuf[1] = 0x00;
	commandBuf[2] = 0x90;
	commandBuf[3] = command;
	commandBuf[4] = 0x00;
	commandBuf[5] = 0x00;
	commandBuf[6] = dataLen;
	for(int i = 0; i < dataLen; i++){
		commandBuf[7+i] = data[i];
	}
	commandBuf[7 + dataLen] = 0x00;
	status = MFRC522::PCD_CalculateCRC(commandBuf, 7 + dataLen + (dataLen>0?1:0), &commandBuf[7 + dataLen + (dataLen>0?1:0)]);
	if (status != STATUS_OK){
		delete[] commandBuf;
		std::cout << "CRC Calc failed" << std::endl;
		return status;
	}

	debugOutByteArray(commandBuf, 7 + dataLen + (dataLen>0?1:0), "APDU Command:");

	status = MFRC522::PCD_TransceiveData(commandBuf, 7 + dataLen + (dataLen>0?1:0)+ 2, responseBuf, &responseBufLen);
	delete[] commandBuf;
	if (status != STATUS_OK){
		std::cout << "Transceive failed with " << (int) status << std::endl;
		return status;
	}

	debugOutByteArray(responseBuf, responseBufLen, "APDU Answer:");

	//Check if response has correct framing
	if ((responseBuf[0] != (0x0a | (blockNo & 0x01))) || (responseBuf[1] != 0x00) || (responseBuf[responseBufLen - 4] != 0x91)){
		return STATUS_ERROR;
	}

	//Check if CRC is correct
	byte crc[2];
	status = MFRC522::PCD_CalculateCRC(responseBuf, responseBufLen - 2, crc);
	if (status != STATUS_OK){
		return status;
	}
	if ((responseBuf[responseBufLen - 2] != crc[0]) || (responseBuf[responseBufLen - 1] != crc[1])){
		return STATUS_ERROR;
	}

	answerStatus = responseBuf[responseBufLen - 3];
	//If no additional information is requested, return
	if (answer == NULL){
		return STATUS_OK;
	}

	//Check if answer buffer is long enough
	if (*answerLen < (responseBufLen - 6)){
		return STATUS_NO_ROOM;
	}

	//Create answer
	memcpy(answer, &responseBuf[2], responseBufLen - 6);
	*answerLen = responseBufLen - 6;
	return STATUS_OK;
}

byte MFRC522Desfire::PICC_SendApduCommandCmaced(byte command, byte *data, byte dataLen, byte &answerStatus, byte *answer, byte *answerLen){
	byte *buffer = new byte[dataLen + 1];
	buffer[0] = command;
	memcpy(&buffer[1], data, dataLen);
	byte bufLen = dataLen + 1;
	PCD_Cmac(sessionKey, initVector, subKey1, subKey2, buffer, bufLen);
	delete[] buffer;
	return PICC_SendApduCommand(command, data, dataLen, answerStatus, answer, answerLen);
}

byte MFRC522Desfire::PICC_SendApduCommandEncrypted(byte command, byte *plainData, byte plainDataLen, byte *securedData, byte securedDataLen, byte &answerStatus, byte *answer, byte *answerLen){
	//Checksumme berechnen
	byte *dataForChecksum = new byte[1 + plainDataLen + securedDataLen];
	dataForChecksum[0] = command;
	memcpy(&dataForChecksum[1], plainData, plainDataLen);
	memcpy(&dataForChecksum[plainDataLen + 1], securedData, securedDataLen);
	//TODO: CRC32 für Checksumme
	//byte checksum[4];
	CRC32 crc32;
	std::string checksumStr = crc32(dataForChecksum, 1 + plainDataLen + securedDataLen);
	byte checksum[4];
	for (unsigned int i = 0; i < checksumStr.length(); i += 2) {
		std::string byteString = checksumStr.substr(6-i, 2);
		byte b = (byte) strtol(byteString.c_str(), NULL, 16);
		//CRC32 von Desfire hat alle Bits invertiert, daher hier XOR 0xff
		checksum[i/2] = b ^ 0xff;
	}
	delete[] dataForChecksum;

	//Checksumme an Daten hängen
	byte paddedDataLen = (securedDataLen + 4) % 16 != 0 ? (securedDataLen + 4) - ((securedDataLen + 4) % 16) + 16 : (securedDataLen + 4);
	byte *dataWithChecksum = new byte[paddedDataLen];
	memset(dataWithChecksum, 0x00, paddedDataLen);
	memcpy(dataWithChecksum, securedData, securedDataLen);
	memcpy(&dataWithChecksum[securedDataLen], &checksum[0], 4);

	//Verschlüsseln und Daten zusammensetzen
	byte *cmdData = new byte[paddedDataLen + plainDataLen];
	memcpy(cmdData, plainData, plainDataLen);
	PCD_Encrypt(sessionKey, dataWithChecksum, paddedDataLen, initVector, &cmdData[plainDataLen], paddedDataLen);
	delete[] dataWithChecksum;

	//Versenden
	byte status;
	status = PICC_SendApduCommand(command, cmdData, paddedDataLen + plainDataLen, answerStatus, answer, answerLen);
	delete[] cmdData;

	return status;
}

byte MFRC522Desfire::Desfire_SelectApplication(uint32_t applicationId){
	byte status;
	byte dataBuf[3];
	byte desfireStatus;
	byte desfireStatusLen = 1;
	dataBuf[0] = (byte) (applicationId & 0xff);
	dataBuf[1] = (byte) ((applicationId >> 8)  & 0xff);
	dataBuf[2] = (byte) ((applicationId >> 16) & 0xff);
	status = PICC_SendApduCommand(0x5a, dataBuf, 3, desfireStatus);
	if (status != STATUS_OK){
		return status;
	}

	//Check if answer has correct length
	if (desfireStatusLen != 1){
		return STATUS_ERROR;
	}

	//Check answer status
	if (desfireStatus != DESFIRE_OPERATION_OK){
		return STATUS_ERROR;
	}

	return STATUS_OK;
}

byte MFRC522Desfire::Desfire_Authenticate(byte keyNo, std::string password, std::string salt){
	MFRC522Desfire::DesfireAesKey key;
	std::vector<byte> passBytes(password.begin(), password.end());
	std::vector<byte> saltBytes(salt.begin(), salt.end());
	PCD_Pbkdf2(&passBytes[0], password.length(), &saltBytes[0], salt.length(), 10000, key);
	return Desfire_Authenticate(keyNo, key);
}

byte MFRC522Desfire::Desfire_Authenticate(byte keyNo, std::string password){
	return Desfire_Authenticate(keyNo, password, "");
}

byte MFRC522Desfire::Desfire_Authenticate(byte keyNo, DesfireAesKey key){
    byte status;
	byte answerBuf[20];
	byte answerBufLen;
	byte answerStatus;

	//Initvector zurücksetzen
	memset(initVector, 0x00, AES_KEY_LENGTH);

    //Verschlüsselten RandB empfangen
    answerBufLen = 20;
	status = PICC_SendApduCommand(0xaa, &keyNo, 1, answerStatus, answerBuf, &answerBufLen);
	if (status != STATUS_OK){
		return status;
	}
	if ((answerBufLen != AES_KEY_LENGTH) || (answerStatus != DESFIRE_ADDITIONAL_FRAME)){
        return STATUS_ERROR;
	}
    //RandB entschlüsseln
    byte randB[AES_KEY_LENGTH];
    //byte testBuf[16] = {0x64, 0x2d, 0xed, 0x2, 0xf6, 0xa5, 0xc3, 0x36, 0x5, 0xe, 0xb0, 0xaf, 0x0, 0x75, 0x2c, 0xcf};
    //PCD_Decrypt(key, &testBuf[0], AES_KEY_LENGTH, initVector, randB, AES_KEY_LENGTH);
    PCD_Decrypt(key, &answerBuf[0], AES_KEY_LENGTH, initVector, randB, AES_KEY_LENGTH);
    //RandB rotieren
    byte randB_rot[AES_KEY_LENGTH];
    for(int i = 0; i < AES_KEY_LENGTH; i++){
        randB_rot[i] = randB[(i+1)%AES_KEY_LENGTH];
    }
    //RandA zufällig bestimmten
    //byte randA[AES_KEY_LENGTH] = {0xbe, 0x73, 0xc9, 0xd3, 0xf6, 0x3f, 0x1, 0xc4, 0xfb, 0x3f, 0x2e, 0xc0, 0x57, 0xb7, 0xd2, 0x99};
    byte randA[AES_KEY_LENGTH];
    for (int i = 0; i < AES_KEY_LENGTH; i++){
    	randA[i] = rand();
    }
    //RandA + RandB_rot erstellen und verschlüsseln
    byte data[2*AES_KEY_LENGTH];
    for(int i = 0; i < AES_KEY_LENGTH; i++){
        data[i] = randA[i];
        data[i+AES_KEY_LENGTH] = randB_rot[i];
    }
    byte encryptedData[2*AES_KEY_LENGTH];
    PCD_Encrypt(key, data, 2*AES_KEY_LENGTH, initVector, encryptedData, 2*AES_KEY_LENGTH);

    //Verschlüsselten RandA+RandB_rot an PICC senden
    status = PICC_SendApduCommand(0xaf, encryptedData, 2 * AES_KEY_LENGTH, answerStatus, answerBuf, &answerBufLen);
    if (status != STATUS_OK){
		return status;
	}
	if ((answerBufLen != AES_KEY_LENGTH) || (answerStatus != DESFIRE_OPERATION_OK)){
        return STATUS_ERROR;
	}
    //Prüfen, ob Antwort verschlüsselt RandA_rot ist
    PCD_Decrypt(key, &answerBuf[0], AES_KEY_LENGTH, initVector, data, AES_KEY_LENGTH);
    for(int i = 0; i < AES_KEY_LENGTH; i++){
    	if(data[i] != randA[(i+1) % AES_KEY_LENGTH]){
        	return STATUS_ERROR;
        }
    }
    //SessionKey berechnen
    for(int i = 0; i < AES_KEY_LENGTH/4; i++){
        sessionKey.keyByte[i] = randA[i];
        sessionKey.keyByte[i+AES_KEY_LENGTH/4] = randB[i];
        sessionKey.keyByte[i+AES_KEY_LENGTH/4*2] = randA[i+AES_KEY_LENGTH-4];
        sessionKey.keyByte[i+AES_KEY_LENGTH/4*3] = randB[i+AES_KEY_LENGTH-4];
    }
    //Subkeys berechnen
    memset(initVector, 0x00, AES_KEY_LENGTH);
    memset(data, 0x00, AES_KEY_LENGTH);
    PCD_Encrypt(sessionKey, data, AES_KEY_LENGTH, initVector, encryptedData, AES_KEY_LENGTH);

    memcpy(subKey1, encryptedData, AES_KEY_LENGTH);
    PCD_ShiftLeft(subKey1, AES_KEY_LENGTH);
    if((encryptedData[0] & 0x80) != 0){
        subKey1[AES_KEY_LENGTH-1] ^= 0x87;
    }

    memcpy(subKey2, subKey1, AES_KEY_LENGTH);
    PCD_ShiftLeft(subKey2, AES_KEY_LENGTH);
    if((subKey1[0] & 0x80) != 0){
        subKey2[AES_KEY_LENGTH-1] ^= 0x87;
    }

    //Init Vector resetten
    memset(initVector, 0x00, AES_KEY_LENGTH);

	return STATUS_OK;
}

byte MFRC522Desfire::Desfire_GetValue(byte fileNo, uint32_t &value){
	byte status;
		byte answerBuf[64];
		byte answerBufLen = 64;
		byte answerStatus;
		byte sendBuf[1];
		sendBuf[0] = fileNo;

		status = PICC_SendApduCommandCmaced(0x6c, sendBuf, 1, answerStatus, answerBuf, &answerBufLen);
		if (status != STATUS_OK){
			return status;
		}

		debugOutByteArray(answerBuf, answerBufLen, "AnswerBuf:");

		//Daten dekodieren
		//TODO: Padding wenn Datenlänge kein vielfaches von 16
		byte data[16];
		PCD_Decrypt(sessionKey, answerBuf, answerBufLen, initVector, data, 16);

		CRC32 crc32;
		debugOutByteArray(data, 16, "Data:");
		value = (uint32_t)data[0] + ((uint32_t)data[1] << 8) + ((uint32_t)data[2] << 16) + ((uint32_t)data[3] << 24);

		return STATUS_OK;
}

byte MFRC522Desfire::Desfire_Credit(byte fileNo, uint32_t value){
	return STATUS_OK;
}

byte MFRC522Desfire::Desfire_Debit(byte fileNo, uint32_t value){
	byte sendBuf[4];
	byte status;
	byte answerStatus;
	sendBuf[0] = value & 0xff;
	sendBuf[1] = (value >> 8) & 0xff;
	sendBuf[2] = (value >> 16) & 0xff;
	sendBuf[3] = (value >> 24) & 0xff;

	status = PICC_SendApduCommandEncrypted(0xdc, &fileNo, 1, sendBuf, 4, answerStatus);
	if (status != STATUS_OK){
		return status;
	}
	if (answerStatus != DESFIRE_OPERATION_OK){
		return STATUS_ERROR;
	}

	//CMAC berechnen, um IV zu aktualisieren
	//TODO: CMAC in PICC Antwort mit berechneter CMAC vergleichen
	//TODO: klären, woraus sich die Daten zusammensetzen
	byte cmacDummy = 0;
	byte cmacDummyLen = 1;
	PCD_Cmac(sessionKey, initVector, subKey1, subKey2, &cmacDummy, cmacDummyLen);

	return STATUS_OK;
}

byte MFRC522Desfire::Desfire_CommitTransaction(){
	byte status;
	byte desfireStatus;

	status = PICC_SendApduCommandCmaced(0xc7, NULL, 0, desfireStatus);
	if (status != STATUS_OK){
		return status;
	}

	//Check answer status
	if (desfireStatus != DESFIRE_OPERATION_OK){
		return STATUS_ERROR;
	}

	//CMAC berechnen, um IV zu aktualisieren
	//TODO: CMAC in PICC Antwort mit berechneter CMAC vergleichen
	//TODO: klären, woraus sich die Daten zusammensetzen
	byte cmacDummy = 0;
	byte cmacDummyLen = 1;
	PCD_Cmac(sessionKey, initVector, subKey1, subKey2, &cmacDummy, cmacDummyLen);

	return STATUS_OK;
}

byte MFRC522Desfire::Desfire_ReadData(byte fileNo, uint32_t offset, uint32_t length, byte *data, byte *dataLen){
	byte status;
	byte answerBuf[64];
	byte answerBufLen = 64;
	byte answerStatus;
	byte sendBuf[7];
	sendBuf[0] = fileNo;
	sendBuf[1] = offset & 0xff;
	sendBuf[2] = (offset >> 8) & 0xff;
	sendBuf[3] = (offset >> 16) & 0xff;
	sendBuf[4] = length & 0xff;
	sendBuf[5] = (length >> 8) & 0xff;
	sendBuf[6] = (length >> 16) & 0xff;
	status = PICC_SendApduCommandCmaced(0xbd, sendBuf, 7, answerStatus, answerBuf, &answerBufLen);
	if (status != STATUS_OK){
		return status;
	}

	debugOutByteArray(answerBuf, answerBufLen, "AnswerBuf:");

	//Daten dekodieren
	//TODO: Padding wenn Datenlänge kein vielfaches von 16
	PCD_Decrypt(sessionKey, answerBuf, answerBufLen, initVector, data, *dataLen);

	debugOutByteArray(data, *dataLen, "Data:");

	return STATUS_OK;
}

byte MFRC522Desfire::PICC_Select(Uid *uid){
	byte status;
	status = MFRC522::PICC_Select(uid);
	if (status != STATUS_OK){
		return status;
	}
	//Only support Desfire cards. Check if 14443-4 type, otherwise card is no Desfire.
	if (MFRC522::PICC_GetType(uid->sak) != PICC_TYPE_ISO_14443_4) {
		return STATUS_ERROR;
	}

	//RATS -> ATQA
	byte ats[6];
	PICC_Rats(ats);
	debugOutByteArray(ats, 6, "ATS:");
	return STATUS_OK;
}
