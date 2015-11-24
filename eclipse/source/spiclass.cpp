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

#define MAX_PATH_LEN  40
#define SPI_DEVICE_PATH_BASE "/dev/spidev"

SPIClass::SPIClass()
{
	active = false;
	mode = 0;
	bpw = 0;
	speed = 0;
	fd = -1;
	lsb_first = false;
}

int SPIClass::open(int bus, int channel)
{
	return 1;
}

int SPIClass::close()
{
		return 1;
}

int SPIClass::setClockPolarity(uint8_t pol)
{
	return 1;
}

int SPIClass::setClockPhase(uint8_t phase)
{
	return 1;
}

int SPIClass::setLSBFirst(bool lsb_first)
{
	return 1;
}

int SPIClass::setBitsPerWord(int bits)
{
	return 1;
}

int SPIClass::setSpeed(uint32_t speed)
{
	return 1;
}

int SPIClass::write(uint8_t wbuf[], int len)
{
	return 1;
}

int SPIClass::read(uint8_t rbuf[], int len)
{
	return 1;
}

int SPIClass::xfer1(uint8_t wbuf[], uint8_t rbuf[], int len)
{
	return 1;
}

SPIClass::~SPIClass()
{
	close();
}
