//============================================================================
// Name        : TestUI.cpp
// Author      : Shahada Abubakar
// Version     :
// Copyright   : Copyright (c) 2018 Shahada Abubakar
// Description : Test 737 I/O Controller UI
//============================================================================

#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <iostream>
#include <mutex>

#include "../common/GPIORPi.h"

using namespace std;

/* to test: LED2, button A, button B, buzzer.
 *
 *
 *
 */

struct termios old_tio;

// http://shtrom.ssji.net/skb/getc.html

void setInputUnbuffered() {

	struct termios new_tio;

	/* get the terminal settings for stdin */
	tcgetattr(STDIN_FILENO, &old_tio);

	/* we want to keep the old setting to restore them a the end */
	new_tio = old_tio;

	/* disable canonical mode (buffered i/o) and local echo */
	new_tio.c_lflag &= (~ICANON & ~ECHO);

	/* set the new settings immediately */
	tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

}

void restoreInputBuffering() {

	/* restore the former settings */
	tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

}

void intHandler(int dummy) {

	restoreInputBuffering();
	cout << endl << "Quit" << endl;
	GPIORPi::getInstance().shutdown();

	exit(0);
}

bool led2On = false;
bool buttonAPressed = false;
bool buttonBPressed = false;

std::mutex drawScreenMutex;

void drawScreen() {

	//std::lock_guard<std::mutex> lock (drawScreenMutex);

	cout << "\r" << " ButtonA:" << (buttonAPressed ? "ON " : "OFF")
			<< " ButtonB:" << (buttonBPressed ? "ON " : "OFF") << " [b] Buzzer"
			<<  " [l] LED2:"
			<< (led2On ? "ON " : "OFF") <<

			" [q] Quit ";
	cout.flush();

}

void buttonEvent(int pin, GPIO::STATE state) {

	auto x = GPIORPi::getInstance().readInput(pin);

	if (pin == 23) {
		buttonAPressed = state == GPIO::STATE::LO;
	} else {
		buttonBPressed = state == GPIO::STATE::LO;
	}

	/*
	cout << "buttonEvent (" << pin << ", "
			<< (state == GPIO::STATE::HI ? "HI" : "LOW") << ")"
			<< (x == GPIO::STATE::HI ? "HI" : "LOW") << endl;
	*/

	drawScreen();
}

void morseCode(int pin, std::string msg, int timeUnit = 30) {

	for (auto c = msg.begin(); c != msg.end(); c++) {
		switch (*c) {
		case '.':
			GPIORPi::getInstance().setOutput(25, GPIO::STATE::HI);
			usleep(timeUnit * 1000);
			GPIORPi::getInstance().setOutput(25, GPIO::STATE::LO);
			break;
		case '-':
			GPIORPi::getInstance().setOutput(25, GPIO::STATE::HI);
			usleep(timeUnit * 1000 * 3);
			GPIORPi::getInstance().setOutput(25, GPIO::STATE::LO);
			break;
		case ' ':
			usleep(timeUnit * 1000 * 4);
			break;
		}

		usleep(timeUnit * 1000 * 3);
	}

}

int main(int argc, char * argv[]) {

	signal(SIGINT, intHandler);
	setInputUnbuffered();

	//GPIORPi::getInstance().setDebug (1);

	// use Broadcom BCM pin numbers

	// setup buttons
	GPIORPi::getInstance().setDirection(23, GPIO::DIR::IN);
	GPIORPi::getInstance().setPullUpDown(23, GPIO::PULL::UP);
	GPIORPi::getInstance().setInterruptHandler(23, GPIO::EDGE::BOTH,
			std::bind(&buttonEvent, placeholders::_1, placeholders::_2));

	GPIORPi::getInstance().setDirection(24, GPIO::DIR::IN);
	GPIORPi::getInstance().setPullUpDown(24, GPIO::PULL::UP);
	GPIORPi::getInstance().setInterruptHandler(24, GPIO::EDGE::BOTH,
			std::bind(&buttonEvent, placeholders::_1, placeholders::_2));

	// setup LED
	GPIORPi::getInstance().setDirection(12, GPIO::DIR::OUT);
	GPIORPi::getInstance().setOutput(12, GPIO::STATE::LO);

	// setup buzzer
	GPIORPi::getInstance().setDirection(25, GPIO::DIR::OUT);
	GPIORPi::getInstance().setOutput(25, GPIO::STATE::LO);

	while (1) {

		drawScreen();

		char c = getchar();

		switch (c) {

		case 'b':
			morseCode (25, ".... . .-.. .-.. ---", 30);
			drawScreen();
			break;

		case 'l':
			led2On = !led2On;
			GPIORPi::getInstance().setOutput(12,
					led2On ? GPIO::STATE::HI : GPIO::STATE::LO);
			drawScreen();
			break;

		case 'q':
			intHandler(0);

		};

	};

}
