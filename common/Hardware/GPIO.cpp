/*
 * GPIO.cpp
 *
 *  Created on: Jul 13, 2018
 *      Author: shahada
 */

#include <sstream>
#include <stdexcept>

#include "GPIO.h"

using namespace std;

// init static attributes
std::map<int, GPIOProvider *> GPIO::gpioProviders;

void GPIO::init() {

	// raspberry pi is always at address 0. Others will have to
	// attached using attachGPIOProvider().
	gpioProviders[0] = new GPIORPi();

}

void GPIO::shutdown() {

	// TODO: remove all the gpio providers
}


void GPIO::setDebug(int debug) {

	for (auto i = gpioProviders.begin(); i != gpioProviders.end(); i++) {
		i->second->setDebug(debug);
	}

}

void GPIO::attachGPIOProvider(GPIOProvider * gpioProvider) {

	int address = gpioProvider->getAddress();

	if (gpioProviders.find(address) != gpioProviders.end()) {

		ostringstream buf;
		buf << "GPIO::attachGPIOPRovider (" << address
				<< ", ...) failed, address " << address << " already attached";
		throw runtime_error(buf.str());
	}

	gpioProviders[address] = gpioProvider;
}

GPIOProvider * GPIO::getGPIOProvider(int address) {

	if (gpioProviders.find(address) != gpioProviders.end()) {

		ostringstream buf;
		buf << "GPIO::attachGPIOPRovider (" << address
				<< ", ...) failed, address " << address << " already attached";
		throw runtime_error(buf.str());
	}

	return gpioProviders[address];
}

void GPIO::setDirection(int address, int pin, GPIODIR dir) {

	if (gpioProviders.find(address) == gpioProviders.end()) {
		ostringstream buf;
		buf << "GPIO::setDirection (" << address << ", ...) failed as "
				<< address << " doesn't refer to an attached handler";
		throw runtime_error(buf.str());
	}

	gpioProviders[address]->setDirection(pin, dir);
}

void GPIO::setPullUpDown(int address, int pin, GPIOPULL hilo) {

	if (gpioProviders.find(address) == gpioProviders.end()) {
		ostringstream buf;
		buf << "GPIO::setPullUpDown (" << address << ", ...) failed as "
				<< address << " doesn't refer to an attached handler";
		throw runtime_error(buf.str());
	}

	gpioProviders[address]->setPullUpDown(pin, hilo);
}

void GPIO::setOutput(int address, int pin, GPIOSTATE hilo) {

	if (gpioProviders.find(address) == gpioProviders.end()) {
		ostringstream buf;
		buf << "GPIO::setOutput (" << address << ", ...) failed as " << address
				<< " doesn't refer to an attached handler";
		throw runtime_error(buf.str());
	}

	gpioProviders[address]->setOutput(pin, hilo);
}

GPIOSTATE GPIO::readInput(int address, int pin) {

	if (gpioProviders.find(address) == gpioProviders.end()) {
		ostringstream buf;
		buf << "GPIO::setOutput (" << address << ", ...) failed as " << address
				<< " doesn't refer to an attached handler";
		throw runtime_error(buf.str());
	}

	return gpioProviders[address]->readInput(pin);

}

uint32_t GPIO::readInputSequence (int address, int startPin, int endPin) {

	if (gpioProviders.find(address) == gpioProviders.end()) {
		ostringstream buf;
		buf << "GPIO::setOutput (" << address << ", ...) failed as " << address
				<< " doesn't refer to an attached handler";
		throw runtime_error(buf.str());
	}

	return gpioProviders[address]->readInputSequence (address, startPin, endPin);
}

void GPIO::setInterruptHandler(int address, int pin, GPIOEDGE edge,
		std::function<void(int pin, GPIOSTATE state)> callback) {

	if (gpioProviders.find(address) == gpioProviders.end()) {
		ostringstream buf;
		buf << "GPIO::setInterruptHandler (" << address << ", ...) failed as "
				<< address << " doesn't refer to an attached handler";
		throw runtime_error(buf.str());
	}

	gpioProviders[address]->setInterruptHandler(pin, edge, callback);
}

void GPIO::clearInterruptHandler(int address, int pin) {

	if (gpioProviders.find(address) == gpioProviders.end()) {
		ostringstream buf;
		buf << "GPIO::clearInterruptHandler (" << address << ", ...) failed as "
				<< address << " doesn't refer to an attached handler";
		throw runtime_error(buf.str());
	}

	gpioProviders[address]->clearInterruptHandler(pin);
}

int GPIO::getPinCount(int address) {

	if (gpioProviders.find(address) == gpioProviders.end()) {
		ostringstream buf;
		buf << "GPIO::getPinCount (" << address << ", ...) failed as "
				<< address << " doesn't refer to an attached handler";
		throw runtime_error(buf.str());
	}

	return gpioProviders[address]->getPinCount();
}

