/*
 * I2C.cpp
 *
 *  Created on: Apr 4, 2016
 *      Author: shahada
 */


#include <sstream>
#include <fcntl.h>
#include <fstream>
#include <errno.h>
#include <string.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <syslog.h>
#include <iostream>

#include "I2C.h"

// sudo apt-get install libi2c-dev gives you the *correct* linux/i2c-dev.h


using namespace std;

I2C::I2C(int i2caddr, int bus) {

	if (bus == -1) {
		this->bus = 1;
		ifstream inf;
		inf.open ("/proc/cpuinfo");
		string line;
		while (getline (inf, line)) {
			if (line.substr (0, 8) == "Revision") {
				string revision = line.substr (line.length()-4, 4);
				cout << "[" << revision << "]" << endl;
				if (revision == "0003" || revision == "0002") {
					this->bus = 0;
				}
			}
		}
		inf.close();
	} else {
		this->bus = bus;
	}

	this->i2caddr = i2caddr;

	// open i2c device
	ostringstream buf;
	buf << "/dev/i2c-" << this->bus;
	cout << "Bus dev: [" << buf.str() << "]" << endl;
	i2cdev = open (buf.str().c_str(), O_RDWR|O_NONBLOCK);
	if (i2cdev < 0) {
		ostringstream buf2;
		buf2 << "Failed to open " << buf.str() << ": " << strerror (errno);
		throw runtime_error (buf2.str());
	}

	// select i2c device
	ioctl (i2cdev, I2C_SLAVE, i2caddr);

	ostringstream buf2;
	buf2 << "Opened \"" << buf.str() << "\" for slave " << i2caddr << " as fd " << i2cdev;
	syslog (LOG_DEBUG, buf2.str().c_str());

}



I2C::~I2C() {

	if (i2cdev != -1) {
		close (i2cdev);
	}
}

uint8_t I2C::readRaw() {

	return i2c_smbus_read_byte(i2cdev);
}

void I2C::writeRaw(uint8_t data) {

	i2c_smbus_write_byte(i2cdev, data);
}

void I2C::writeReg8(uint8_t reg, uint8_t data) {

	i2c_smbus_write_byte_data(i2cdev, reg, data);
}

void I2C::writeReg16(uint8_t reg, uint16_t data) {

	i2c_smbus_write_word_data(i2cdev,reg, data);
}


uint8_t I2C::readReg8(uint8_t reg) {

	uint8_t ret = i2c_smbus_read_byte_data(i2cdev, reg);
	return ret;
}


uint16_t I2C::readReg16(uint8_t reg) {

	uint16_t ret = i2c_smbus_read_word_data(i2cdev, reg);
	return ret;
}


uint16_t I2C::readReg16BE (uint8_t reg) {

	// there should be a smbus_read_word_swapped but there isn't.
	// so we swap the data manually.

	uint16_t ret = i2c_smbus_read_word_data(i2cdev, reg);
	ret = (ret  << 8) | (ret >> 8);
	return ret;

}

void I2C::writeReg16BE (uint8_t reg, uint16_t data) {

	// there should be a smbus_write_word_swapped but there isn't.
	// so we swap the data manually.

	data = (data << 8) | (data >> 8);
	i2c_smbus_write_word_data(i2cdev,reg, data);

}


int I2C::readRegBlock (uint8_t reg, uint8_t * data, int count) {

	// smbus standard returns maximum 32 bytes.
	uint8_t buf [32];

	int c = i2c_smbus_read_block_data (i2cdev, reg, buf);

	int rc = min (c, count);

	memcpy (data, buf, rc);

	return rc;

}


int I2C::writeRegBlock (uint8_t reg, uint8_t * data, int count) {

		return i2c_smbus_write_block_data (i2cdev, reg, count, data);
}
