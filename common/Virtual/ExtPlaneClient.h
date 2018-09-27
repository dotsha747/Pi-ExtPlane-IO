/*
 * ExtPlane.h
 *
 *  Created on: Sep 8, 2018
 *      Author: shahada
 */


#ifndef PIEXTPLANEIO_EXTPLANECLIENT_H_
#define PIEXTPLANEIO_EXTPLANECLIENT_H_

#include <string>

#include "XPlaneExtPlaneClient/ExtPlaneClient.h"

class ExtPlaneClient {

protected:
	static std::string hostname;
	static int port;
	static XPlaneExtPlaneClient::ExtPlaneClient * extPlaneClient;


public:

	static void setHostname (std::string _hostname) {
		hostname = _hostname;
	}

	static std::string getHostname () {
		return hostname;
	}

	static void setPort (int _port) {
		port = _port;
	}

	static void start ();
	static bool isStarted () {
		return extPlaneClient != NULL;
	}

	static void onConnect ();
	static void onDisconnect ();
	static void receiveData (std::string a, std::string b, std::string c);

	static void stop ();
};

#endif /* PIEXTPLANEIO_EXTPLANECLIENT_H_ */
