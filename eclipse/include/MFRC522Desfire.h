#include <MFRC522.h>
#include <string>

#define AES_KEY_LENGTH 16

class MFRC522Desfire : public MFRC522 {
	public:
		enum DesfireStatusCode {
			DESFIRE_OPERATION_OK 			= 0x00,
			DESFIRE_APPLICATION_NOT_FOUND	= 0xa0,
			DESFIRE_ADDITIONAL_FRAME        = 0xaf
		};

		typedef struct {
            byte keyByte[AES_KEY_LENGTH];
        } DesfireAesKey;

		MFRC522Desfire(byte chipSelectPin, byte resetPowerDownPin) : MFRC522(chipSelectPin, resetPowerDownPin){};
		MFRC522Desfire() : MFRC522(0, 1){};
		byte PICC_Select(Uid *uid);
		MFRC522Desfire::DesfireAesKey DeriveKeyFromPassword(std::string password, std::string salt);
		byte Desfire_SelectApplication(uint32_t applicationId);
		byte Desfire_Authenticate(byte keyNo, DesfireAesKey key);
		byte Desfire_Authenticate(byte keyNo, std::string password);
		byte Desfire_Authenticate(byte keyNo, std::string password, std::string salt);
		byte Desfire_GetValue(byte fileNo, uint32_t &value);
		byte Desfire_Credit(byte fileNo, uint32_t value);
		byte Desfire_Debit(byte fileNo, uint32_t value);
		byte Desfire_CommitTransaction();
		byte Desfire_ReadData(byte fileNo, uint32_t offset, uint32_t length, byte *data, byte *dataLen);
	private:
		byte PICC_Rats(byte *atqa);
		byte PICC_SendApduCommand(byte command, byte *data, byte dataLen, byte &answerStatus, byte *answer = NULL, byte *answerLen = 0);
		byte PICC_SendApduCommandCmaced(byte command, byte *data, byte dataLen, byte &answerStatus, byte *answer = NULL, byte *answerLen = 0);
		byte PICC_SendApduCommandEncrypted(byte command, byte *plainData, byte plainDataLen, byte *securedData, byte secureDataLen, byte &answerStatus, byte *answer = NULL, byte *answerLen = 0);
		byte PCD_Decrypt(DesfireAesKey key, byte *encryptedData, byte encryptedDataLen, byte *initVector, byte *decryptedData, byte decryptedDataLen);
		byte PCD_Encrypt(DesfireAesKey key, byte *decryptedData, byte decryptedDataLen, byte *initVector, byte *encryptedData, byte encryptedDataLen);
		byte PCD_Xor(byte *inOutBuf, byte bufLen, byte *in2);
		byte PCD_Cmac(DesfireAesKey key, byte *iv, byte *subkey1, byte *subkey2, byte *data, byte &dataLen);
		byte PCD_ShiftLeft(byte *data, byte dataLen);
		byte PCD_Pbkdf2(byte* password, byte passLen, byte* salt, byte saltLen, int count, DesfireAesKey &key);
		byte initVector[AES_KEY_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		//byte sessionKey[AES_KEY_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		DesfireAesKey sessionKey;
		byte subKey1[AES_KEY_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		byte subKey2[AES_KEY_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
};
