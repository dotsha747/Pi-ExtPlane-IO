/*
 * GPIORPi.h
 *
 *	handler for raspberry pi onboard gpio.
 *
 *	Pin numbers are BCM numbers.
 *
 *	There should only ever be one instance of this ever created. Or at least if you
 *	do create multiple instances, make sure they don't both operate on the same pin.
 *
 *
 *
 *  Created on: Jul 13, 2018
 *      Author: shahada
 */

#ifndef COMMON_GPIORPI_H_
#define COMMON_GPIORPI_H_

#include <bitset>
#include <map>
#include <set>
#include <vector>
#include <mutex>
#include <poll.h>
#include <atomic>

#include "GPIOProvider.h"

#define MAXRPIGPIOCOUNT 26

class GPIORPi: public GPIOProvider {
protected:


	static void atExitHandler();

	// exports the GPIO via sysfs interface
	virtual void exportGPIO(int pin);

	// unexport the GPIO via sysfs interface
	virtual void unexportGPIO(int pin);

	// for mmap gpio control registers
	const int gppud = 37;
	const unsigned long int blocksize = 4 * 1024;
	const uint8_t gpioToPUDCLK[64] = { 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
			38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
			38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
			39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
			39, 39, 39 };
	volatile void * gpioptr = NULL;

	// mutex to control access to internal structures gpioToValueFD and valueFDtoGPIO
	// locked whhenever any function makes use of isExported (almost everything),
	// gpioToValueFD or vallueFDtoGPIO.
	std::mutex gpioRPiMutex;

	// tracks which GPIOs have been exported (and are in use).
	std::bitset<MAXRPIGPIOCOUNT> isExported;
	std::map<uint8_t, int> pinToValueFD;
	std::map<int, uint8_t> valueFDtoPin;

	// mutex to control access to mutex structures used by interrupt loop.
	// locked whenever interrupt loop body runs, and also when interrupt list
	// is regenerated during exportGPIO, unexportGPIO, setInterruptHandler and
	// clearInterruptHandler.
	std::mutex isrDataMutex;

	// interrupt handler data
	std::vector<int> pollPins;
	struct pollfd pollList[MAXRPIGPIOCOUNT];
	std::vector<std::function<void(int pin, GPIOSTATE state)>> pollCallbacks;

	enum class THREADSTATE { Running, Stopping, Stopped};
	THREADSTATE threadstate;

public:

	GPIORPi();
	virtual ~GPIORPi();

	virtual void shutdown();
	virtual int getAddress ();

	int getPinCount ();
	virtual void setDirection(int pin, GPIODIR dir);
	virtual void setPullUpDown(int pin, GPIOPULL hilo);
	virtual void setOutput(int pin, GPIOSTATE hilo);
	virtual GPIOSTATE readInput(int pin);
	virtual uint32_t readInputSequence (int address, int startPin, int endPin);
	virtual void setInterruptHandler(int pin, GPIOEDGE edge,
			std::function<void(int pin, GPIOSTATE state)> callback);
	virtual void clearInterruptHandler(int pin);
	virtual void waitForInterruptLoop();


};

#endif /* COMMON_GPIORPI_H_ */
