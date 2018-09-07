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

void readHardwareXML ();


#endif /* COMMON_CONFIGFILES_HARDWAREXML_H_ */
