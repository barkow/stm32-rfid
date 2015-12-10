/*
 * display.h
 *
 *  Created on: 26.11.2015
 *      Author: barkow
 */

#ifndef INCLUDE_DISPLAY_H_
#define INCLUDE_DISPLAY_H_

#include <stdint.h>

#define LCDWIDTH 84
#define LCDHEIGHT 48

#define PCD8544_FUNCTIONSET 		0x20
#define PCD8544_POWERDOWN			0x04
#define PCD8544_VERTICALADDRESSING	0x02
#define PCD8544_EXTENDEDINSTRUCTION 0x01

//Commands in Basic Instruction Set
#define PCD8544_DISPLAYCONTROL 		0x08
#define PCD8544_DISPLAYBLANK 		0x00
#define PCD8544_DISPLAYALLON 		0x01
#define PCD8544_DISPLAYNORMAL 		0x04
#define PCD8544_DISPLAYINVERSE		0x05

#define PCD8544_SETXADDRESS 		0x80

#define PCD8544_SETYADDRESS 		0x40

//Commands in Extended Instruction Set
#define PCD8544_TEMPERATURECONTROL	0x04

#define PCD8544_SETBIAS 			0x10

#define PCD8544_SETVOP 				0x80


class display {
public:
	display();
	void data(uint8_t *dat, uint16_t len);
	void command(uint8_t cmd);
	void transferBuffer(uint8_t* buffer, uint16_t bufferSize);
};



#endif /* INCLUDE_DISPLAY_H_ */
