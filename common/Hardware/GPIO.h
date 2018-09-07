/*
 * GPIO.h
 *
 * Singleton object that abstracts all GPIOs.
 *
 * When initialized it reads hardware.xml to determine what GPIO hardware is available.
 *
 * The underlying GPIO hardware currently can be of two types:
 * - raspberry pi internal GPIO
 * - MCP23017 GPIOs on the i2c bus
 *
 *
 *
 * GPIOs are accessed via a "expander address" and "Gpio number".
 *
 * Two types of GPIOs are supported ... that of the raspberry pi and also
 * those provided by MCP23017 expander chips.
 *
 *  Created on: Jul 13, 2018
 *      Author: shahada
 */

#ifndef COMMON_GPIO_H_
#define COMMON_GPIO_H_

#include <ostream>
#include <map>

#include "GPIOEnum.h"
#include "GPIORPi.h"

class gpioProviders;

class GPIO {
protected:

	// pointers to all our GPIO handlers.
	static std::map<int, GPIOProvider *> gpioProviders;

public:

	static void init();
	static void shutdown();

	static void attachGPIOProvider(GPIOProvider * gpioProvider);
	static GPIOProvider * getGPIOProvider(int address);

	static void setDebug(int debug);

	// pass these through to GPIO Provider methods
	static void setDirection(int address, int pin, GPIODIR dir);
	static void setPullUpDown(int address, int pin, GPIOPULL hilo);
	static void setOutput(int address, int pin, GPIOSTATE hilo);
	static GPIOSTATE readInput(int address, int pin);
	static uint32_t readInputSequence (int address, int startPin, int endPin);
	static void setInterruptHandler(int address, int pin, GPIOEDGE edge,
			std::function<void(int pin, GPIOSTATE state)> callback);
	static void clearInterruptHandler(int address, int pin);
	static int getPinCount(int address);

};

#endif /* COMMON_GPIO_H_ */
