#include <iostream>
#include <unistd.h>

#include "HT16K33.h"

using namespace std;



int main (int argc, char * argv[]) {

	HT16K33 * ht16k33 = new HT16K33 (); // default address and bus

	ht16k33->setLED (2);
	ht16k33->sendLED();

	sleep (5);


	delete ht16k33;
}
