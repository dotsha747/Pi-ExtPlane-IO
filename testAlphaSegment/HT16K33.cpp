/*
 * ht16k33.cpp
 *
 *  Created on: Jul 1, 2018
 *      Author: shahada
 */

#include <string.h>
#include <stdexcept>

#include "HT16K33.h"

using namespace std;

HT16K33::HT16K33(int i2caddr, int bus) {

	i2c = new I2C(i2caddr, bus);

	i2c->writeRaw(HT16K33_SS | HT16K33_SS_NORMAL); // Wakeup
	i2c->writeRaw(HT16K33_DSP | HT16K33_DSP_ON | HT16K33_DSP_NOBLINK);
	i2c->writeRaw(HT16K33_RIS | HT16K33_RIS_OUT);
	i2c->writeRaw(HT16K33_DIM | HT16K33_DIM_16);

	clearAll();
}

HT16K33::~HT16K33() {

	delete i2c;
}

void HT16K33::bitSet(uint8_t & var, int bit) {

	uint8_t mask = 1 << bit;
	var = var | mask;
}

void HT16K33::bitClear(uint8_t & var, int bit) {

	uint8_t mask = 1 << bit;
	var = var & (~mask);
}

uint8_t HT16K33::bitRead(uint8_t & var, int bit) {

	uint8_t mask = 1 << bit;
	return (var & mask) != 0;

}

uint8_t HT16K33::lowByte(uint16_t word) {

	return word & 0xFF;
}

uint8_t HT16K33::highByte(uint16_t word) {

	return word >> 8;
}

void HT16K33::clearAll() {

	memset(displayRam, 0, sizeof(displayRam));
	i2c->writeRegBlock(HT16K33_DDAP, displayRam, sizeof(displayRam));
}

void HT16K33::sleep() {

	i2c->writeRaw(HT16K33_SS | HT16K33_SS_STANDBY); // Stop oscillator
}

void HT16K33::normal() {

	i2c->writeRaw(HT16K33_SS | HT16K33_SS_NORMAL); // Start oscillator
}

uint8_t HT16K33::clearLED(uint8_t ledno) { // clear an individual segment (16x8=128) in memory

	if (ledno >= 0 && ledno < 128) {
		bitClear(displayRam[int(ledno / 8)], (ledno % 8));
		return 0;
	} else {
		return 1;
	}
}

uint8_t HT16K33::setLED(uint8_t ledno) { // set an individual segment (16x8=128) in memory
	if (ledno >= 0 && ledno < 128) {
		bitSet(displayRam[int(ledno / 8)], (ledno % 8));
		return 0;
	} else {
		return 1;
	}
}
uint8_t HT16K33::getLED(uint8_t ledno) { // get status of an individual segment (16x8=128)

	// get the current state from chip
	i2c->readRegBlock(HT16K33_DDAP, displayRam, sizeof(displayRam));

	if (ledno >= 0 && ledno < 128) {
		return bitRead(displayRam[int(ledno / 8)], ledno % 8) != 0;
	} else {
		throw runtime_error ("ledno out of range");
	}
}

uint8_t HT16K33::setDisplayRaw(uint8_t pos, uint8_t val) {

	if (pos < sizeof(displayRam)) {
		displayRam[pos] = val;
		return 0;
	} else {
		return 1;
	}

}

uint8_t HT16K33::sendLED() {

	return i2c->writeRegBlock(HT16K33_DDAP, displayRam, sizeof(displayRam));

}

void HT16K33::define16SegFont(uint16_t *ptr) { // pass a pointer to a font table for 16seg

	seg16Font = ptr;
}

uint8_t HT16K33::set16Seg(uint8_t dig, uint8_t cha) { // display character at particular digit using defined font

	if (cha >= 0 && cha < 128 && dig >= 0 && dig < 8) {
		dig = dig * 2;
		displayRam[dig] = lowByte(seg16Font[cha]);
		displayRam[dig + 1] = highByte(seg16Font[cha]);
		return 0;
	} else {
		return 1;
	}

}

void HT16K33::setBrightness(uint8_t level) { 		// set brightness

	if (HT16K33_DIM_1 >= level && level < HT16K33_DIM_16) {
		i2c->writeRaw(HT16K33_DIM | level);
	} else {
		throw runtime_error ("level out of range");
	}
}

uint8_t HT16K33::setBlinkRate(uint8_t rate) {
	switch (rate) {
	case HT16K33_DSP_NOBLINK:
	case HT16K33_DSP_BLINK2HZ:
	case HT16K33_DSP_BLINK1HZ:
	case HT16K33_DSP_BLINK05HZ:
		i2c->writeRaw(HT16K33_DSP | rate);
		return 0;
	default:
		return 1;
	}
}

void HT16K33::displayOn() {

	i2c->writeRaw(HT16K33_DSP |HT16K33_DSP_ON);
}

void HT16K33::displayOff() {

	i2c->writeRaw (HT16K33_DSP |HT16K33_DSP_OFF);
}
