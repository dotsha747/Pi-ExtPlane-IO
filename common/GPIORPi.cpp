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

	pollList = (struct pollfd *) malloc(
	MAXRPIGPIOCOUNT * sizeof(struct pollfd));

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

	free(pollList);

	// undo mmap
	munmap((void *) gpioptr, blocksize);
}

void GPIORPi::setDirection(int pin, GPIO::DIR dir) {

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

	// lock for entire scope
	std::lock_guard<std::mutex> lock(gpioRPiMutex);

	if (!isExported[pin]) {
		exportGPIO(pin);
	}

	int rc;
	if (hilo == GPIO::STATE::HI) {
		rc = write(gpioToValueFD[pin], "1\n", 2);
	} else {
		rc = write(gpioToValueFD[pin], "0\n", 2);
	}

	if (rc != 2) {
		ostringstream buf;
		buf << "Failed to write to /sys/class/gpio/gpio" << pin << "/value";
		throw runtime_error(buf.str());
	}
}

GPIO::STATE GPIORPi::readInput(int pin) {

	if (!isExported[pin]) {
		exportGPIO(pin);
	}

	int fd = gpioToValueFD[pin];

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

	// lock for entire scope
	std::lock_guard<std::mutex> lock(gpioRPiMutex);

	// record callback by FD
	pinToCallback[pin] = callback;

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

	// record this pin as being interrupt-enabled
	interruptPins.insert(pin);
}

void GPIORPi::clearInterruptHandler(int pin) {

	// lock for entire scope
	std::lock_guard<std::mutex> lock(gpioRPiMutex);

	if (interruptPins.find(pin) == interruptPins.end()) {
		ostringstream buf;
		buf << "clearInterruptHandler() pin " << pin
				<< " is not interrupt-enabled";
		throw runtime_error(buf.str());
	}

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

	// remove the callback entry
	pinToCallback.erase(gpioToValueFD[pin]);

	// remove from interrupt pin list
	interruptPins.erase(pin);

}

/** @brief exports a GPIO via sysfs
 *
 * 	This should be called before dealing with any GPIO.
 *
 * @param pin
 */

void GPIORPi::exportGPIO(int pin) {

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
		fd = open(valueFilePath.str().c_str(), O_RDWR);
	} while (fd == -1 && tries-- > 0);
	if (fd == -1) {
		throw runtime_error("Gave up waiting for GPIO to be exported");
	}

	// open the gpio value file so we can use it later.
	gpioToValueFD.insert(pair<uint8_t, int>(pin, fd));

	// reverse map for select()
	valueFDtoGPIO.insert(pair<int, uint8_t>(fd, pin));

	isExported.set(pin);

}

void GPIORPi::unexportGPIO(int pin) {

	// assume the caller has locked the mutex, so we don't do it here.

	if (!isExported[pin]) {
		ostringstream buf;
		buf << "Can't unexport RPi GPIO pin " << pin << " as not exported";
		throw runtime_error(buf.str());
	}

	setDirection(pin, GPIO::DIR::IN);

	// close the corresponding gpioValueFile
	close(gpioToValueFD[pin]);
	valueFDtoGPIO.erase(gpioToValueFD[pin]);
	gpioToValueFD.erase(pin);

	// TODO: remove any interrupt handlers if there are

	ofstream f("/sys/class/gpio/unexport");
	if (!f.is_open()) {
		throw runtime_error("Failed to open /sys/class/gpio/unexport");
	}
	f << pin;
	f.close();

	isExported.reset(pin);

}

/** @brief loop waiting for interrupt to trigger */

void GPIORPi::waitForInterruptLoop() {

	cout << "IN THREAD!!!" << endl;

	while (threadstate == THREADSTATE::RUNNING) {

		// lock the mutex
		gpioRPiMutex.lock();

		// do we have anything to monitor?
		if (interruptPins.empty()) {
			gpioRPiMutex.unlock();
			sleep(1);
			cerr << "NOTHING TO MONITOR" << endl;
			continue;
		}

		// build the pollist
		int pollCount = 0;
		for (auto i = interruptPins.begin(); i != interruptPins.end(); i++) {

			pollList[pollCount].fd = gpioToValueFD[*i];
			pollList[pollCount].events = POLLPRI | POLLERR;
			pollList[pollCount].revents = 0;
			pollCount++;
		}

		gpioRPiMutex.unlock();

		int ret = poll(pollList, pollCount, 10);

		if (ret == -1) {
			ostringstream buf;
			buf << "in waitForInterruptLoop() select() returned " << ret << ": "
					<< strerror(errno);
			throw runtime_error(buf.str());
		}

		if (ret != 0) {

			// check results
			std::map<int, GPIO::STATE> triggeredPins;
			gpioRPiMutex.lock();
			pollCount = 0;
			for (auto i = interruptPins.begin(); i != interruptPins.end();
					i++) {

				if (pollList[pollCount].revents != 0) {
					int fd = gpioToValueFD[*i];

					lseek(fd, 0, SEEK_SET);
					char state;
					int rc = read(fd, &state, 1);
					if (rc != 1) {
						ostringstream buf;
						buf << "Failed to read from /sys/class/gpio/gpio"
								<< (*i) << "/value; errno=" << errno << " : "
								<< strerror(errno);
						throw runtime_error(buf.str());
					}

					// record it for later
					triggeredPins.insert(
							pair<int, GPIO::STATE>(*i,
									state == '1' ?
											GPIO::STATE::HI : GPIO::STATE::LO));
				}
				pollCount++;
			}

			gpioRPiMutex.unlock();

			// now that we've freed the mutex, call each of the triggered exceptions
			for (auto t = triggeredPins.begin(); t != triggeredPins.end();
					t++) {

				cerr << "TRIGGERED " << t->first << endl;
				pinToCallback[t->first](t->first, t->second);
			}
		}
	} // still running

	cout << "EXITING THREAD!!" << endl;
	threadstate = THREADSTATE::STOPPED;

}
