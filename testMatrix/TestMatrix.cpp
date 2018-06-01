//============================================================================
// Name        : TestMatrix.cpp
// Author      : Shahada Abubakar
// Version     :
// Copyright   : Copyright (c) 2017-2018, Shahada Abubakar
// Description : Tests Matrix input on the 737 I/O Controller board.
//
// testMatrix
//============================================================================

#include <iostream>
#include <iomanip>
#include <wiringPi.h>
#include <mcp23017.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>

using namespace std;

int main(int argc, char * argv[]) {


	if (argc != 2
			|| (
					(strcmp(argv[1], "readall") != 0)
					&&
					(strcmp(argv[1], "scan") != 0)
				)
		) {
		cerr << "Usage: testMatrix (readall)|(scan)" << endl;
		exit(1);
	}

	int readallFlag = strcmp (argv[1], "readall") == 0;

	int inputStart;
	int inputEnd;
	int outputStart;
	int outputEnd;

	wiringPiSetup();

	mcp23017Setup(100, 0x20); // outputs (U1)

	outputStart = 100;
	outputEnd = 115;

	mcp23017Setup(120, 0x21); // inputs (U2)

	inputStart = 120;
	inputEnd = 135;

	!readallFlag && cout << "Press Control-C to exit." << endl;

	// set inputs
	for (int pin = inputStart; pin <= inputEnd; pin++) {
		pinMode(pin, INPUT);
		pullUpDnControl(pin, PUD_UP);
	}

	// set outputs
	for (int pin = outputStart; pin <= outputEnd; pin++) {
		pinMode(pin, OUTPUT);
		digitalWrite(pin, HIGH);
	}

	int prevState[inputEnd + 1][outputEnd + 1];

	bool first = true;

	while (true) {
		for (int output = outputStart; output <= outputEnd; output++) {

			digitalWrite(output, LOW);

			!readallFlag && usleep(10000);

			readallFlag && cout << "Row " << setw(2) << (output-outputStart) << ": ";

			for (int input = inputStart; input <= inputEnd; input++) {

				int state = digitalRead(input);

				//cout << "row=" << output << "col=" << input << "="<< (state ? "1" : "0")<<endl;
				readallFlag && cout << (state ? "1" : "0") << " ";

				if (state != prevState[input][output]) {
					if (!first) {
						struct timeval tp;
						gettimeofday(&tp, NULL);
						char buf[80];
						strftime(buf, sizeof(buf), "%F %H:%M:%S.",
								localtime(&tp.tv_sec));

						// announce
						cout << buf << setfill('0') << setw(6) << tp.tv_usec
								<< " State Change [R" << setw(2) << (input - inputStart)
								<< "][C" << setw(2) << (output - outputStart) << "] "
								<< (state == HIGH ? "N/C" : "CON") << endl;
					}
					prevState[input][output] = state;
				}
			}

			readallFlag && cout << endl;

			digitalWrite(output, HIGH);

		}

		if (readallFlag) {
			cout << endl;
			break; // only do one scan for "readall"
		}

		first = false;

	}

}
