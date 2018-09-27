/** @brief PiExtPlane Main Program
 *
 *	sudo apt-get install libtinyxml2-4 libxplane-extplane-client0 libxplane-udp-client1
 *
 */

#include <string>
#include <stdexcept>
#include <sstream>
#include <sys/syslog.h>
#include <unistd.h>
#include <iostream>

#include "tinyxml2.h"
#include "XPlaneBeaconListener.h"

#include "../common/configFiles/HardwareXML.h"
#include "../common/configFiles/LayoutXML.h"
#include "../common/Hardware/GPIO.h"


#include "../common/Virtual/ExtPlaneClient.h"

using namespace std;

bool quitFlag = false;

void receiverBeaconCallback(XPlaneBeaconListener::XPlaneServer server,
		bool exists) {

	// only set this if we havent set it before
	if (! ExtPlaneClient::isStarted) {
		ExtPlaneClient::setHostname(server.host);
	}
	syslog (LOG_INFO, "Found XPlane beacon from %s", server.host.c_str());
}





int main(int argc, char * argv[]) {

	GPIO::init ();

	configFiles::HardwareXML::readHardwareXML();

	configFiles::LayoutXML::readLayoutXML();

	while (!quitFlag) {

		if (ExtPlaneClient::getHostname() == "auto") {

			// wait for beacon to set host

			syslog (LOG_INFO, "hostname is \"auto\", waiting for X-Plane beacon");
			XPlaneBeaconListener::getInstance()->registerNotificationCallback(
					std::bind(receiverBeaconCallback, std::placeholders::_1,
							std::placeholders::_2));
			while (ExtPlaneClient::getHostname() == "auto") {
				sleep (1);
			}
		}

		// start ExtPlane thread
		ExtPlaneClient::start();




		while (!quitFlag) {
			sleep (1);
		}

	}
}
