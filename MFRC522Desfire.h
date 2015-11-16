#include <MFRC522.h>

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
		byte PICC_Select(Uid *uid);
		byte Desfire_SelectApplication(uint32_t applicationId);
		byte Desfire_Authenticate(byte keyNo, DesfireAesKey key);
		byte Desfire_GetValue(byte fileNo, uint32_t &value);
		byte Desfire_Credit(byte fileNo, uint32_t value);
		byte Desfire_Debit(byte fileNo, uint32_t value);
		byte Desfire_ReadData(byte fileNo, uint32_t offset, uint32_t length, byte *data, byte *dataLen);
	private:
		byte PICC_Rats(byte *atqa);
		byte PICC_SendApduCommand(byte command, byte *data, byte dataLen, byte *answer, byte *answerLen);
		byte PCD_Decrypt(DesfireAesKey key, byte *encryptedData, byte encryptedDataLen, byte *initVector, byte *decryptedData, byte *decryptedDataLen);
		byte PCD_Encrypt(DesfireAesKey key, byte *decryptedData, byte *decryptedDataLen, byte *initVector, byte *encryptedData, byte encryptedDataLen);
		byte initVector[AES_KEY_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		byte sessionKey[AES_KEY_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		byte subKey1[AES_KEY_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		byte subKey2[AES_KEY_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
};
