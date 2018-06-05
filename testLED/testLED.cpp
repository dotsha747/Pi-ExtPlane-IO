#include <iostream>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringShift.h>
#include <mcp23017.h>
#include <string.h>
#include <cstdint>
#include <termios.h>
#include <signal.h>
#include <iomanip>

using namespace std;

int enaPin = 115;
int dataPin = 100;
int clockPinBase = 101;
int latchPinBase = 102;


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
	setInitState();

	cout << endl << "Quit" << endl;

	exit(0);
}


void setInitState () {

	for (int nowUnit = 0; nowUnit < 7 ; nowUnit++) {

		// switch off the latch, so changes as we shift bits in don't effect the LEDs.
		digitalWrite (latchPinBase+(nowUnit*2), LOW);

		// shift the 32-bit status out to the DM13A
		shiftOut (dataPin, clockPinBase+(nowUnit*2), MSBFIRST, 0);
		shiftOut (dataPin, clockPinBase+(nowUnit*2), MSBFIRST, 0);
		shiftOut (dataPin, clockPinBase+(nowUnit*2), MSBFIRST, 0);
		shiftOut (dataPin, clockPinBase+(nowUnit*2), MSBFIRST, 0);

		// switch on latch so LEDs follow what we've shifted in
		digitalWrite (latchPinBase+(nowUnit*2), HIGH);
	}

}


int main(int argc, char * argv[]) {

	signal(SIGINT, intHandler);
	setInputUnbuffered();

	wiringPiSetup();
	mcp23017Setup(100, 0x22); // LEDs are driven by MCP23017 0x22 (U20)

	// init the MCP to all outputs, and all off initially
	for (int i = 0; i< 16; i++) {
		digitalWrite (100+i, i != 15 ? LOW : HIGH);
		pinMode (100+i, OUTPUT);
	}

	setInitState();


	// we need to keep track of all the LED settings. There are 32 LEDs per control unit
	// so we use a uint32_t to hold them. And a max of 7 control units per MCP23017.
	uint32_t status[7] = {0, 0, 0, 0, 0, 0, 0};



	unsigned char c;

	int nowLED = 0;
	bool nowState[16];
	double nowTime[16];
	bool enabled = false;



	// set ENA
	digitalWrite (enaPin, enabled ? LOW : HIGH);

	do {

		int nowUnit = nowLED / 32;
		int nowBit = nowLED % 32;
		bool  nowState = (status [nowUnit] & (1 << nowBit)) != 0;

		cout << "\r" <<
				(enabled ? "ENABLED " : "DISABLED") <<
				" LED #" << setw(3) << setfill('0')<< nowLED <<
				" (Unit " << nowUnit << " bit " << setw(2) << setfill('0') << nowBit << ") " <<
				" Status: " << (nowState ? " ON" : "OFF") <<
				" ";

		c = getchar();

		switch (c) {
		case 'j':
			if (nowLED > 0)
				nowLED --;
			break;
		case 'k':
			if (nowLED < (7*32))
				nowLED++;
			break;
		case 'e':
			enabled = !enabled;
			digitalWrite (enaPin, enabled ? LOW: HIGH);
			break;
		case 's':

			uint32_t mask = 1 << nowBit;
			if (nowState) {
				// turn it off
				status [nowUnit] = status [nowUnit] & (~mask);
			} else {
				// turn it on
				status [nowUnit] |= mask;
			}

			// switch off the latch, so changes as we shift bits in don't effect the LEDs.
			digitalWrite (latchPinBase+(nowUnit*2), LOW);

			// shift the 32-bit status out to the DM13A
			shiftOut (dataPin, clockPinBase+(nowUnit*2), MSBFIRST, (uint8_t) ((status[nowUnit] >> 24) & 0xff));
			shiftOut (dataPin, clockPinBase+(nowUnit*2), MSBFIRST, (uint8_t) ((status[nowUnit] >> 16) & 0xff));
			shiftOut (dataPin, clockPinBase+(nowUnit*2), MSBFIRST, (uint8_t) ((status[nowUnit] >> 8) & 0xff));
			shiftOut (dataPin, clockPinBase+(nowUnit*2), MSBFIRST, (uint8_t) (status[nowUnit] & 0xff));


			// switch on latch so LEDs follow what we've shifted in
			digitalWrite (latchPinBase+(nowUnit*2), HIGH);
			break;

		}

		//cout << c << endl;

	} while (c != 'q');

	restoreInputBuffering();

	cout << endl;

	return 0;
}
