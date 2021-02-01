// (c) 2020, redfast00
// See the LICENSE file for conditions for copying


// A red button
// A green button
// A blue led (with 330 ohm resistor)

#include <obus_module.h>
#include <ezButton.h>

#define PIN_RED_BUTTON   5
#define PIN_GREEN_BUTTON 6
#define PIN_LED_BLUE     9

ezButton red_button(PIN_RED_BUTTON);
ezButton green_button(PIN_GREEN_BUTTON);

bool blue_state = false;
bool checking_input = false;

void setup() {
	Serial.begin(115200);
	obus_module::setup(OBUS_TYPE_PUZZLE, OBUS_PUZZLE_ID_DEVELOPMENT);
	red_button.setDebounceTime(100);
	green_button.setDebounceTime(100);
	pinMode(BLUE_LED, OUTPUT);
}

obus_can::message message;

void loop() {
	bool received = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);
	// TODO handle update frames (not needed for this module, but could be useful as example code)

	red_button.loop();
	green_button.loop();

	if (checking_input) {
		if (red_button.getCount() > 0) {
			if (blue_state) {
				obus_module::strike();
			} else {
				obus_module::solve();
				checking_input = blue_state = false;
			}
		}

		if (green_button.getCount() > 0) {
			if (blue_state) {
				obus_module::solve();
				checking_input = blue_state = false;
			} else {
				obus_module::strike();
			}
		}
	}
	red_button.resetCount();
	green_button.resetCount();

	digitalWrite(BLUE_LED, blue_state);
}

void callback_game_start(uint8_t puzzle_modules_connected) {
  (void)puzzle_modules_connected;
	blue_state = random(0, 2);
	checking_input = true;
}

void callback_game_stop() {
	blue_state = checking_input = false;
}
