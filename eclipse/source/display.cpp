/*
 * display.cpp
 *
 *  Created on: 26.11.2015
 *      Author: barkow
 */

#include "display.h"
#include "spi.h"

/*
 * Aus PCD8544 Datenblatt (https://www.sparkfun.com/datasheets/LCD/Monochrome/Nokia5110.pdf)
 *
 * Display Data RAM (DDRAM)
 * The DDRAM is a 48 × 84 bit static RAM which stores the
 * display data. The RAM is divided into six banks of 84 bytes
 * (6 × 8 × 84 bits). During RAM access, data is transferred
 * to the RAM through the serial interface. There is a direct
 * correspondence between the X-address and the column
 * output number.
 *
 * LCD row and column drivers
 * The PCD8544 contains 48 row and 84 column drivers,
 * which connect the appropriate LCD bias voltages in
 * sequence to the display in accordance with the data to be
 * displayed. Figure 2 shows typical waveforms. Unused
 * outputs should be left unconnected.
 *
 * Addressing
 * Data is downloaded in bytes into the 48 by 84 bits RAM
 * data display matrix of PCD8544, as indicated in
 * Figs. 3, 4, 5 and 6. The columns are addressed by the
 * address pointer. The address ranges are: X 0 to 83
 * (1010011), Y 0 to 5 (101). Addresses outside these
 * ranges are not allowed. In the vertical addressing mode
 * (V = 1), the Y address increments after each byte (see
 * Fig.5). After the last Y address (Y = 5), Y wraps around
 * to 0 and X increments to address the next column. In the
 * horizontal addressing mode (V = 0), the X address
 * increments after each byte (see Fig.6). After the last
 * X address (X = 83), X wraps around to 0 and
 * Y increments to address the next row. After the very last
 * address (X = 83 and Y = 5), the address pointers wrap
 * around to address (X = 0 and Y = 0).
 */

display::display(){
	MX_SPI2_Init();
	//GPIO muss in main initialisiert werden

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
	uint8_t dummy = 0;
	HAL_SPI_Transmit(&hspi2, &dummy, 1, 100);

	//Reset display
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(100);

	// get into the EXTENDED mode!
	command(PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION );

	uint8_t bias = 4;
	// LCD bias select (4 is optimal?)
	command(PCD8544_SETBIAS | bias);

	//Set VOP
	uint8_t contrast = 0x2f;//0x7f;
	command( PCD8544_SETVOP | contrast);

	// normal mode
	command(PCD8544_FUNCTIONSET);

	// Set display to Normal
	command(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
}

void display::data(uint8_t* dat, uint16_t len){
	//D/C Pin auf HIGH --> data
	HAL_GPIO_WritePin(LCD_DCMODE_GPIO_Port, LCD_DCMODE_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, dat, len, 100);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void display::command(uint8_t cmd){
	//D/C Pin auf LOW --> Command
	HAL_GPIO_WritePin(LCD_DCMODE_GPIO_Port, LCD_DCMODE_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, & cmd, 1, 100);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void display::transferBuffer(uint8_t* buffer, uint16_t bufferSize){
	uint8_t row;
	command(PCD8544_SETYADDRESS | 0);
	command(PCD8544_SETXADDRESS | 0);
	data(buffer, bufferSize);
	command(PCD8544_SETYADDRESS);
}
