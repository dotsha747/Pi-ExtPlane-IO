/*
 * MCP3208.h
 *
 *  Created on: Dec 3, 2016
 *      Author: shahada
 */

#ifndef MCP3208_H_
#define MCP3208_H_

#include "SPI.h"


class MCP3208 {

	SPI * spi;

public:
	MCP3208(int cs, int bus = 0);
	virtual ~MCP3208();

	uint16_t AnalogRead (int port, bool isSingleEnded);
};

#endif /* MCP3208_H_ */
