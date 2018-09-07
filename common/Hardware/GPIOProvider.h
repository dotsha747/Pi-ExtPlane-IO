/*
 * GPIOAbstractProvider.h
 *
 *  Created on: Jul 13, 2018
 *      Author: shahada
 */

#ifndef COMMON_GPIOPROVIDER_H_
#define COMMON_GPIOPROVIDER_H_

#include <cstdint>
#include <functional>

#include "GPIOEnum.h"


class GPIOProvider {
protected:
	int debug = 0;

public:
	GPIOProvider();
	virtual ~GPIOProvider();

	void setDebug (int debug);
	virtual int getAddress ()=0;

	virtual void setDirection (int pin, GPIODIR dir)=0;
	virtual void setPullUpDown (int pin, GPIOPULL hilo)=0;
	virtual void setOutput (int pin, GPIOSTATE hilo)=0;
	virtual GPIOSTATE readInput(int pin)=0;
	virtual uint32_t readInputSequence (int address, int startPin, int endPin)=0;
	virtual void setInterruptHandler(int pin, GPIOEDGE edge,
			std::function<void(int pin, GPIOSTATE state)> callback)=0;
	virtual void clearInterruptHandler(int pin) =0;
	virtual int getPinCount ()=0;

};

#endif /* COMMON_GPIOPROVIDER_H_ */
