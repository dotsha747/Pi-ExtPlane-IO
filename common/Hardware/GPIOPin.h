/*
 * GPIOLine.h
 *
 *  Created on: Aug 23, 2018
 *      Author: shahada
 */

#ifndef COMMON_HARDWARE_GPIOPIN_H_
#define COMMON_HARDWARE_GPIOPIN_H_

#include <string>

#include <sstream>
#include <stdexcept>

using namespace std;

namespace Hardware {


class GPIOPin {
	int address;
	int pin;
public:
	GPIOPin(std::string identifier) {
		// find the ":"
		size_t sep = identifier.find_first_of (':');
		if (sep == string::npos) {
			ostringstream buf;
			buf << "GPIO identifier \"" << identifier << "\" has no separator ':'";
			throw runtime_error (buf.str());
		}

		address = stoi (identifier.substr(0, sep), nullptr, 0);
		pin = stoi (identifier.substr(sep, string::npos), nullptr, 0);

	}
	GPIOPin(int address, int pin) {
		this->address = address;
		this->pin = pin;
	}
	virtual ~GPIOPin();
	int getAddress () {
		return address;
	}
	int getPin() {
		return pin;
	}
};

} /* namespace Hardware */

#endif /* COMMON_HARDWARE_GPIOPIN_H_ */
