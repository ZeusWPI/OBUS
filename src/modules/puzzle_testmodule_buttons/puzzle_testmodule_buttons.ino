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

void setup() {
	Serial.begin(115200);
	obus_module::setup(OBUS_TYPE_PUZZLE, OBUS_PUZZLE_ID_DEVELOPMENT);
	red_button.setDebounceTime(100);
	green_button.setDebounceTime(100);
	pinMode(PIN_LED_BLUE, OUTPUT);
}

obus_can::message message;

void loop() {
	obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);

	red_button.loop();
	green_button.loop();

	// Check if this module is active: the timer is counting down and this module hasn't been solved yet
	if (obus_module::is_active()) {
		if (red_button.getCount() > 0) {
			if (blue_state) {
				obus_module::strike();
			} else {
				obus_module::solve();
			}
		}

		if (green_button.getCount() > 0) {
			if (blue_state) {
				obus_module::solve();
			} else {
				obus_module::strike();
			}
		}
	}
	// Button state is cleared regardless of if this game is running, this is done
	// to prevent presses accumulating when game is not running and then all getting
	// read at once as soon as the game starts
	red_button.resetCount();
	green_button.resetCount();

	digitalWrite(PIN_LED_BLUE, blue_state);
}

void callback_game_start(uint8_t puzzle_modules_connected) {
	// Ignore the amount of puzzle modules connected
	(void)puzzle_modules_connected;
	blue_state = random(0, 2);
}

void callback_game_stop() {
	blue_state = false;
}
