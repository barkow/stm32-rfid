/*
 * display.cpp
 *
 *  Created on: 26.11.2015
 *      Author: barkow
 */

#include "display.h"
#include "spi.h"

display::display(){
	MX_SPI2_Init();
	//GPIO muss in main initialisiert werden

	//Reset display
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);

	// get into the EXTENDED mode!
	command(PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION );

	uint8_t bias = 4;
	// LCD bias select (4 is optimal?)
	command(PCD8544_SETBIAS | bias);

	//Set VOP
	uint8_t contrast = 0x7f;
	command( PCD8544_SETVOP | contrast);

	// normal mode
	command(PCD8544_FUNCTIONSET);

	// Set display to Normal
	command(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
}

void display::data(uint8_t* dat, uint8_t len){
	//D/C Pin auf HIGH --> data
	HAL_GPIO_WritePin(LCD_DCMODE_GPIO_Port, LCD_DCMODE_Pin, GPIO_PIN_SET);
	HAL_SPI_Transmit(&hspi1, dat, len, 100);
}

void display::command(uint8_t cmd){
	//D/C Pin auf LOW --> Command
	HAL_GPIO_WritePin(LCD_DCMODE_GPIO_Port, LCD_DCMODE_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, & cmd, 1, 100);
}
