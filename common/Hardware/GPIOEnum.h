/*
 * GPIOEnum.h
 *
 *  Created on: Jul 25, 2018
 *      Author: shahada
 */

#ifndef COMMON_GPIOENUM_H_
#define COMMON_GPIOENUM_H_


enum class GPIODIR {
	IN, OUT
};
enum class GPIOSTATE {
	LO, HI
};
enum class GPIOPULL {
	OFF, UP, DOWN
};
enum class GPIOEDGE {
	NONE, RISING, FALLING, BOTH
};


std::ostream &operator<<(std::ostream &os, GPIODIR const &m);
std::ostream &operator<<(std::ostream &os, GPIOSTATE const &m);
std::ostream &operator<<(std::ostream &os, GPIOPULL const &m);
std::ostream &operator<<(std::ostream &os, GPIOEDGE const &m);

#endif /* COMMON_GPIOENUM_H_ */
