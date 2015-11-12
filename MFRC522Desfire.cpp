#include "MFRC522Desfire.h"

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

byte MFRC522Desfire::PICC_SendApduCommand(byte command, byte *data, byte dataLen, byte *answer, byte *answerLen){
	byte status;
	byte *commandBuf;
	byte responseBuf[10];
	byte responseBufLen = 10;
	
	commandBuf = new byte[8 + dataLen + 2];
	
	commandBuf[0] = 0x0a;
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
	status = MFRC522::PCD_CalculateCRC(commandBuf, 8 + dataLen, &commandBuf[8 + dataLen]);
	if (status != STATUS_OK){
		delete[] commandBuf;
		std::cout << "CRC Calc failed" << std::endl;
		return status;
	}
	
	status = MFRC522::PCD_TransceiveData(commandBuf, 8 + dataLen + 2, responseBuf, &responseBufLen);
	delete[] commandBuf;
	if (status != STATUS_OK){
		std::cout << "Transceive failed with " << (int) status << std::endl;
		return status;
	}
	
	std::cout << "APDU: ";
	for(int i = 0; i < responseBufLen; i++){
		std::cout << "0x" << std::hex << (int) responseBuf[i] << " ";
	}
	std::cout << std::endl;
	
	//Check if response has correct framing
	if ((responseBuf[0] != 0x0a) || (responseBuf[1] != 0x00) || (responseBuf[2] != 0x91)){
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
	
	//Check if answer buffer is long enough
	if (*answerLen < (responseBufLen - 5)){
		return STATUS_NO_ROOM;
	}
	
	//Create answer
	for (int i = 0; i < responseBufLen - 5; i++){
		answer[i] = responseBuf[i+3];
	}
	*answerLen = responseBufLen - 5;
	
	return STATUS_OK;
}

byte MFRC522Desfire::Desfire_SelectApplication(uint32_t applicationId){
	byte status;
	byte dataBuf[3];
	byte desfireStatus;
	byte desfireStatusLen = 1;
	dataBuf[0] = (byte) (applicationId & 0xff);
	dataBuf[1] = (byte) ((applicationId >> 8)  & 0xff);
	dataBuf[2] = (byte) ((applicationId >> 16) & 0xff);
	status = PICC_SendApduCommand(0x5a, dataBuf, 3, &desfireStatus, &desfireStatusLen);
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

byte MFRC522Desfire::Desfire_Authenticate(byte keyNo, DesfireAesKey key){
	return STATUS_OK;
}

byte MFRC522Desfire::Desfire_GetValue(byte fileNo, uint32_t &value){
	return STATUS_OK;
}

byte MFRC522Desfire::Desfire_Credit(byte fileNo, uint32_t value){
	return STATUS_OK;
}

byte MFRC522Desfire::Desfire_Debit(byte fileNo, uint32_t value){
	return STATUS_OK;
}

byte MFRC522Desfire::Desfire_ReadData(byte fileNo, uint32_t offset, uint32_t length, byte *data, byte *dataLen){
	return STATUS_OK;
}

byte MFRC522Desfire::PICC_Select(Uid *uid){
	std::cout << "My selection routine" << std::endl;

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
	std::cout << "ATS: ";
	for (int i = 0; i < 6; i++){
		std::cout << "0x" << std::hex << (int) ats[i] << " ";
	}
	std::cout << std::endl;
	return STATUS_OK;
}