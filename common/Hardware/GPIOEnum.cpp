#include <sstream>

#include "GPIOEnum.h"

std::ostream &operator<<(std::ostream &os, GPIODIR const &m) {

	switch (m) {
	case GPIODIR::IN:
		return os << "GPIODIR::IN";
	case GPIODIR::OUT:
		return os << "GPIODIR::OUT";
	default:
		return os << "GPIODIR::UNKNOWN";
	}
}

std::ostream &operator<<(std::ostream &os, GPIOSTATE const &m) {

	switch (m) {
	case GPIOSTATE::HI:
		return os << "GPIOSTATE::HI";
	case GPIOSTATE::LO:
		return os << "GPIOSTATE::LO";
	default:
		return os << "GPIOSTATE::UNKNOWN";
	}
}

std::ostream &operator<<(std::ostream &os, GPIOPULL const &m) {

	switch (m) {
	case GPIOPULL::DOWN:
		return os << "GPIOPULL::DOWN";
	case GPIOPULL::OFF:
		return os << "GPIOPULL::OFF";
	case GPIOPULL::UP:
		return os << "GPIOPULL::UP";
	default:
		return os << "GPIOPULL::UNKNOWN";
	}
}

std::ostream &operator<<(std::ostream &os, GPIOEDGE const &m) {

	switch (m) {
	case GPIOEDGE::BOTH:
		return os << "GPIOEDGE::BOTH";
	case GPIOEDGE::FALLING:
		return os << "GPIOEDGE::FALLING";
	case GPIOEDGE::NONE:
		return os << "GPIOEDGE::NONE";
	case GPIOEDGE::RISING:
		return os << "GPIOEDGE::RISING";
	default:
		return os << "GPIOEDGE::UNKNOWN";
	}
}

