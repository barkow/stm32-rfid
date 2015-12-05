/*
 * SPI.cpp
 *
 *  Created on: Jun 2, 2013
 *      Author: jacek
 */

#include "spiclass.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "spi.h"
#include "stm32f1xx_hal.h"

SPIClass::SPIClass()
{
	//MX_SPI1_Init();

}

void SPIClass::setDataMode(uint8_t mode){
	HAL_GPIO_WritePin(RFID_CS_GPIO_Port, RFID_CS_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(RFID_RESET_GPIO_Port, RFID_RESET_Pin, GPIO_PIN_RESET);
		HAL_Delay(50);
		//dummy transmit um clk leitung auf richtigen pegel zu bekommen
		uint8_t dat = 0xaa;
		HAL_SPI_Transmit(&hspi1, &dat, 1, 10);
		HAL_GPIO_WritePin(RFID_RESET_GPIO_Port, RFID_RESET_Pin, GPIO_PIN_SET);
		HAL_Delay(100);

}

int SPIClass::xfer1(uint8_t wbuf[], uint8_t rbuf[], int len)
{
	HAL_StatusTypeDef status;
	HAL_GPIO_WritePin(RFID_CS_GPIO_Port, RFID_CS_Pin, GPIO_PIN_RESET);
	status = HAL_SPI_TransmitReceive(&hspi1,wbuf, rbuf, len, 100);
	HAL_GPIO_WritePin(RFID_CS_GPIO_Port, RFID_CS_Pin, GPIO_PIN_SET);
	if (status == HAL_OK){
		return 1;
	}
	else {
		return -1;
	}
}
