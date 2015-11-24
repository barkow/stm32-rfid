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

SPIClass::SPIClass()
{
	hspi.Instance = SPI1;
	hspi.Init.Mode = SPI_MODE_MASTER;
	hspi.Init.Direction = SPI_DIRECTION_2LINES;
	hspi.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi.Init.NSS = SPI_NSS_HARD_OUTPUT;
	hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi.Init.TIMode = SPI_TIMODE_DISABLED;
	hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
	hspi.Init.CRCPolynomial = 10;
	HAL_SPI_Init(&hspi);
}

int SPIClass::xfer1(uint8_t wbuf[], uint8_t rbuf[], int len)
{
	if (HAL_SPI_TransmitReceive(&hspi,wbuf, rbuf, len, 100) == HAL_OK){
		return 1;
	}
	else {
		return -1;
	}
}
