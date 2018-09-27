/*
 * Buzzer.h
 *
 *  Created on: Sep 8, 2018
 *      Author: shahada
 */

#ifndef COMMON_HARDWARE_BUZZER_H_
#define COMMON_HARDWARE_BUZZER_H_

#include <string>
#include <map>

namespace Hardware {

class Buzzer {
protected:
	static int buzzerAddress;
	static int buzzerPin;

	static std::map<int, std::string> morseTable;


public:
	static void setBuzzerIO (int buzzerAddress, int buzzerPin);
	static void morseCode(std::string msg, int timeUnit = 50);
};

} /* namespace Hardware */

#endif /* COMMON_HARDWARE_BUZZER_H_ */
