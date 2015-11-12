#include <MFRC522.h>

class MFRC522Desfire : public MFRC522 {
	public:
		enum DesfireStatusCode {
			DESFIRE_OPERATION_OK 					= 0x00,
			DESFIRE_APPLICATION_NOT_FOUND	= 0xa0
		};
		
		typedef struct {
		byte		keyByte[16];
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
};