//============================================================================
// Name        : TestServo.cpp
// Author      : Shahada Abubakar
// Version     :
// Copyright   : Copyright (c) 2017-2018, Shahada Abubakar
// Description : Tests Servo Outputs on the 737 I/O Controller board.
//
// Depends:
//============================================================================

#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <iomanip>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <math.h>

using namespace std;


// --------------------------------------------------------------------

int fd;
int freq;

// Registers/etc:
#define PCA9685_ADDRESS    0x40
#define MODE1              0x00
#define MODE2              0x01
#define SUBADR1            0x02
#define SUBADR2            0x03
#define SUBADR3            0x04
#define PRESCALE           0xFE
#define LED0_ON_L          0x06
#define LED0_ON_H          0x07
#define LED0_OFF_L         0x08
#define LED0_OFF_H         0x09
#define ALL_LED_ON_L       0xFA
#define ALL_LED_ON_H       0xFB
#define ALL_LED_OFF_L      0xFC
#define ALL_LED_OFF_H      0xFD

// Bits:
#define RESTART            0x80
#define SLEEP              0x10
#define ALLCALL            0x01
#define INVRT              0x10
#define OUTDRV             0x04

#define PCA9685_SWRST 0x06



void set_pwm_freq(unsigned int freqHz) {
	double prescaleval = 25000000.0; 	// 25mHZ
	prescaleval /= 4096.0;       		// 12-bit
	prescaleval /= freqHz;
	prescaleval -= 1.0;
	uint8_t prescale = floor(prescaleval + 0.5);
	uint8_t oldmode = wiringPiI2CReadReg8(fd, MODE1);
	uint8_t newmode = (oldmode & 0x7f) | 0x10;	// sleep
	wiringPiI2CWriteReg8 (fd, MODE1, newmode);				// go to sleep
	wiringPiI2CWriteReg8 (fd, PRESCALE, prescale);
	wiringPiI2CWriteReg8 (fd, MODE1, oldmode);
	usleep(5000);
	wiringPiI2CWriteReg8 (fd, MODE1, oldmode | 0x80);

	// update our copy
	freq = freqHz;
}


void set_pwm(unsigned int channel, unsigned int on,
		unsigned int off) {
	wiringPiI2CWriteReg8 (fd, LED0_ON_L + 4 * channel, on & 0xFF);
	wiringPiI2CWriteReg8 (fd, LED0_ON_H + 4 * channel, on >> 8);
	wiringPiI2CWriteReg8 (fd, LED0_OFF_L + 4 * channel, off & 0xFF);
	wiringPiI2CWriteReg8 (fd, LED0_OFF_H + 4 * channel, off >> 8);
}


void set_pwm_usecs(unsigned int channel, double pulseMicros) {

	/* https://learn.adafruit.com/adafruit-16-channel-servo-driver-with-raspberry-pi/library-reference
	 *
	 * If you need to calculate pulse-width in microseconds, you can do that by first
	 * figuring out how long each cycle is. That would be 1/freq where freq is the PWM
	 * frequency you set above. For 1000 Hz, that would be 1 millisecond. Then divide
	 * by 4096 to get the time per tick, eg 1 millisecond / 4096 = ~0.25 microseconds.
	 * If you want a pulse that is 10 microseconds long, divide the time by time-per-tick
	 * (10us / 0.25 us = 40) then turn on at tick 0 and turn off at tick 40.
	 *
	 */

	double cycleLengthMicros = 1000000/freq; // 244.140625
	double timePerTickMicros = cycleLengthMicros / 4096; // 0.059604645
	double onPulseLengthTicks = pulseMicros / timePerTickMicros * 1000; //

	//cout << "onPulseLengthTicks is " << fixed << onPulseLengthTicks << endl;

	set_pwm (channel, 0, ceil (onPulseLengthTicks));

}


// --------------------------------------------------------------------

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

	for (int i = 0; i < 16; i++) {
		set_pwm(i, 0, 0); // power off all channels
	}
	cout << endl << "Quit" << endl;

	exit(0);
}

int main(int argc, char * argv[]) {

	signal(SIGINT, intHandler);

	double min=1.000;
	double center=1.500;
	double max=2.000;
	double small = 0.001;
	double large = 0.010;
	bool safety = true;

	wiringPiSetup();
	fd = wiringPiI2CSetup (64);

	// init the PCA9685
	set_pwm_freq (60);
	wiringPiI2CWriteReg8 (fd, MODE2, OUTDRV);
	wiringPiI2CWriteReg8 (fd, MODE1, ALLCALL);
	usleep(5000);
	uint8_t mode1 = wiringPiI2CReadReg8(fd, MODE1);
	mode1 = mode1 & ~SLEEP;
	wiringPiI2CWriteReg8 (fd, MODE1, mode1);
	usleep(5000);

	cout << "TestServo Keys:" << endl << "\t.: Change Channel Up" << endl
			<< "\t,: Change Channel Down" << endl << "\t1: PWM on" << endl
			<< "\t0: PWM off" << endl << "\tj: decrease PWM time (h for larger decrease)" << endl
			<< "\tk: increase PWM time (l for larger increase)" << endl << "\tc: center (set time=1.500 m/s)" << endl
			<< "\ts: toggle Safety range on off" << endl
			<< endl << "\tq: quit" << endl << endl;

	setInputUnbuffered();
	unsigned char c;

	int nowCh = 0;
	bool nowState[16];
	double nowTime[16];

	for (int i = 0; i < 16; i++) {
		nowState[i] = false;
		nowTime[i] = center;
		set_pwm(i, 0, 4096); // power off
	}

	do {

		cout << "\r"
				<< "Safety:" << (safety ? "On " : "Off")
				<< " Ch:" << setw(2) << setfill('0') << nowCh
				<< " State:" << (nowState[nowCh] ? "On " : "Off")
				<< " Timing:"
				<< setfill(' ') << fixed << setprecision (3) << setw(5) << nowTime[nowCh] << " m/s ";

		c = getchar();

		switch (c) {
		case '0':
			nowState[nowCh] = false;
			break;
		case '1':
			nowState[nowCh] = true;
			break;
		case '.':
			if (nowCh < 15)
				nowCh++;
			break;
		case ',':
			if (nowCh > 0)
				nowCh--;
			break;
		case 'j':
			if (!safety || nowTime[nowCh] > min + small)
				nowTime[nowCh]-= small;
			break;
		case 'h':
			if (!safety || nowTime[nowCh] > min + large)
				nowTime[nowCh] -= large;
			break;
		case 'k':
			if (!safety || nowTime[nowCh] < max - small)
				nowTime[nowCh] += small;
			break;
		case 'l':
			if (!safety || nowTime[nowCh] < max - large)
				nowTime[nowCh] += large;
			break;
		case 'c':
			nowTime[nowCh] = 1.500;
			break;
		case 's':
			safety = !safety;
			break;
		}

		// update current channel
		if (nowState[nowCh]) {
			set_pwm_usecs(nowCh, nowTime[nowCh]);
		} else {
			set_pwm(nowCh, 0, 4096); // power off
		}

		//cout << c << endl;

	} while (c != 'q');

	restoreInputBuffering();

	cout << endl;

	return 0;

}
