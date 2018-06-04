/*
 * SPI.h
 *
 *  Created on: Dec 3, 2016
 *      Author: shahada
 */

#ifndef SPI_H_
#define SPI_H_

#include <cstdint>

using namespace std;

class SPI {
	int cs;
	int spidev;

	uint8_t spiBitsPerWord;
	uint32_t spiSpeed;
	uint16_t spiDelayUsecs;
	uint8_t mode;		// 0,1,2,3

public:
	SPI(int cs, int bus = 0);
	virtual ~SPI();

	void setSpeed (uint32_t speed);
	void setDelay (uint16_t delay);
	void setBitsPerWord (uint8_t bits);
	void setMode (uint8_t mode);


	int read (uint8_t * buffer, int count);
	int write (uint8_t * buffer, int count);

	int writeRead (uint8_t * writeBuf, uint8_t * readBuf, int count);
};

#endif /* SPI_H_ */
