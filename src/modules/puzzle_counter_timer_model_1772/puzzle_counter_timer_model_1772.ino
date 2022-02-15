// (c) 2022, Tibo Ulens
// See the LICENSE file for conditions for copying

#include <obus_module.h>
#include <LedControl.h>

#include "lcd_control.h"

#define OBUS_PUZZLE_ID 2

void setup() {
	Serial.begin(115200);

	lcd.shutdown(0, false);
	lcd.setIntensity(0, 15); // 0: dimmest, 15: brightest
	lcd.clearDisplay(0);

	lcd_print_number(987654);

	// Puzzle: a module that must be solved
	obus_module::setup(OBUS_TYPE_PUZZLE, OBUS_PUZZLE_ID);
}

obus_can::message message;

void loop() {
	bool is_message_valid = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);
}

void callback_game_start(uint8_t message) {

}

void callback_game_stop() {

}
