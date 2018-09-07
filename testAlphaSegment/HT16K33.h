/*
 * ht16k33.h
 *
 *  Created on: Jul 1, 2018
 *      Author: shahada
 */


// Ported from Arduino Library https://github.com/lpaseen/ht16k33
#ifndef TESTALPHASEGMENT_HT16K33_H_
#define TESTALPHASEGMENT_HT16K33_H_

#include <cstddef>
#include <cstdint>

#include "I2C.h"

//Commands
#define HT16K33_DDAP          0b00000000 // Display data address pointer: 0000xxxx
#define HT16K33_SS            0b00100000 // System setup register
#define HT16K33_SS_STANDBY    0b00000000 // System setup - oscillator in standby mode
#define HT16K33_SS_NORMAL     0b00000001 // System setup - oscillator in normal mode
#define HT16K33_KDAP          0b01000000 // Key Address Data Pointer
#define HT16K33_IFAP          0b01100000 // Read status of INT flag
#define HT16K33_DSP           0b10000000 // Display setup
#define HT16K33_DSP_OFF       0b00000000 // Display setup - display off
#define HT16K33_DSP_ON        0b00000001 // Display setup - display on
#define HT16K33_DSP_NOBLINK   0b00000000 // Display setup - no blink
#define HT16K33_DSP_BLINK2HZ  0b00000010 // Display setup - 2hz blink
#define HT16K33_DSP_BLINK1HZ  0b00000100 // Display setup - 1hz blink
#define HT16K33_DSP_BLINK05HZ 0b00000110 // Display setup - 0.5hz blink
#define HT16K33_RIS           0b10100000 // ROW/INT Set
#define HT16K33_RIS_OUT       0b00000000 // Set INT as row driver output
#define HT16K33_RIS_INTL      0b00000001 // Set INT as int active low
#define HT16K33_RIS_INTH      0b00000011 // Set INT as int active high
#define HT16K33_DIM           0b11100000 // Dimming set
#define HT16K33_DIM_1         0b00000000 // Dimming set - 1/16
#define HT16K33_DIM_2         0b00000001 // Dimming set - 2/16
#define HT16K33_DIM_3         0b00000010 // Dimming set - 3/16
#define HT16K33_DIM_4         0b00000011 // Dimming set - 4/16
#define HT16K33_DIM_5         0b00000100 // Dimming set - 5/16
#define HT16K33_DIM_6         0b00000101 // Dimming set - 6/16
#define HT16K33_DIM_7         0b00000110 // Dimming set - 7/16
#define HT16K33_DIM_8         0b00000111 // Dimming set - 8/16
#define HT16K33_DIM_9         0b00001000 // Dimming set - 9/16
#define HT16K33_DIM_10        0b00001001 // Dimming set - 10/16
#define HT16K33_DIM_11        0b00001010 // Dimming set - 11/16
#define HT16K33_DIM_12        0b00001011 // Dimming set - 12/16
#define HT16K33_DIM_13        0b00001100 // Dimming set - 13/16
#define HT16K33_DIM_14        0b00001101 // Dimming set - 14/16
#define HT16K33_DIM_15        0b00001110 // Dimming set - 15/16
#define HT16K33_DIM_16        0b00001111 // Dimming set - 16/16


class HT16K33 {
private:
	I2C * i2c;
	std::uint8_t displayRam [16];
	std::uint16_t * seg16Font = NULL;

	void bitSet (std::uint8_t & var, int bit);
	void bitClear (std::uint8_t & var, int bit);
	std::uint8_t bitRead (std::uint8_t & var, int bit);
	std::uint8_t lowByte (std::uint16_t word);
	std::uint8_t highByte (std::uint16_t word);

public:
	HT16K33(int i2caddr = 0x70, int bus = -1);
	virtual ~HT16K33();

	void clearAll ();
	void sleep ();
	void normal();
	std::uint8_t clearLED (std::uint8_t ledno); 	// clear an individual segment (16x8=128) in display RAM
	std::uint8_t setLED (std::uint8_t ledno);		// set an individual segment (16x8=128) in display RAM
	std::uint8_t getLED (std::uint8_t ledno);		// get status of an individual segment (16x8=128) in display RAM
	std::uint8_t setDisplayRaw (std::uint8_t pos, std::uint8_t val); // set raw value in display RAM
	std::uint8_t sendLED (); // write display RAM to device
	void define16SegFont (std::uint16_t *ptr); 	// pass a pointer to a font table for 16seg
	std::uint8_t set16Seg (std::uint8_t dig, uint8_t cha); 	// display character at particular digit using defined font
	void setBrightness (std::uint8_t level); 		// set brightness
	std::uint8_t setBlinkRate (std::uint8_t rate);
	void displayOn();
	void displayOff();

};

#endif /* TESTALPHASEGMENT_HT16K33_H_ */
