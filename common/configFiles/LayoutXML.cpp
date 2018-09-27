/*
 * LayoutXML.cpp
 *
 *  Created on: Sep 8, 2018
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

#include "LayoutXML.h"

#include "../Virtual/ExtPlaneClient.h"

using namespace std;

namespace configFiles {

void LayoutXML::readLayoutXML() {

	char const * path = "./layout.xml";
	FILE * file = fopen(path, "r");
	if (file == NULL) {
		path = "/etc/piIO/layout.xml";
		file = fopen(path, "r");
		if (file == NULL) {
			throw runtime_error(
					"Unable to find layout.xml in current direction or /etc/piIO");
		}
	}
	syslog(LOG_INFO, "readLayoutXML: using %s", path);

	// read in the XML file

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError status = doc.LoadFile(file);
	if (status != tinyxml2::XML_SUCCESS) {
		ostringstream buf;
		buf << "Unable to load " << path << " as document";
		throw runtime_error(buf.str().c_str());
	}

	// get root
	tinyxml2::XMLNode* root = doc.FirstChild();
	if (root == nullptr) {
		ostringstream buf;
		buf << "Unable to find root eleent in " << path;
		throw runtime_error(buf.str().c_str());
	}

	// iterate through child nodes
	tinyxml2::XMLElement * elem = root->FirstChildElement();
	while (elem != nullptr) {

		if (strcmp(elem->Name(), "extPlane") == 0) {

			const char * s = elem->Attribute("host");
			if (s != NULL) {
				ExtPlaneClient::setHostname(s);
			}

			s = elem->Attribute("port");
			if (s != NULL) {
				ExtPlaneClient::setPort(atoi(s));
			}
		}

	}

	syslog(LOG_INFO, "Finished processing %s", path);

}


} /* namespace configFiles */
