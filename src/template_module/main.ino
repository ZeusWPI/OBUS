// (c) {YEAR}, {AUTHOR}
// See the LICENSE file for conditions for copying

#include <obus_module.h>

void setup() {
	Serial.begin(115200);

	// Choose one
	// Puzzle: a module that must be solved
	obus_module::setup(OBUS_TYPE_PUZZLE, /* Retrieve ID from MOANA */);
	// Needy: a module that periodically requires an action not to get strikes
	// obus_module::setup(OBUS_TYPE_NEEDY, /* Retrieve ID from MOANA */);
}

obus_can::message message;

void loop() {
	bool is_message_valid = obus_module::loop(&message);
}

void callback_game_start() {

}

void callback_game_stop() {

}
