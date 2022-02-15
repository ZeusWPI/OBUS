// (c) 2022, redfast00
// See the LICENSE file for conditions for copying

#include <obus_module.h>
#include "LedControl.h"

#define LED_DIN_PIN 3
#define LED_CLK_PIN 2
#define LED_CS_PIN 5

// yellow = up A0
// blue = down A1
// orange = left A2
// green = right A3

LedControl lc = LedControl(LED_DIN_PIN, LED_CLK_PIN, LED_CS_PIN, 1);

void setup() {
	lc.shutdown(0, false);
	lc.setIntensity(0, 3);
	lc.clearDisplay(0);
	Serial.begin(115200);
	obus_module::setup(OBUS_TYPE_PUZZLE, 0);
}

obus_can::message message;

void loop() {
	bool is_message_valid = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);

	// Turn on all leds
	for (int row = 0; row < 8; row++) {
		for (int col = 0; col < 8; col++) {
			lc.setLed(0, row, col, true);
    }
  }
}

void callback_game_start(uint8_t puzzle_modules) {

}

void callback_game_stop() {

}
