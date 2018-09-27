/*
 * HardwareXML.cpp
 *
 *  Created on: Aug 23, 2018
 *      Author: shahada
 */

#include <tinyxml2.h>
#include <unistd.h>
#include <sys/syslog.h>
#include <stdexcept>
#include <sstream>
#include <string.h>
#include <cstdlib>
#include <iostream>

#include "../Hardware/GPIO.h"
#include "../Hardware/MCP23017.h"
#include "../Hardware/Buzzer.h"

#include "HardwareXML.h"

using namespace std;

namespace configFiles {

void HardwareXML::readHardwareXML() {

	// look for the hardware.xml either in the local folder,
	// or in the /etc/piIO folder.

	char const * path = "./hardware.xml";
	FILE * file = fopen(path, "r");
	if (file == NULL) {
		path = "/etc/piIO/hardware.xml";
		file = fopen(path, "r");
		if (file == NULL) {
			throw runtime_error(
					"Unable to find hardware.xml in current direction or /etc/piIO");
		}
	}
	syslog(LOG_INFO, "readHardwareXML: using %s", path);

	// read in the XML file

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError status = doc.LoadFile(file);
	if (status != tinyxml2::XML_SUCCESS) {
		ostringstream buf;
		buf << "Unable to load " << path << " as document";
		throw runtime_error(buf.str().c_str());
	}

	// get root
	tinyxml2::XMLElement * root = doc.RootElement();
	if (root == nullptr) {
		ostringstream buf;
		buf << "Unable to find root element in " << path;
		throw runtime_error(buf.str().c_str());
	}

	// iterate through child nodes
	tinyxml2::XMLElement * elem = root->FirstChildElement();
	while (elem != nullptr) {

		if (strcmp(elem->Name(), "buzzer") == 0) {

			const char * s = elem->Attribute("io");
			if (s != NULL) {

				int addr;
				int pin;
				parseIO(s, addr, pin);
				Hardware::Buzzer::setBuzzerIO(addr, pin);
				syslog (LOG_INFO, "Found Buzzer at 0x%x:0x%x", addr, pin);
			}

		} else if (strcmp(elem->Name(), "mcp23017") == 0) {

			const char * s = elem->Attribute("addr");
			if (s == NULL) {
				throw runtime_error(
						"In %s, <mcp23017 ...> has no attribute \"addr\"");
			}
			int i2caddr = strtol(s, NULL, 0);

			int interruptPin;
			s = elem->Attribute("intr");
			if (s == NULL) {
				interruptPin = -1;
			} else {
				interruptPin = strtol(s, NULL, 0);
			}

			int i2cbus;
			s = elem->Attribute("i2cbus");
			if (s == NULL) {
				i2cbus = -1;
			} else {
				i2cbus = strtol(s, NULL, 0);
			}

			// create mcp23017
			MCP23017 * mcp23017 = new MCP23017(i2caddr, interruptPin, i2cbus);

			// register into GPIO
			GPIO::attachGPIOProvider(mcp23017);

		}

		elem = elem->NextSiblingElement();

	}

	syslog(LOG_INFO, "readHardwareXML: Finished processing %s", path);

}


/** @brief parses an GPIO IO address:pin pair
 *
 *	- separated by ":"
 *	- either value may be base-16 with prefix of "0x"
 *
 * @param s
 * @param address
 * @param pin
 */

void HardwareXML::parseIO(const char * s, int & address, int & pin) {

	std::string d = s;

	cout << "GOT [" << d << "]" << endl;

	size_t sep = d.find_first_of (':');
	if (sep == string::npos) {
		ostringstream buf;
		buf << "Unable to locate ':' while parsing GPIO IO \"" << s << "\"";
		throw runtime_error (buf.str().c_str());
	}

	address = strtol (d.substr(0, sep).c_str(), NULL, 0);
	pin = strtol (d.substr(sep+1).c_str(), NULL, 0);

	cout << "addr [" << d.substr(0, sep) << "]" << endl;
	cout << "pin  [" << d.substr(sep+1) << "]" << endl;

}


}


/* namespace configFiles */
