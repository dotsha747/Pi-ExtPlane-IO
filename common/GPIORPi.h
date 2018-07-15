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
#include <mutex>
#include <poll.h>

#include "../common/GPIOProvider.h"

#define MAXRPIGPIOCOUNT 26

class GPIORPi: public GPIOProvider {
protected:

	// tracks which GPIOs have been exported (and are in use).
	std::bitset<MAXRPIGPIOCOUNT> isExported;

	// exports the GPIO via sysfs interface
	virtual void exportGPIO (int pin);

	// unexport the GPIO via sysfs interface
	virtual void unexportGPIO (int pin);

	// for mmap gpio control registers
	const int gppud = 37;
	const unsigned long int blocksize = 4*1024;
	const uint8_t gpioToPUDCLK[64] = { 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
			38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
			38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
			39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39 };
	volatile void * gpioptr = NULL;


	// mutex to control access to internal structures
	std::mutex gpioRPiMutex;

	// map of gpio value files. Keep this open for the life of the pin usage
	// to minimize fopen overhead.
	std::map<uint8_t, int> gpioToValueFD;
	std::map<int, uint8_t> valueFDtoGPIO;

	// map of callbacks for each valueFD
	std::map<int, std::function<void(int pin, GPIO::STATE state)> > pinToCallback;

	// set of which pins are interrupting
	std::set<int> interruptPins;
	bool interruptPinsChanged;
	struct pollfd * pollList;
	int pollCount;

	// states of the background thread
	enum class THREADSTATE { RUNNING, STOPPING, STOPPED } threadstate;




public:
	GPIORPi();
	virtual ~GPIORPi();

	virtual void setDirection(int pin, GPIO::DIR dir);
	virtual void setPullUpDown(int pin, GPIO::PULL hilo);
	virtual void setOutput(int pin, GPIO::STATE hilo);
	virtual GPIO::STATE readInput(int pin);
	virtual void setInterruptHandler(int pin, GPIO::EDGE edge, std::function<void(int pin, GPIO::STATE state)> callback);
	virtual void clearInterruptHandler (int pin);


	virtual void cleanup ();

	virtual void waitForInterruptLoop ();

};

#endif /* COMMON_GPIORPI_H_ */
