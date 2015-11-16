#include "MFRC522Desfire.h"
#include "Arduino.h"
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
	
	if (mfrc522.PICC_Select(&uid) != mfrc522.STATUS_OK){
		Serial.println("No Desfire Card");
	}
	Serial.println("Select Application 0x000005");
	if (mfrc522.Desfire_SelectApplication(0x000005) != mfrc522.STATUS_OK){
		Serial.println("Select Application failed");
	}
	Serial.println(uid.size);
	Serial.println(uid.uidByte[0]);
	Serial.println(uid.uidByte[1]);
	Serial.println(uid.uidByte[2]);
	Serial.println(uid.uidByte[3]);
	Serial.println(uid.uidByte[4]);
	Serial.println(uid.uidByte[5]);
	Serial.println(uid.uidByte[6]);
	Serial.println(uid.sak);
	//mfrc522.PICC_ReadCardSerial();
	//mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
	
	/*mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_SoftReset);
	for(int hh = 0; hh<10; hh++){
	mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 7);
	usleep(5000);
	mfrc522.PCD_SetRegisterBitMask(mfrc522.BitFramingReg, 0x80);
	}*/
	
	mfrc522.PCD_AntennaOff();

  Serial.println("RFID Test finished");
}
