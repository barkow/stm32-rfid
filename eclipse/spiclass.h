/*
 * SPI.h
 *
 *  Created on: Jun 2, 2013
 *      Author: jacek
 */

#include <stdint.h>
#include <linux/spi/spidev.h>

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
	private:
		uint8_t mode;
		uint8_t bpw;
		bool lsb_first;
		uint32_t speed;
		bool active;
		int fd;
	public:

		/**
		 * Default constructor for SPIClass class.
		 */
		SPIClass();

		/**
		 * Function opens the SPI device.
		 * @param bus
		 * @param channel
		 * @return
		 */
		int open(int bus, int channel);

		/**
		 * Function closes the SPI device.
		 * @return
		 */
		int close();

		/**
		 * Functions sets the idle level and active edge of the clock signal.
		 * @param mode
		 * @return
		 */
		int setMode(uint8_t mode);
		void setDataMode(uint8_t mode) {setMode(mode);};
		/**
		 * Function sets idle level for the clock signal.
		 * @param pol
		 * @return
		 */
		int setClockPolarity(uint8_t pol);

		/**
		 *  Function sets active edge for the clock signal.
		 * @param phase
		 * @return
		 */
		int setClockPhase(uint8_t phase);

		/**
		 * Function sets order of bits in transmitted data.
		 * @param lsb_first
		 * @return
		 */
		int setLSBFirst(bool lsb_first);
		void setBitOrder(uint8_t order){setLSBFirst(order == LSBFIRST);};
		
		/**
		 * Function sets the number of bits per transferred word.
		 * @param bits
		 * @return
		 */
		int setBitsPerWord(int bits);

		/**
		 * Function set the speed of the SPI interface.
		 * @param speed
		 * @return
		 */
		int setSpeed(uint32_t speed);


		/**
		 * Method writes len bytes to the device.
		 * @param wbuf
		 * @param len
		 * @return
		 */
		int write(uint8_t wbuf[], int len);

		/**
		 * Method reads len bytes from the device.
		 * @param rbuf
		 * @param len
		 * @return
		 */
		int read(uint8_t rbuf[], int len);

		/**
		 * Function performs simultaneous read and write on the device.
		 * @param wbuf
		 * @param rbuf
		 * @param len
		 * @return
		 */
		int xfer1(uint8_t wbuf[], uint8_t rbuf[], int len);
		uint8_t transfer(uint8_t _data){uint8_t wbuff = _data; uint8_t rbuff; xfer1(&wbuff, &rbuff, 1); return rbuff;};

		/**
		 *
		 */
		virtual ~SPIClass();
};

#endif /* SPI_H_ */