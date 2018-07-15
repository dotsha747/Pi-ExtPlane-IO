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

#include "../common/GPIO.h"

class GPIOProvider {
public:
	GPIOProvider();
	virtual ~GPIOProvider();

	virtual void setDirection (int pin, GPIO::DIR dir) =0;
	virtual void setPullUpDown (int pin, GPIO::PULL hilo) =0;
	virtual void setOutput (int pin, GPIO::STATE hilo) =0;
	virtual GPIO::STATE readInput(int pin) =0;
	virtual void setInterruptHandler(int pin, GPIO::EDGE edge, std::function<void(int pin, GPIO::STATE state)> callback) =0;

};

#endif /* COMMON_GPIOPROVIDER_H_ */
