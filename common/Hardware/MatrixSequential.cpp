/*
 * MatrixSequential.cpp
 *
 *  Created on: Aug 23, 2018
 *      Author: shahada
 */

#include <thread>
#include <chrono>

#include "GPIO.h"
#include "MatrixSequential.h"

namespace Hardware {

MatrixSequential::MatrixSequential(int switchStart, int colInputAddr, int colStartPin,
		int colEndPin, int rowOutputAddr, int rowStartPin, int rowEndPin) : Matrix (switchStart) {

	this->colInputAddr = colInputAddr;
	this->colStartPin = colStartPin;
	this->colEndPin = colEndPin;
	this->rowOutputAddr = rowOutputAddr;
	this->rowStartPin = rowStartPin;
	this->rowEndPin = rowEndPin;

	this->rowSize = rowEndPin-rowStartPin+1;
	this->colSize = colEndPin-colStartPin+1;

	// init GPIO

	// set inputs
	for (int pin = colStartPin; pin <= colEndPin; pin++) {
		GPIO::setDirection(colInputAddr, pin, GPIODIR::IN);
		GPIO::setPullUpDown(colInputAddr, pin, GPIOPULL::UP);
	}

	// set ouputs
	for (int pin=rowStartPin; pin <= rowEndPin; pin++) {
		GPIO::setDirection(rowOutputAddr, pin, GPIODIR::OUT);
		GPIO::setOutput(rowOutputAddr, pin, GPIOSTATE::HI);
	}

	initPrevState (rowEndPin-rowStartPin, colEndPin-colStartPin);

	// launch scanning thread

	std::thread t (& MatrixSequential::scanLoop, this);
	t.detach();

}

MatrixSequential::~MatrixSequential() {

	if (threadState == THREADSTATE::RUNNING) {

		threadState = THREADSTATE::STOPPING;
		while (threadState != THREADSTATE::STOPPED) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}


	}
}


void MatrixSequential::scanLoop () {

	threadState = THREADSTATE::RUNNING;

	bool first = true;

	while (threadState == THREADSTATE::RUNNING) {

		// ROWS
		for (int output = rowStartPin; output <= rowEndPin; output++) {

			GPIO::setOutput(rowOutputAddr, output, GPIOSTATE::LO);

			// COLS

			// read a batch of pins in a go
			uint16_t cols = GPIO::readInputSequence (colInputAddr, colStartPin, colEndPin);

			for (int input = colStartPin; input <= colEndPin; input++) {

				// get the lowest bit in cols
				bool state = (cols & 0x01) == 1;

				// shift all the bits cols
				cols = cols >> 1;

				// check with previous
				if (state != prevState [input][output]) {

					if (! first) {
						// report it!
					}
					prevState[input][output] = state;
				}

			}

			GPIO::setOutput (rowOutputAddr, output, GPIOSTATE::HI);

		}

		first = false;

	}

	threadState = THREADSTATE::STOPPED;
}

} /* namespace Hardware */
