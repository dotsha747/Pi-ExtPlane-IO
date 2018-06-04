/*
 * SPI.cpp
 *
 *  Created on: Dec 3, 2016
 *      Author: shahada
 */

#include <cstdint>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sstream>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#include "SPI.h"

using namespace std;


SPI::SPI(int cs, int bus) {

	this->cs= cs;

	ostringstream buf;
	buf << "/dev/spidev" << bus << "." << cs;
	spidev = open (buf.str().c_str(), O_RDWR);
	if (spidev < 0) {
		syslog (LOG_CRIT, "Unable to open %s: %s", buf.str().c_str(), strerror(errno));
		cerr << "Unable to open " << buf.str() << ": " << strerror(errno) << endl;
	}
	cout << "Opened " << buf.str() << " as fd " << spidev << endl;

	setSpeed (500000);
	setBitsPerWord (8);
	setDelay (0);
	setMode (0);


}

SPI::~SPI() {
	if (spidev >= 0) {
		close (spidev);
	}
}



int SPI::read (uint8_t * buf, int count) {

	return ::read (spidev, buf, count);

}

int SPI::write (uint8_t * buf, int count) {

	return ::write (spidev, buf, count);
}


void SPI::setSpeed (uint32_t speed) {
	this->spiSpeed = speed;

	ioctl (spidev, SPI_IOC_WR_MAX_SPEED_HZ, &this->spiSpeed);
}

void SPI::setDelay (uint16_t delay) {
	this->spiDelayUsecs = delay;

}

void SPI::setBitsPerWord (uint8_t bits) {
	this->spiBitsPerWord = bits;

	ioctl (spidev, SPI_IOC_WR_BITS_PER_WORD, &this->spiBitsPerWord);
}

void SPI::setMode (uint8_t mode) {
	this->mode = mode;

	ioctl (spidev, SPI_IOC_WR_MODE, &this->mode);
}



int SPI::writeRead (uint8_t * writeBuf, uint8_t * readBuf, int count) {

	struct spi_ioc_transfer xfer;
	memset (&xfer, 0, sizeof (xfer));

	xfer.tx_buf = (uint64_t) writeBuf;
	xfer.rx_buf = (uint64_t) readBuf;
	xfer.len = count;
	xfer.delay_usecs = spiDelayUsecs;
	xfer.speed_hz = spiSpeed;
	xfer.bits_per_word = spiBitsPerWord;


	return ioctl (spidev, SPI_IOC_MESSAGE(1), &xfer);

}
