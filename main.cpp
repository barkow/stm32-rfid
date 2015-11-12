#include <MFRC522Desfire.h>
#include <Arduino.h>
#include <unistd.h>

#define RC522_CS 0
#define RC522_RESET 1

SerialClass Serial;
SPIClass SPI;

byte pgm_read_byte(const byte* address_short){
  return *address_short;
}

int digitalRead(uint8_t){
  return 1;
}

//BlackLib::BlackGPIO rc522Cs(BlackLib::GPIO_5,BlackLib::output, BlackLib::FastMode);

void digitalWrite(uint8_t pin, uint8_t val){
  switch (pin){
    case RC522_CS:
		  //usleep(100);
      //rc522Cs.setValue(val ? BlackLib::high : BlackLib::low);
      break;
  }
}

void pinMode(uint8_t pin, uint8_t mode){
  switch (pin){
    case RC522_CS:
      break;
  }
}

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1

#define LSBFIRST 0
#define MSBFIRST 1

#define CHANGE 1
#define FALLING 2
#define RISING 3

void delay(unsigned long us){
usleep(us+1000);
sleep(2);
}


int main( int argc, const char* argv[] )
{
  Serial.println("RFID Test");
	SPI.open(1,0);
  MFRC522Desfire mfrc522(RC522_CS, RC522_RESET);

	mfrc522.PCD_Init();
  /*bool hasFailed = !mfrc522.PCD_PerformSelfTest();
  if (hasFailed) {
    Serial.println("Selftest failed");
    hasFailed = !mfrc522.PCD_PerformSelfTest();
     mfrc522.PCD_Init();
  }
  if (hasFailed) {
    Serial.println("Selftest failed");
    hasFailed = !mfrc522.PCD_PerformSelfTest();
     mfrc522.PCD_Init();
  }*/

  //byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  //Serial.println(v);
  mfrc522.PCD_WriteRegister(mfrc522.GsNReg, 0xff);
  mfrc522.PCD_WriteRegister(mfrc522.CWGsPReg, 0x3f);
  mfrc522.PCD_SetAntennaGain(0xff);

	int timeout = 3000;
  while (!mfrc522.PICC_IsNewCardPresent() && timeout > 0){timeout--;};
	if (timeout <= 0){
		Serial.println("Timeout");
	}
	else {
		Serial.println("Card detected");
	}
	
	MFRC522::Uid uid;
	mfrc522.PICC_Select(&uid);
	Serial.println(uid.size);
	//mfrc522.PICC_ReadCardSerial();
	//mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
	
	/*mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_SoftReset);
	for(int hh = 0; hh<10; hh++){
	mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 7);
	usleep(5000);
	mfrc522.PCD_SetRegisterBitMask(mfrc522.BitFramingReg, 0x80);
	}*/

  Serial.println("RFID Test finished");
}
