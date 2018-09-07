/*
 * MCP23017.h
 *
 *  Created on: Jul 22, 2018
 *      Author: shahada
 */

#ifndef COMMON_MCP23017_H_
#define COMMON_MCP23017_H_

#include "GPIOEnum.h"

#include "GPIOProvider.h"
#include "I2C.h"

#include <bitset>
#include <mutex>

class MCP23017 : public GPIOProvider {
protected:
	I2C * i2c;
	int interruptPin;
	int i2caddr;


	// mutex for all I2C operations on the same I2C device.
	std::mutex i2cmutex;

	// tracks which MCP23017 pins we have enabled interrupts on.
	std::bitset<16> interruptEnabled;

	// ISRs
	std::function<void(int pin, GPIOSTATE state)> isr [16];


	/** @brief MCP23017 interrupt handler.
	 *
	 * handle interrupts on a MCP23017 that has a interrupt pin tied
	 * back to a RPi GPIO. This routine interrogates the MCP23017
	 * and determines which pin(s) caused interrupts, and calls their
	 * respective ISRs.
	 *
	 */

	virtual void interruptHandler (int, enum GPIOSTATE);


public:
	MCP23017(int i2caddr, int interruptPin = -1, int i2cbus = -1);
	virtual ~MCP23017();


	virtual int getAddress ();

	virtual int getPinCount ();

	/** @brief set the direction of a GPIO pin
	 *
	 * @param pin 0-15
	 * @param dir GPIODIR::IN or GPIODIR::OUT
	 */
	virtual void setDirection (int pin, GPIODIR dir);

	/** @brief set the direction of a GPIO pin
	 *
	 * @param pin 0-15
	 * @param dir GPIOPULL::UP or GPIOPULL::OFF
	 */
	virtual void setPullUpDown (int pin, GPIOPULL hilo);

	/** @brief set the state of a GPIO output pin
	 *
	 * @param pin 0-15
	 * @param dir GPIOSTATE::HI or GPIOSTATE::HI
	 */
	virtual void setOutput (int pin, GPIOSTATE hilo);

	virtual GPIOSTATE readInput(int pin);

	/** @brief reads up to 16 pins in sequence.
	 *
	 *	On the MCP23017, startPin must always be zero. It returns
	 *	all pins status, with pin 0 being lsb.
	 *
	*/
	virtual uint32_t readInputSequence (int address, int startPin, int endPin);

	virtual void setInterruptHandler(int pin, GPIOEDGE edge,
			std::function<void(int pin, GPIOSTATE state)> callback);
	virtual void clearInterruptHandler(int pin);



	virtual uint16_t readWord (int addr);



};

#endif /* COMMON_MCP23017_H_ */
