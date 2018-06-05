//============================================================================
// Name        : TestSevenSegment.cpp
// Author      : Shahada Abubakar
// Version     :
// Copyright   : Copyright (c) 2014, NEXTSense Sdn Bhd
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <wiringPi.h>
#include <mcp23017.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

using namespace std;

int datpin;
int clkpin;
int latpin;

void max7219Latch (int level) {
	cout << "LAT " << level << endl;
	digitalWrite (latpin, level);
}

void max7219Write16 (uint8_t address, uint8_t data) {

	uint16_t word = address << 8 | data;

	cout << "Writing " << hex << word << endl;

	for (int bit = 0; bit < 16; bit++) {
		cout << "CLK LOW" << endl;
		digitalWrite (clkpin, LOW);
		cout << bit << "DAT " << ((word & 0x8000) != 0) << endl;
		digitalWrite (datpin, (word & 0x8000) != 0);
		cout << "CLK HIGH" << endl;
		digitalWrite (clkpin, HIGH);

		word = word << 1;
	}

}

int main(int argc, char * argv[]) {

	wiringPiSetup();

	if (argc != 5) {
		cout << argc
				<< "TestShiftRegister <mcp23017dev> <datPin> <clkPin> <latPin> <cascade>"
				<< endl;
		exit(1);
	}

	int dev = (int) strtol(argv[1], NULL, 16);
	cout << "MCP23017 at I2C address 0x" << hex << dev << dec << endl;
	cout << "datpin " << atoi (argv[2]) << " clkpin " << atoi(argv[3]) << " latpin " << atoi (argv[4]) << endl;


	datpin = 100 + atoi(argv[2]);
	clkpin = 100 + atoi(argv[3]);
	latpin = 100 + atoi(argv[4]);


	// setup ports
	mcp23017Setup(100, dev);

	pullUpDnControl (datpin, PUD_OFF);
	digitalWrite (datpin, LOW);
	pinMode (datpin, OUTPUT);

	pullUpDnControl (clkpin, PUD_OFF);
	digitalWrite (clkpin, LOW);
	pinMode (clkpin, OUTPUT);

	pullUpDnControl (latpin, PUD_OFF);
	max7219Latch (HIGH);
	pinMode (latpin, OUTPUT);


	// normal operation
	max7219Latch (LOW);
	max7219Write16 (0x0c, 1);
	max7219Latch (HIGH);

	// code-B on all digits
	max7219Latch (LOW);
	max7219Write16 (0x09, 0xFF);
	max7219Latch (HIGH);

	// full intensity
	max7219Latch (LOW);
	max7219Write16 (0x0a, 0x0F);
	max7219Latch (HIGH);

	// scan limit all 8 digits
	max7219Latch (LOW);
	max7219Write16 (0x0b, 0x07);
	max7219Latch (HIGH);

	// display test mode
	max7219Latch (LOW);
	max7219Write16 (0x0f, 0x1);
	max7219Latch (HIGH);



	return 0;
}
