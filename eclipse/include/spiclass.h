/*
 * SPI.h
 *
 *  Created on: Jun 2, 2013
 *      Author: jacek
 */

#include <stdint.h>

#ifndef SPI_H_
#define SPI_H_

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define LSBFIRST 0
#define MSBFIRST 1

class SPIClass
{
	public:

		/**
		 * Default constructor for SPIClass class.
		 */
		SPIClass();

		/**
		 * Functions sets the idle level and active edge of the clock signal.
		 * @param mode
		 * @return
		 */
		void setDataMode(uint8_t mode) {};

		/**
		 * Function sets order of bits in transmitted data.
		 * @param lsb_first
		 * @return
		 */
		void setBitOrder(uint8_t order){};
		
		/**
		 * Function performs simultaneous read and write on the device.
		 * @param wbuf
		 * @param rbuf
		 * @param len
		 * @return
		 */
		int xfer1(uint8_t wbuf[], uint8_t rbuf[], int len);
		uint8_t transfer(uint8_t _data){uint8_t wbuff = _data; uint8_t rbuff; xfer1(&wbuff, &rbuff, 1); return rbuff;};

};

#endif /* SPI_H_ */
