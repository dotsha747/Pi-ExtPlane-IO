/*
 * MCP3208.cpp
 *
 *  Created on: Dec 3, 2016
 *      Author: shahada
 */

#include "../testADC/MCP3208.h"

#include <iostream>
#include <iomanip>


MCP3208::MCP3208(int cs, int bus) {

	spi = new SPI (cs, bus);

}



MCP3208::~MCP3208() {

	delete spi;
}


uint16_t MCP3208::AnalogRead (int port, bool isSingleEnded) {

	uint8_t buf [3];
	buf[0] = 4 | (isSingleEnded ? 1 : 0) << 1 | (port & 0x7) >> 2;
	buf[1] = (port & 0x7) << 6;
	buf[2] = 0;
	//cout << "Write:" << setfill('0') << setw(2) << hex << (uint16_t) buf [0] << setw(2) << hex << (uint16_t) buf[1] << setw(2) << hex << (uint16_t) buf[2] << endl;

	spi->writeRead(buf, buf, sizeof(buf));

	//cout << "Read:" << setfill('0') << setw(2) << hex << (uint16_t) buf [0] << setw(2) << hex << (uint16_t) buf[1] << setw(2) << hex << (uint16_t) buf[2] << endl;

	return ( ((buf[1] & 0xf) << 8) | buf[2]);

}
