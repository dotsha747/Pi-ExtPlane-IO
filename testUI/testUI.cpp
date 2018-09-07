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

#include "../common/Hardware/GPIORPi.h"

using namespace std;

/* to test: LED2, button A, button B, buzzer.
 *
 *
 *
 */

GPIORPi * gpiorpi = NULL;

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
	gpiorpi->shutdown();

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

void buttonEvent(int pin, GPIOSTATE state) {

	if (pin == 23) {
		buttonAPressed = state == GPIOSTATE::LO;
	} else {
		buttonBPressed = state == GPIOSTATE::LO;
	}

	/*
	cout << "buttonEvent (" << pin << ", "
			<< (state == GPIOSTATE::HI ? "HI" : "LOW") << ")"
			<< (x == GPIOSTATE::HI ? "HI" : "LOW") << endl;
	*/

	drawScreen();
}

void morseCode(int pin, std::string msg, int timeUnit = 30) {

	for (auto c = msg.begin(); c != msg.end(); c++) {
		switch (*c) {
		case '.':
			gpiorpi->setOutput(25, GPIOSTATE::HI);
			usleep(timeUnit * 1000);
			gpiorpi->setOutput(25, GPIOSTATE::LO);
			break;
		case '-':
			gpiorpi->setOutput(25, GPIOSTATE::HI);
			usleep(timeUnit * 1000 * 3);
			gpiorpi->setOutput(25, GPIOSTATE::LO);
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

	gpiorpi = new GPIORPi ();
	// gpiorpi->setDebug (1);

	// use Broadcom BCM pin numbers

	// setup buttons
	gpiorpi->setDirection(23, GPIODIR::IN);
	gpiorpi->setPullUpDown(23, GPIOPULL::UP);
	gpiorpi->setInterruptHandler(23, GPIOEDGE::BOTH,
			std::bind(&buttonEvent, placeholders::_1, placeholders::_2));

	gpiorpi->setDirection(24, GPIODIR::IN);
	gpiorpi->setPullUpDown(24, GPIOPULL::UP);
	gpiorpi->setInterruptHandler(24, GPIOEDGE::BOTH,
			std::bind(&buttonEvent, placeholders::_1, placeholders::_2));

	// setup LED
	gpiorpi->setDirection(12, GPIODIR::OUT);
	gpiorpi->setOutput(12, GPIOSTATE::LO);

	// setup buzzer
	gpiorpi->setDirection(25, GPIODIR::OUT);
	gpiorpi->setOutput(25, GPIOSTATE::LO);

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
			gpiorpi->setOutput(12,
					led2On ? GPIOSTATE::HI : GPIOSTATE::LO);
			drawScreen();
			break;

		case 'q':
			intHandler(0);

		};

	};

}
