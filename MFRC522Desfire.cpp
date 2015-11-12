#include "MFRC522Desfire.h"
#include <iomanip>

void MFRC522Desfire::Anticoll(){
	byte status;
	byte command[2] = {PICC_CMD_SEL_CL1, 0x20};
	byte buffer[5];
	byte bufferLen = 5;
	status = PCD_TransceiveData(command, 2, buffer, &bufferLen);
	if (status != STATUS_OK) {
		//TODO: Exception
		std::cout << "Error" << std::endl;
		return;
	}
	//Checksumme überprüfen
	
	std::cout << "Los: " << std::hex << (int) buffer[0] << std::endl;
	std::cout << std::hex << (int) buffer[1] << std::endl;
	std::cout << std::hex << (int) buffer[2] << std::endl;
	std::cout << std::hex << (int) buffer[3] << std::endl;
	std::cout << std::hex << (int) buffer[4] << std::endl;
}

void MFRC522Desfire::Anticoll2(){
}

byte MFRC522Desfire::PICC_Select(Uid *uid){
	std::cout << "My selection routine" << std::endl;
	Anticoll();
	return 0;
}