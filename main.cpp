#include "MFRC522Desfire.h"
#include "Arduino.h"
#include <unistd.h>
#include <stdexcept>

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

	mfrc522.PCD_WriteRegister(mfrc522.GsNReg, 0xff);
	mfrc522.PCD_WriteRegister(mfrc522.CWGsPReg, 0x3f);
	mfrc522.PCD_SetAntennaGain(0xff);

	try{
		int timeout = 3000;
		while (!mfrc522.PICC_IsNewCardPresent() && timeout > 0){timeout--;};
		if (timeout <= 0){
			 throw std::runtime_error("Timeout");
		}
		else {
			Serial.println("Card detected");
		}

		MFRC522::Uid uid;
		if (mfrc522.PICC_Select(&uid) != mfrc522.STATUS_OK){
			throw std::runtime_error("No Desfire Card");
		}
		Serial.println("Select Application 0x000005");
		if (mfrc522.Desfire_SelectApplication(0x000005) != mfrc522.STATUS_OK){
			throw std::runtime_error("Select Application failed");
		}

		if (mfrc522.Desfire_Authenticate(1, "password1") != mfrc522.STATUS_OK){
			throw std::runtime_error("Auth failed");
		}
		byte data[32];
		byte dataLen = 32;
		if (mfrc522.Desfire_ReadData(5, 0, 32, data, &dataLen) != mfrc522.STATUS_OK){
			throw std::runtime_error("Readdata failed");
		}
		Serial.println("Data");
		Serial.println((((uint32_t) data[0]) << 24) + (((uint32_t) data[1]) << 16) + (((uint32_t) data[2]) << 8) + (((uint32_t) data[3]) << 0));
		Serial.println("Select Application 0x0000f7");
		if (mfrc522.Desfire_SelectApplication(0x0000f7) != mfrc522.STATUS_OK){
			throw std::runtime_error("Select Application failed");
		}
	
		if (mfrc522.Desfire_Authenticate(1, "password2", "uidandsalt") != mfrc522.STATUS_OK){
			throw std::runtime_error("Auth failed");
		}
		uint32_t v;
		if (mfrc522.Desfire_GetValue(1, v) != mfrc522.STATUS_OK){
			throw std::runtime_error("GetValue failed");
		}
		Serial.println("Value:");
		Serial.println(v);
		if (mfrc522.Desfire_Debit(1, 1) != mfrc522.STATUS_OK){
			throw std::runtime_error("Debit failed");
		}
		if (mfrc522.Desfire_CommitTransaction() != mfrc522.STATUS_OK){
			throw std::runtime_error("Commit Transaction failed");
		}
		if (mfrc522.Desfire_GetValue(1, v) != mfrc522.STATUS_OK){
			throw std::runtime_error("GetValue failed");
		}
		Serial.println("Value:");
		Serial.println(v);

		Serial.println(uid.size);
		Serial.println(uid.uidByte[0]);
		Serial.println(uid.uidByte[1]);
		Serial.println(uid.uidByte[2]);
		Serial.println(uid.uidByte[3]);
		Serial.println(uid.uidByte[4]);
		Serial.println(uid.uidByte[5]);
		Serial.println(uid.uidByte[6]);
		Serial.println(uid.sak);
	}
	catch(std::exception &e){
		Serial.println("Error:");
		Serial.println(e.what());
	}
	mfrc522.PCD_AntennaOff();

  Serial.println("RFID Test finished");
}
