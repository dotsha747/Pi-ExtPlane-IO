/** @brief PiExtPlane Main Program
 *
 */

#include <string>
#include <stdexcept>
#include <sstream>
#include <sys/syslog.h>
#include <unistd.h>

#include "tinyxml2.h"
#include "XPlaneBeaconListener.h"
#include "XPlaneExtPlaneClient/ExtPlaneClient.h"

using namespace std;

std::string extPlaneHost = "auto";
std::string nowExtPlaneHost;
int extPlanePort = 51000;

void readLayoutXML() {

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
				extPlaneHost = s;
			}

			s = elem->Attribute("port");
			if (s != NULL) {
				extPlanePort = atoi(s);
			}
		}

	}

	syslog(LOG_INFO, "Finished processing %s", path);

}

bool quitFlag = false;

void receiverBeaconCallback(XPlaneBeaconListener::XPlaneServer server,
		bool exists) {

	nowExtPlaneHost = server.host;
	syslog (LOG_INFO, "Found XPlane beacon from %s", server.host.c_str());
}



void onConnect() {

	syslog (LOG_INFO, "Connected to ExtPlane at %s:%d", nowExtPlaneHost.c_str(), extPlanePort);
}


void onDisconnect() {

	syslog (LOG_INFO, "Disconnected from ExtPlane at %s:%d", nowExtPlaneHost.c_str(), extPlanePort);
}


void receiveData (std::string a, std::string b, std::string c) {

}


int main(int argc, char * argv[]) {

	readLayoutXML();

	while (!quitFlag) {

		nowExtPlaneHost = extPlaneHost;
		if (extPlaneHost == "auto") {

			// wait for beacon to set host

			syslog (LOG_INFO, "hostname is \"auto\", waiting for X-Plane beacon");
			XPlaneBeaconListener::getInstance()->registerNotificationCallback(
					std::bind(receiverBeaconCallback, std::placeholders::_1,
							std::placeholders::_2));
			while (nowExtPlaneHost == "auto") {
				sleep (1);
			}
		}

		XPlaneExtPlaneClient::ExtPlaneClient * extPlaneClient = new XPlaneExtPlaneClient::ExtPlaneClient (
				nowExtPlaneHost,
				extPlanePort,
				onConnect,
				onDisconnect,
				receiveData
				);


		while (!quitFlag) {
			sleep (1);
		}

	}
}
