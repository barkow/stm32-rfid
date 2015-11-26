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
	MX_SPI1_Init();
}

int SPIClass::xfer1(uint8_t wbuf[], uint8_t rbuf[], int len)
{
	if (HAL_SPI_TransmitReceive(&hspi1,wbuf, rbuf, len, 100) == HAL_OK){
		return 1;
	}
	else {
		return -1;
	}
}
