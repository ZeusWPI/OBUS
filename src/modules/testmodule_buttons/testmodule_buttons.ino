// (c) 2020, redfast00
// See the LICENSE file for conditions for copying

#include <obus_module.h>
#include <ezButton.h>

ezButton red_button(5);
ezButton green_button(6);

void setup() {
	Serial.begin(115200);
	// WARNING: do not use 255 for your module
	obus_module::setup(OBUS_TYPE_PUZZLE, 255);
	red_button.setDebounceTime(100);
	green_button.setDebounceTime(100);
}

obus_can::message message;

void loop() {
	bool received = obus_module::loopPuzzle(&message);
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

void callback_game_start() {
	// Intentionally emtpy
}

void callback_game_stop() {
	// Intentionally empty
}
