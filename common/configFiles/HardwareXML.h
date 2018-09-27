/*
 * HardwareXML.h
 *
 *  Created on: Aug 23, 2018
 *      Author: shahada
 */

#ifndef COMMON_CONFIGFILES_HARDWAREXML_H_
#define COMMON_CONFIGFILES_HARDWAREXML_H_

/** @brief read in the hardwareXML file, and set up internal data structures.
 *
 * 	This should only be called once at system startup, since the hardware
 * 	never changes.
 *
 */

namespace configFiles {

class HardwareXML {
public:
	static void readHardwareXML();
	static void parseIO(const char * s, int & address, int & pin);
};

}

#endif /* COMMON_CONFIGFILES_HARDWAREXML_H_ */
