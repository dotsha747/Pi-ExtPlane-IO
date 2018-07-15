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

class GPIO {



public:

	enum class DIR { IN, OUT };
	enum class STATE {LO, HI };
	enum class PULL {OFF, UP, DOWN };
	enum class EDGE {NONE, RISING, FALLING, BOTH};

	GPIO();
	virtual ~GPIO();
};

#endif /* COMMON_GPIO_H_ */
