// (c) 2020, redfast00
// See the LICENSE file for conditions for copying

#include <obus_module.h>
#include <ezButton.h>

#define PIN_GREEN_BUTTON 6
#define PIN_SPEAKER     10

ezButton green_button(PIN_GREEN_BUTTON);

void setup() {
	Serial.begin(115200);
	obus_module::setup(OBUS_TYPE_NEEDY, OBUS_NEEDY_ID_DEVELOPMENT);
	green_button.setDebounceTime(100);
}

obus_can::message message;

uint32_t next_activation_time = 0;
uint32_t trigger_time = 0;

void loop() {
	bool is_message_valid = obus_module::loopNeedy(&message, callback_game_start, callback_game_stop);
	green_button.loop();

	// Every second, have a 1/20 chance to trigger the countdown
	if (obus_module::is_active() && !trigger_time && (millis() > next_activation_time)) {
		next_activation_time = millis() + 1000;
		if (random(20) == 0) {
			trigger_time = millis() + 30000;
		}
	}

	// Strike if time runs out
	if (trigger_time && millis() > trigger_time) {
		obus_module::strike();
		trigger_time = 0;
	}

	// If the button is pressed, reset countdown if countdown is running, else strike
	if (green_button.getCount() > 0) {
		green_button.resetCount();
		if (trigger_time) {
			trigger_time = 0;
		} else {
			obus_module::strike();
		}
	}

	// Play the appropriate sound
	if (trigger_time && millis() > trigger_time - 15000) {
		tone(PIN_SPEAKER, 440);
	}
	else if (trigger_time) {
		tone(PIN_SPEAKER, 449);
	} else {
		noTone(PIN_SPEAKER);
	}
}

void callback_game_start(uint8_t puzzle_modules_connected) {
	// Intentionally emtpy
	(void)puzzle_modules_connected;
}

void callback_game_stop() {

}
