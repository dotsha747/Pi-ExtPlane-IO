/*
 * MCP23017.cpp
 *
 *  Created on: Jul 22, 2018
 *      Author: shahada
 */

#include <sstream>
#include <iostream>

#include "GPIO.h"
#include "MCP23017.h"

// control address register mapping. Assumes
// IOCON.BANK=0, which is the default state.

#define MCP23x17_IODIRA			0x00
#define MCP23x17_IODIRB			0x01
#define MCP23x17_GPINTENA       0x04
#define MCP23x17_DEFVALA        0x06
#define MCP23x17_INTCONA        0x08
#define MCP23x17_GPINTENB       0x05
#define MCP23x17_DEFVALB        0x07
#define MCP23x17_INTCONB        0x09
#define MCP23x17_IOCON          0x0A
#define MCP23x17_GPPUA			0x0C
#define MCP23x17_GPPUB			0x0D
#define MCP23x17_INTFA          0x0E
#define MCP23x17_INTFB          0x0F
#define MCP23x17_INTCAPA        0x10
#define MCP23x17_INTCAPB        0x11
#define MCP23x17_GPIOA          0x12
#define MCP23x17_GPIOB          0x13

#define IOCON_UNUSED    0x01
#define IOCON_INTPOL    0x02
#define IOCON_ODR       0x04
#define IOCON_HAEN      0x08
#define IOCON_DISSLW    0x10
#define IOCON_SEQOP     0x20
#define IOCON_MIRROR    0x40
#define IOCON_BANK_MODE 0x80

#define	IOCON_SEQOP 	0x20

// our preferred initial values for IOCON register.

#define IOCON_INIT 	(IOCON_MIRROR)

using namespace std;

MCP23017::MCP23017(int i2caddr, int interruptPin, int i2cbus) {

	debug = 0;
	this->interruptPin = interruptPin;
	this->i2caddr = i2caddr;
	i2c = new I2C(i2caddr, i2cbus);

	// released when out of scope
	std::lock_guard<std::mutex> lock(i2cmutex);

	// the MCP23017 starts with IOCON set to 0. This implies
	// runningin IOCON.BANK = 0 mode, where A and B registers
	// are in sequence. Set our init IOCON.

	i2c->writeReg8(MCP23x17_IOCON, IOCON_INIT);

	// set all ports for input
	i2c->writeReg8(MCP23x17_IODIRA, 0xff);
	i2c->writeReg8(MCP23x17_IODIRB, 0xff);

	// disable interrupts on all pins
	i2c->writeReg8(MCP23x17_GPINTENA, 0);
	i2c->writeReg8(MCP23x17_GPINTENB, 0);

	// disable pull ups on all pins
	i2c->writeReg8(MCP23x17_GPPUA, 0);
	i2c->writeReg8(MCP23x17_GPPUB, 0);

	if (interruptPin != -1) {

		// set a RPi interrupt handler on interrupt pin to call
		// the our MCP23017 interruptHandler(). Since default INTPOL=0,
		// the interrupt pin is Active-LOW.

	 GPIO::setDirection(0, interruptPin, GPIODIR::IN);
	 GPIO::setInterruptHandler(0, interruptPin, GPIOEDGE::FALLING,
				std::bind(&MCP23017::interruptHandler, this, placeholders::_1,
						placeholders::_2));

	}

	// dump IOCON
	uint8_t b = i2c->readReg8(MCP23x17_IOCON);
	bitset<8> bs = bitset<8>(b);
	cerr << "IOCON is " << bs.to_string() << endl;
}

MCP23017::~MCP23017() {

	// clear the rpi interrupt handler
	if (interruptPin != -1) {
		GPIO::clearInterruptHandler(0, interruptPin);
	}

	delete i2c;
}

int MCP23017::getPinCount() {

	return 16;
}


int MCP23017::getAddress () {
	return i2caddr;
}

void MCP23017::setDirection(int pin, GPIODIR dir) {

	if (pin >= getPinCount()) {
		ostringstream buf;
		buf << "MCP23017::setDirection (" << pin << ", " << dir << ") failed : "
				<< pin << " is >= " << getPinCount();
		throw runtime_error(buf.str());
	}

	// released when out of scope
	std::lock_guard<std::mutex> lock(i2cmutex);

	// read 16-bit IODIR
	uint16_t iodir = i2c->readReg16(MCP23x17_IODIRA);

	if (dir == GPIODIR::IN) {
		iodir = iodir | (1 << pin);
	} else {
		iodir = iodir & ~(1 << pin);
	}

	// write it back
	i2c->writeReg16(MCP23x17_IODIRA, iodir);

}

void MCP23017::setPullUpDown(int pin, GPIOPULL hilo) {

	if (pin >= getPinCount()) {
		ostringstream buf;
		buf << "MCP23017::setPullUpDown (" << pin << ", " << hilo
				<< ") failed : " << pin << " is >= " << getPinCount();
		throw runtime_error(buf.str());
	}

	// released when out of scope
	std::lock_guard<std::mutex> lock(i2cmutex);

	// read 16-bit GPPU
	uint16_t gppu = i2c->readReg16(MCP23x17_GPPUA);

	if (hilo == GPIOPULL::UP) {
		gppu = gppu | (1 << pin);
	} else if (hilo == GPIOPULL::OFF) {
		gppu = gppu & ~(1 << pin);
	} else {
		ostringstream buf;
		buf << "MCP23017::setPullUpDown (" << pin << ", " << hilo
				<< ") failed : " << hilo << " is unsupported on MCP23017";
		throw runtime_error(buf.str());
	}

	// write it back
	i2c->writeReg16(MCP23x17_GPPUA, gppu);

}

void MCP23017::setOutput(int pin, GPIOSTATE hilo) {

	if (pin >= getPinCount()) {
		ostringstream buf;
		buf << "MCP23017::setOutput (" << pin << ", " << hilo << ") failed : "
				<< pin << " is >= " << getPinCount();
		throw runtime_error(buf.str());
	}

	// released when out of scope
	std::lock_guard<std::mutex> lock(i2cmutex);

	// read 16-bit GPIO
	uint16_t gpio = i2c->readReg16(MCP23x17_GPIOA);

	if (hilo == GPIOSTATE::HI) {
		gpio = gpio | (1 << pin);
	} else {
		gpio = gpio & ~(1 << pin);
	}

	// write it back
	i2c->writeReg16(MCP23x17_GPIOA, gpio);

}

GPIOSTATE MCP23017::readInput(int pin) {

	if (pin >= getPinCount()) {
		ostringstream buf;
		buf << "MCP23017::readInput (" << pin << ") failed : " << pin
				<< " is >= " << getPinCount();
		throw runtime_error(buf.str());
	}

	// released when out of scope
	std::lock_guard<std::mutex> lock(i2cmutex);

	// read 16-bit GPIO
	uint16_t gpio = i2c->readReg16(MCP23x17_GPIOA);

	if ((gpio & (1 << pin)) == 0) {
		return GPIOSTATE::LO;
	} else {
		return GPIOSTATE::HI;
	}
}

void MCP23017::setInterruptHandler(int pin, GPIOEDGE edge,
		std::function<void(int pin, GPIOSTATE state)> callback) {

	if (edge == GPIOEDGE::NONE) {
		clearInterruptHandler(pin);
		return;
	}

	if (pin >= getPinCount()) {
		ostringstream buf;
		buf << "MCP23017::readInput (" << pin << ") failed : " << pin
				<< " is >= " << getPinCount();
		throw runtime_error(buf.str());
	}

	// released when out of scope
	std::lock_guard<std::mutex> lock(i2cmutex);

	// keep track of which pins we have enabled interrupts on.
	interruptEnabled.set(pin);

	// Set GPINTEN. This enables interrupt-on-change.
	uint16_t gpinten = i2c->readReg16(MCP23x17_GPINTENA);
	gpinten = gpinten | (1 << pin);
	i2c->writeReg16(MCP23x17_GPINTENA, gpinten);

	gpinten = i2c->readReg16(MCP23x17_GPINTENA);

	if (edge == GPIOEDGE::BOTH) {

		// if EDGE is BOTH, then we trip on *any* change, and
		// should set the INTCON bit to 0.

		uint16_t intcon = i2c->readReg16(MCP23x17_INTCONA);
		intcon = intcon & ~(1 << pin);
		i2c->writeReg16(MCP23x17_INTCONA, intcon);

		// in this case, DEFVAL doesn't matter.

	} else {

		// if EDGE is RISING or FALLING, then set the INTCON
		// bit to 1.
		uint16_t intcon = i2c->readReg16(MCP23x17_INTCONA);
		intcon = intcon | (1 << pin);
		i2c->writeReg16(MCP23x17_INTCONA, gpinten);

		// and set DEFVAL. Interrupts are triggered when
		// the new state is different from DEFVAL.
		if (edge == GPIOEDGE::RISING) {

			// RISING: transition from 0 to 1. Set DEFVAL to 0.
			uint16_t defval = i2c->readReg16(MCP23x17_DEFVALA);
			defval = defval & ~(1 << pin);
			i2c->writeReg16(MCP23x17_DEFVALA, defval);

		} else {

			// FALLING: transition from 1 to 0. Set DEFVAL to 1.
			uint16_t defval = i2c->readReg16(MCP23x17_DEFVALA);
			defval = defval | (1 << pin);
			i2c->writeReg16(MCP23x17_DEFVALA, defval);

		}
	}

	// keep a copy of the ISR
	isr[pin] = callback;
}

void MCP23017::clearInterruptHandler(int pin) {

	if (pin >= getPinCount()) {
		ostringstream buf;
		buf << "MCP23017::readInput (" << pin << ") failed : " << pin
				<< " is >= " << getPinCount();
		throw runtime_error(buf.str());
	}

	// released when out of scope
	std::lock_guard<std::mutex> lock(i2cmutex);

	// untrack it.
	interruptEnabled.reset(pin);

	// Clear GPINTEN. This disables interrupt-on-change.
	uint16_t gpinten = i2c->readReg16(MCP23x17_GPINTENA);
	gpinten = gpinten & ~(1 << pin);
	i2c->writeReg16(MCP23x17_GPINTENA, gpinten);

	// leave the isr as is (since we can't nullify it). as long
	// as GPINTEN is 0, it will not trigger an interrupt.
}

void MCP23017::interruptHandler(int interruptPin, enum GPIOSTATE state) {

	if (debug > 0) {
		cout
				<< "MCP23017::interruptHandler - received interrupt from Pi, PiIntPin="
				<< interruptPin << ", state=" << state << endl;
	}

	if (interruptPin == this->interruptPin && state == GPIOSTATE::LO) {

		// released when out of scope
		std::lock_guard<std::mutex> lock(i2cmutex);

		// read INTCAP (The state of GPIO when interrupt occurred)
		uint16_t intcap = i2c->readReg16(MCP23x17_INTCAPA);
		bitset<16> intcapbs = bitset<16>(intcap);

		if (debug > 0) {
			cout << "MCP23017::interruptHandler - INTCAP is "
					<< intcapbs.to_string() << endl;
		}

		for (int bit = 0; bit < 16; bit++) {

			// is it something we enabled interrupts on?
			if (interruptEnabled[bit] == true) {

				if (debug > 0) {
					cout
							<< "MCP23017::interruptHandler - Triggered for MCP23017 pin "
							<< bit << " state "
							<< (intcapbs[bit] == true ?
									GPIOSTATE::HI : GPIOSTATE::LO) << endl;
				}

				// call the interrupt handler
				isr[bit](bit,
						intcapbs[bit] == true ?
								GPIOSTATE::HI : GPIOSTATE::LO);
			}

		}
	}
}

uint32_t MCP23017::readInputSequence (int address, int startPin, int endPin) {

	if (startPin != 0) {
		ostringstream buf;
		buf << "MCP23017::readInputAsWord (" << startPin << ") failed : "
				<< startPin << " must == 0 for MCP23017";
		throw runtime_error(buf.str());
	}

	// released when out of scope
	std::lock_guard<std::mutex> lock(i2cmutex);

	// read 16-bit GPIO
	uint32_t gpio = i2c->readReg16(MCP23x17_GPIOA) >> startPin;

	return gpio;

}



uint16_t MCP23017::readWord(int addr) {

	// released when out of scope
	std::lock_guard<std::mutex> lock(i2cmutex);

	// read 16-bit GPIO
	return i2c->readReg16(addr);

}
