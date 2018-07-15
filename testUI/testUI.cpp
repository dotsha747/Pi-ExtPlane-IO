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

// global!
GPIORPi gpiorpi;



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

	gpiorpi.cleanup();

	cout << endl << "Quit" << endl;

	exit(0);
}


bool buzzerOn = false;
bool led2On = false;
bool buttonAPressed = false;
bool buttonBPressed = false;


std::mutex drawScreenMutex;

void drawScreen() {

	//std::lock_guard<std::mutex> lock (drawScreenMutex);

	cout << "\r" <<
			" ButtonA:" << (buttonAPressed ? "ON " : "OFF") <<
			" ButtonB:" << (buttonBPressed ? "ON " : "OFF") <<
			" [b] Buzzer:" << (buzzerOn ? "ON " : "OFF") <<
			" [l] LED2:" << (led2On ? "ON " : "OFF") <<

			" [q] Quit ";
	cout.flush();

}

/*
void buttonAEvent () {

	buttonAPressed = digitalRead (23) == 0;
	delay (10);
	drawScreen();

}

void buttonBEvent () {

	buttonBPressed = digitalRead (24) == 0;
	delay (10);
	drawScreen();
}
*/


void buttonEvent (int pin, GPIO::STATE state) {

	auto x = gpiorpi.readInput (pin);
	cout << "buttonEvent (" << pin << ", " << (state == GPIO::STATE::HI ? "HI" : "LOW") << ")" << (x == GPIO::STATE::HI ? "HI" : "LOW") << endl;

}

int main(int argc, char * argv[]) {

	signal(SIGINT, intHandler);
	setInputUnbuffered();

	// use Broadcom BCM pin numbers


	// setup buttons
	gpiorpi.setDirection (23, GPIO::DIR::IN);
	gpiorpi.setPullUpDown (23, GPIO::PULL::UP);
	gpiorpi.setInterruptHandler (23, GPIO::EDGE::BOTH, std::bind (&buttonEvent, placeholders::_1, placeholders::_2));


	gpiorpi.setDirection (24, GPIO::DIR::IN);
	gpiorpi.setPullUpDown (24, GPIO::PULL::UP);
	gpiorpi.setInterruptHandler (24, GPIO::EDGE::BOTH, std::bind (&buttonEvent, placeholders::_1, placeholders::_2));


	// setup LED
	gpiorpi.setDirection (12, GPIO::DIR::OUT);
	gpiorpi.setOutput (12, GPIO::STATE::LO);

	// setup buzzer
	gpiorpi.setDirection (25, GPIO::DIR::OUT);
	gpiorpi.setOutput (25, GPIO::STATE::LO);


	while (1) {

		drawScreen();

		char c = getchar();

		switch (c) {

		case 'b':
			buzzerOn = ! buzzerOn;
			gpiorpi.setOutput (25, buzzerOn ? GPIO::STATE::HI : GPIO::STATE::LO);

			drawScreen ();
			break;

		case 'l':
			led2On = ! led2On;
			gpiorpi.setOutput (12, led2On ? GPIO::STATE::HI : GPIO::STATE::LO);
			drawScreen ();
			break;

		case 'q':
			intHandler (0);

		};



	};

}
