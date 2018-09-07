#include <thread>
#include <iostream>
#include <unistd.h>
#include <functional>
#include <bitset>

#include "../common/Hardware/GPIO.h"
#include "../common/Hardware/MCP23017.h"

using namespace std;


#define MCP23x17_GPINTENA       0x04
#define MCP23x17_DEFVALA        0x06
#define MCP23x17_INTCONA        0x08
#define MCP23x17_GPINTENB       0x05
#define MCP23x17_DEFVALB        0x07


void intHandler (int pin, GPIOSTATE state) {

	cout << "Pin " << pin << " " << state << endl;

}


int main (int argc, char * argv[]) {


	int addr = 0x24;

	GPIO::attachGPIOProvider (new MCP23017 (addr, 20, 1));
	GPIO::setDebug(1);

	GPIO::setPullUpDown(addr, 0, GPIOPULL::UP);
	GPIO::setDirection(addr, 0, GPIODIR::IN);


	GPIO::setInterruptHandler(
			addr,
			0,
			GPIOEDGE::BOTH,
			std::bind(&intHandler, std::placeholders::_1, std::placeholders::_2)
	);



	while (1) {
		sleep (1);

		cout << "Tick ..." << endl;
	}

	exit (0);
}
