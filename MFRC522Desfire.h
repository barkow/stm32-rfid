#include <MFRC522.h>

class MFRC522Desfire : public MFRC522 {
	public:
		MFRC522Desfire(byte chipSelectPin, byte resetPowerDownPin) : MFRC522(chipSelectPin, resetPowerDownPin){};
		byte PICC_Select(Uid *uid);
		
	private:
		void Anticoll();
		void Anticoll2();
		
};