// (c) {YEAR}, {AUTHOR}
// See the LICENSE file for conditions for copying

#include <obus_module.h>

void setup() {
	Serial.begin(115200);

	// Choose one
	// Puzzle: a module that must be solved
	obus_module::setup(OBUS_TYPE_PUZZLE, OBUS_PUZZLE_ID_DEVELOPMENT);
	// Needy: a module that periodically requires an action not to get strikes
	// obus_module::setup(OBUS_TYPE_NEEDY, OBUS_NEEDY_ID_DEVELOPMENT);
}

obus_can::message message;

void loop() {
	bool is_message_valid = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);
	// bool is_message_valid = obus_module::loopNeedy(&message, callback_game_start, callback_game_stop);
}

void callback_game_start(uint8_t puzzle_modules) {

}

void callback_game_stop() {

}
