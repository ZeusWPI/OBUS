// (c) 2020, redfast00
// See the LICENSE file for conditions for copying

#include <obus_module.h>
#include <ezButton.h>

ezButton red_button(5);
ezButton green_button(6);

void setup() {
	Serial.begin(115200);
	obus_module::setup(OBUS_TYPE_PUZZLE, OBUS_PUZZLE_ID_DEVELOPMENT);
	red_button.setDebounceTime(100);
	green_button.setDebounceTime(100);
}

obus_can::message message;

void loop() {
	bool received = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);
	// TODO handle update frames (not needed for this module, but could be useful as example code)

	red_button.loop();
	green_button.loop();

	if (red_button.getCount() > 0) {
		red_button.resetCount();
		obus_module::strike();
	}

	if (green_button.getCount() > 0) {
		green_button.resetCount();
		obus_module::solve();
	}
}

void callback_game_start(uint8_t puzzle_modules_connected) {
	// Intentionally emtpy
	(void)puzzle_modules_connected;
}

void callback_game_stop() {
	// Intentionally empty
}
