/*
 * Buzzer.cpp
 *
 *  Created on: Sep 8, 2018
 *      Author: shahada
 */

#include <unistd.h>
#include <ctype.h>
#include <iostream>

#include "Buzzer.h"

#include "GPIO.h"

using namespace std;

namespace Hardware {

int Buzzer::buzzerAddress = 0;
int Buzzer::buzzerPin = 25;

std::map<int, std::string> Buzzer::morseTable = {
		{'A', ".-"},
		{'B', "-..."},
		{'C', "-.-."},
		{'D', "-.."},
		{'E', "."},
		{'F', "..-."},
		{'G', "--."},
		{'H', "...."},
		{'I', ".."},
		{'J',".---"},
		{'K', "-.-"},
		{'L', ".-.."},
		{'M', "--"},
		{'N', "-."},
		{'O', "---"},
		{'P', ".--."},
		{'Q', "--.-"},
		{'R', ".-."},
		{'S', "..."},
		{'T', "-"},
		{'U', "..-"},
		{'V', "...-"},
		{'W', ".--"},
		{'X', "-..-"},
		{'Y', "-.--"},
		{'Z', "--.."},
		{'.', ".-.-.-"},
		{',', "--..--" }
};

void Buzzer::setBuzzerIO(int buzzerAddress, int buzzerPin) {
	Buzzer::buzzerAddress = buzzerAddress;
	Buzzer::buzzerPin = buzzerPin;

	GPIO::setDirection(buzzerAddress, buzzerPin, GPIODIR::OUT);
	GPIO::setPullUpDown(buzzerAddress, buzzerPin, GPIOPULL::UP);
}

void Buzzer::morseCode(std::string msg, int timeUnit) {

	cout << "morse [" << msg <<"]" << endl;

	for (auto ch = msg.begin(); ch != msg.end(); ch++) {

		if (*ch == ' ') {
			usleep (timeUnit * 1000 * 7);
		}

		else {

			auto code = morseTable.find(toupper(*ch, std::locale()));
			if (code != morseTable.end()) {

				for (auto c = code->second.begin(); c != code->second.end(); c++) {
					switch (*c) {
					case '.':
						GPIO::setOutput(buzzerAddress, buzzerPin, GPIOSTATE::HI);
						usleep(timeUnit * 1000);
						GPIO::setOutput(buzzerAddress, buzzerPin, GPIOSTATE::LO);
						break;
					case '-':
						GPIO::setOutput(buzzerAddress, buzzerPin, GPIOSTATE::HI);
						usleep(timeUnit * 1000 * 3);
						GPIO::setOutput(buzzerAddress, buzzerPin, GPIOSTATE::LO);
						break;
					}

					usleep(timeUnit * 3000);
				}


			}

		}

	}



}

} /* namespace Hardware */
