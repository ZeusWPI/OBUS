// (c) 2022, redfast00
// See the LICENSE file for conditions for copying

#include <obus_module.h>

void setup() {
	Serial.begin(115200);
	obus_module::setup(OBUS_TYPE_PUZZLE, 0);
}

obus_can::message message;

void loop() {
	bool is_message_valid = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);
}

void callback_game_start() {

}

void callback_game_stop() {

}
