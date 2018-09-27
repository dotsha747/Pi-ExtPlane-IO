/*
 * Switches.h
 *
 *  Created on: Sep 2, 2018
 *      Author: shahada
 */

#ifndef COMMON_VIRTUAL_SWITCHES_H_
#define COMMON_VIRTUAL_SWITCHES_H_

#include <vector>
#include <functional>


/** @brief Virtual Switches.
 *
 * 	This class virtualizes switches implemented via IO lines and
 * 	also those implemented via a Matrix.
 *
 * 	Switches are identified by a switch number. Hardware classes
 * 	are expected to call RegisterSwitch to register each switch.
 *
 *	Switches have two states, they can be connected (CN) or
 *	Not Connected (NC).
 *
 *	The hardware classes will call notifySwitchState () whenever a
 *	switch state changes.
 *
 *	We can attach callbacks onto a switch using registerTrigger().
 *	They will be called whenever notifySwitchState is called.
 *
 *	we cache the state of all switches. On init, the hardware
 *	classes are expected set the initial state by calling
 *	notifySwitchState.
 *
 *	triggerAllCallbacks () is useful to referesh the sim with
 *	the current state of all switches. I added this so that we
 *	can refresh the switches on aircraft change.
 *
 */

namespace Virtual {

enum class SwitchState { NC , CN };

class Switches {
private:

	static int maxCount;
	static std::vector<SwitchState> currentState;



public:

	static void init();
	static void shutdown();

	static void registerSwitch (int switchNum);
	static void registerSwitchCallback (int switchNum, SwitchState state, std::function<void(int, SwitchState)> callback);
	static void notifySwitchState ();




};

} /* namespace Hardware */

#endif /* COMMON_VIRTUAL_SWITCHES_H_ */
