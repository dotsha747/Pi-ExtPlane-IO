/*
 * GPIORPi.cpp
 *
 *  Created on: Jul 13, 2018
 *      Author: shahada
 */

#include "../common/GPIORPi.h"

#include <sstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <thread>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <tuple>

using namespace std;

GPIORPi::GPIORPi() {

	// map gpio control registers to memory. We need this to be able to set
	// built-in pull up/down resistors, which are not controllable via
	// sysfs.

	int fd;
	if ((fd = open("/dev/gpiomem", O_RDWR | O_SYNC | O_CLOEXEC)) < 0) {
		throw runtime_error("Unable to open /dev/gpiomem");
	}

	// Set the offsets into the memory interface.

	gpioptr = mmap(0, blocksize, PROT_READ | PROT_WRITE,
	MAP_SHARED, fd, 0x00200000);
	if ((int32_t) gpioptr == -1) {
		throw runtime_error("GPIORPi mmap failed");
	};

	// run "wait for interrupt loop" in a separate thread
	threadstate = THREADSTATE::RUNNING;
	std::thread t(&GPIORPi::waitForInterruptLoop, this);
	t.detach();

}

GPIORPi::~GPIORPi() {
	cleanup();
}

void GPIORPi::cleanup() {

	// stop the interrupt handling thread
	threadstate = THREADSTATE::STOPPING;
	int count = 0;
	do {
		usleep(1000); // sleep 1 m/s
	} while (count++ < 1000 && threadstate != THREADSTATE::STOPPED);

	// unexport all exported pins
	for (int i = 0; i <= MAXRPIGPIOCOUNT; i++) {
		if (isExported[i]) {
			unexportGPIO(i);
		}
	}

	// undo mmap
	munmap((void *) gpioptr, blocksize);
}



void GPIORPi::setDirection(int pin, GPIO::DIR dir) {

	if (debug) {
		cout << "setDirection (" << pin << ")" << endl;
	}

	// lock for entire scope
	std::lock_guard<std::mutex> lock(gpioRPiMutex);

	if (!isExported[pin]) {
		exportGPIO(pin);
	}

	ostringstream buf;
	buf << "/sys/class/gpio/gpio" << pin << "/direction";

	ofstream f(buf.str());
	if (!f.is_open()) {
		throw runtime_error("Failed to " + buf.str());
	}
	if (dir == GPIO::DIR::IN) {
		f << "in";
	} else {
		f << "out";
	}
	if (f.bad()) {
		throw runtime_error("Failed to write to " + buf.str());
	}
	f.close();

	// do a dummy read.
	if (dir == GPIO::DIR::IN) {
		readInput(pin);
	}
}

void GPIORPi::setPullUpDown(int pin, GPIO::PULL upDown) {

	if (debug) {
		cout << "setPullUpDown (" << pin << ")" << endl;
	}

	// lock for entire scope
	std::lock_guard<std::mutex> lock(gpioRPiMutex);

	if (!isExported[pin]) {
		exportGPIO(pin);
	}

	// Bummer, there is no sysfs mechanism for setting pull ups. So we have to access
	// the GPIO registers directly.

	int pud = 0;
	switch (upDown) {
	case GPIO::PULL::OFF:
		pud = 0;
		break;
	case GPIO::PULL::DOWN:
		pud = 1;
		break;
	case GPIO::PULL::UP:
		pud = 2;
		break;
	};

	*((uint32_t *) gpioptr + gppud) = pud & 3;
	usleep(5);
	*((uint32_t *) gpioptr + gpioToPUDCLK[pin]) = 1 << (pin & 31);
	usleep(5);
	*((uint32_t *) gpioptr + gppud) = 0;
	usleep(5);
	*((uint32_t *) gpioptr + gpioToPUDCLK[pin]) = 0;
	usleep(5);

}

void GPIORPi::setOutput(int pin, GPIO::STATE hilo) {

	if (debug) {
		cout << "setOutput(" << pin << ")" << endl;
	}

	// lock for entire scope
	std::lock_guard<std::mutex> lock(gpioRPiMutex);

	if (!isExported[pin]) {
		exportGPIO(pin);
	}

	int rc;
	if (hilo == GPIO::STATE::HI) {
		rc = write(pinToValueFD[pin], "1\n", 2);
	} else {
		rc = write(pinToValueFD[pin], "0\n", 2);
	}

	if (rc != 2) {
		ostringstream buf;
		buf << "Failed to write to /sys/class/gpio/gpio" << pin << "/value";
		throw runtime_error(buf.str());
	}
}

GPIO::STATE GPIORPi::readInput(int pin) {

	if (debug) {
		cout << "readInput (" << pin << ")" << endl;
	}

	if (!isExported[pin]) {
		exportGPIO(pin);
	}

	int fd = pinToValueFD[pin];

	lseek(fd, 0, SEEK_SET);

	char state;
	int rc = read(fd, &state, 1);

	if (rc != 1) {
		ostringstream buf;
		buf << "Failed to read from /sys/class/gpio/gpio" << pin << "/value";
		throw runtime_error(buf.str());
	};

	return state == '1' ? GPIO::STATE::HI : GPIO::STATE::LO;
}

void GPIORPi::setInterruptHandler(int pin, GPIO::EDGE edge,
		std::function<void(int pin, GPIO::STATE state)> callback) {

	if (debug) {
		cout << "setInterruptHandler (" << pin << ")" << endl;
	}

	if (edge == GPIO::EDGE::NONE) {
		clearInterruptHandler (pin);
		return;
	}

	// lock for entire scope
	std::lock_guard<std::mutex> lock(gpioRPiMutex);

	// make sure pin is exported
	if (isExported[pin] == 0) {
		ostringstream buf;
		buf << "setInterruptHandler() called for unexported pin " << pin;
		throw runtime_error (buf.str());
	}

	// lock the ISR Mutex
	isrDataMutex.lock ();

	int fd = pinToValueFD [pin];

	bool found = false;
	for (unsigned int i = 0; i < pollPins.size(); i++) {
		if (pollPins[i] == pin) {
			// replace existing one
			pollCallbacks[i] = callback;
			found = true;
		}
	}

	if (! found) {
		// append new one
		pollList[pollPins.size()].fd = fd;
		pollList[pollPins.size()].events = POLLPRI | POLLERR;
		pollPins.push_back (pin);
		pollCallbacks.push_back (callback);
	}

	isrDataMutex.unlock ();

	// make the pin interrupt-enabled
	ostringstream buf;
	buf << "/sys/class/gpio/gpio" << pin << "/edge";
	ofstream f(buf.str());
	if (!f.is_open()) {
		throw runtime_error("Failed to open for writing " + buf.str());
	}
	if (edge == GPIO::EDGE::BOTH) {
		f << "both";
	} else if (edge == GPIO::EDGE::FALLING) {
		f << "falling";
	} else if (edge == GPIO::EDGE::RISING) {
		f << "rising";
	} else {
		f << "none";
	}
	if (f.bad()) {
		throw runtime_error("Failed to write to " + buf.str());
	};
	f.close();

}


void GPIORPi::clearInterruptHandler(int pin) {

	if (debug) {
		cout << "clearInterruptHandler (" << pin << ")" << endl;
	}

	// lock for entire scope
	std::lock_guard<std::mutex> lock(gpioRPiMutex);

	// make sure pin is exported
	if (isExported[pin] == 0) {
		ostringstream buf;
		buf << "clearInterruptHandler() called for unexported pin " << pin;
		throw runtime_error (buf.str());
	}

	// lock for entire scope
	std::lock_guard<std::mutex> lock2(isrDataMutex);

	// remove the array element
	bool found = false;
	for (unsigned int i = 0; i < pollPins.size()-1; i++) {

		if (pollPins[i] == pin) {
			found = true;
		}

		if (found) {
			// shift next element to current
			pollList [i] = pollList[i+1];
			pollPins [i] = pollPins[i+1];
			pollCallbacks[i] = pollCallbacks[i+1];
		}
	}

	// truncate the array by 1. Leave pollList as-is.
	pollPins.pop_back();
	pollCallbacks.pop_back();

	// set edge to none
	ostringstream buf;
	buf << "/sys/class/gpio/gpio" << pin << "/edge";
	ofstream f(buf.str());
	if (!f.is_open()) {
		throw runtime_error("Failed to open for writing " + buf.str());
	}
	f << "none";
	if (f.bad()) {
		throw runtime_error("Failed to write to " + buf.str());
	};
	f.close();


}

/** @brief exports a GPIO via sysfs
 *
 * 	This should be called before dealing with any GPIO.
 *
 * @param pin
 */

void GPIORPi::exportGPIO(int pin) {

	if (debug) {
		cout << "exportGPIO (" << pin << ")" << endl;
	}

	// assume the caller has locked the mutex, so we don't do it here.

	ofstream f("/sys/class/gpio/export");
	if (!f.is_open()) {
		throw runtime_error("Failed to open /sys/class/gpio/export");
	}
	f << pin;
	f.close();

	ostringstream valueFilePath;
	valueFilePath << "/sys/class/gpio/gpio" << pin << "/value";

	// sysfs can take some time to make the gpioNN available. Keep checking every 1 m/s
	int tries = 5000;
	int fd = -1;
	do {
		usleep(1000);
		// open the gpio value file so we can use it later.
		fd = open(valueFilePath.str().c_str(), O_RDWR);
	} while (fd == -1 && tries-- > 0);
	if (fd == -1) {
		throw runtime_error("Gave up waiting for GPIO to be exported");
	}

	// create records
	pinToValueFD.insert (pair<uint8_t, int> (pin, fd));
	valueFDtoPin.insert (pair<int, uint8_t> (fd, pin));
	isExported.set(pin);

}

void GPIORPi::unexportGPIO(int pin) {

	if (debug) {
		cout << "unexportGPIO (" << pin << ")" << endl;
	}

	// assume the caller has locked the mutex, so we don't do it here.

	if (!isExported[pin]) {
		ostringstream buf;
		buf << "Can't unexport RPi GPIO pin " << pin << " as not exported";
		throw runtime_error(buf.str());
	}

	setDirection(pin, GPIO::DIR::IN);

	// keep this for later
	int fd = pinToValueFD[pin];

	// remove the record

	clearInterruptHandler (pin);

	pinToValueFD.erase (pin);
	valueFDtoPin.erase (fd);
	isExported.reset(pin);



	// close the corresponding gpioValueFile
	close(fd);

	// unexport
	ofstream f("/sys/class/gpio/unexport");
	if (!f.is_open()) {
		throw runtime_error("Failed to open /sys/class/gpio/unexport");
	}
	f << pin;
	f.close();

}

/** @brief loop waiting for interrupt to trigger */

void GPIORPi::waitForInterruptLoop() {

	if (debug) {
		cout << "IN waitforInterruptLoop()" << endl;
	}

	while (threadstate == THREADSTATE::RUNNING) {

		// lock the ISRData mutex for pretty much the duration of the loop.
		// anything wanting to change the ISR data needs to squeeze itself
		// in at the end of the loop.

		isrDataMutex.lock();

		// do we have anything to monitor?
		if (pollPins.size() == 0) {
			isrDataMutex.unlock();
			sleep(1);
			continue;
		}

		int ret = poll(pollList, pollPins.size(), 10);

		if (ret == -1) {
			ostringstream buf;
			buf << "in waitForInterruptLoop() poll() returned " << ret << ": "
					<< strerror(errno);
			isrDataMutex.unlock();
			throw runtime_error(buf.str());
		}

		if (ret != 0) {

			// check results
			vector< tuple<std::function<void(int pin, GPIO::STATE state)>, int, GPIO::STATE>> triggeredPins;
			for (unsigned int i = 0; i < pollPins.size(); i++) {

				if ( pollList[i].revents != 0) {

					int fd = pollList[i].fd;

					lseek(fd, 0, SEEK_SET);
					char state;
					int rc = read(fd, &state, 1);
					if (rc != 1) {
						ostringstream buf;
						buf << "Failed to read from /sys/class/gpio/gpio"
								<< pollPins[i] << "/value; errno=" << errno << " : "
								<< strerror(errno);
						throw runtime_error(buf.str());
					}

					// record it for later
					triggeredPins.push_back(
							tuple<std::function<void(int pin, GPIO::STATE state)>, int, GPIO::STATE>
								(
									pollCallbacks[i],
									pollPins[i],
									state == '1' ?
											GPIO::STATE::HI : GPIO::STATE::LO
								));
				}
			}

			isrDataMutex.unlock();

			// now that we've freed the mutex, call each of the triggered exceptions
			for (auto t = triggeredPins.begin(); t != triggeredPins.end();
					t++) {
				get<0>(*t) (get<1>(*t), get<2> (*t));
			}
		} else {
			isrDataMutex.unlock();
		}
	} // still running

	threadstate = THREADSTATE::STOPPED;

}
