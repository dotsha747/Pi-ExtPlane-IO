/*
 * ExtPlane.cpp
 *
 *  Created on: Sep 8, 2018
 *      Author: shahada
 */

#include "../Hardware/Buzzer.h"

#include "ExtPlaneClient.h"

std::string ExtPlaneClient::hostname = "auto";
int ExtPlaneClient::port = 51000;
XPlaneExtPlaneClient::ExtPlaneClient * ExtPlaneClient::extPlaneClient = NULL;

void ExtPlaneClient::start() {

	extPlaneClient = new XPlaneExtPlaneClient::ExtPlaneClient(hostname, port,
			onConnect, onDisconnect, receiveData);

}

void ExtPlaneClient::onConnect() {

	Hardware::Buzzer::morseCode("X ");
	syslog(LOG_INFO, "Connected to ExtPlane at %s:%d", hostname.c_str(), port);
	extPlaneClient->sendLine("sub sim/aircraft/view/acf_descrip");

}

void ExtPlaneClient::onDisconnect() {

	Hardware::Buzzer::morseCode("Q ");
	syslog(LOG_INFO, "Disconnected from ExtPlane at %s:%d", hostname.c_str(),
			port);
}

void ExtPlaneClient::receiveData(std::string a, std::string b, std::string c) {

	syslog(LOG_INFO, "Received %s|%s|%s", a.c_str(), b.c_str(), c.c_str());
}

