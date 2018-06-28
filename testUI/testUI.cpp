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


#include <wiringPi.h>

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

	digitalWrite(12, LOW);
	pinMode(12, INPUT);

	digitalWrite(25, LOW);
	pinMode(25, INPUT);

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


int main(int argc, char * argv[]) {

	signal(SIGINT, intHandler);
	setInputUnbuffered();

	// use Broadcom BCM pin numbers
	wiringPiSetupGpio();

	// setup buttons
	pinMode(23, INPUT);
	pullUpDnControl(23, PUD_UP);
	wiringPiISR (23, INT_EDGE_BOTH, buttonAEvent);

	pinMode(24, INPUT);
	pullUpDnControl(24, PUD_UP);
	wiringPiISR (24, INT_EDGE_BOTH, buttonBEvent);

	// setup LED
	digitalWrite(12, LOW);
	pinMode(12, OUTPUT);

	// setup buzzer
	digitalWrite(25, LOW);
	pinMode(25, OUTPUT);

	char c = 0;

	while (1) {

		drawScreen();

		char c = getchar();

		switch (c) {

		case 'b':
			buzzerOn = ! buzzerOn;
			digitalWrite (25, buzzerOn ? HIGH: LOW);
			drawScreen ();
			break;

		case 'l':
			led2On = ! led2On;
			digitalWrite (12, led2On ? HIGH : LOW);
			drawScreen ();
			break;

		case 'q':
			intHandler (0);
			exit (1);

		};



	};

}
