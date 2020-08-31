// (c) {YEAR}, {AUTHOR}
// See the LICENSE file for conditions for copying

#include <obus_module.h>

void setup() {
	Serial.begin(9600);

	// Choose one
	// Puzzle: a module that must be solved
	obus_module::setup(OBUS_TYPE_PUZZLE, OBUS_PUZZLE_ID_DEVELOPMENT);
	// Needy: a module that periodically requires an action not to get strikes
	// obus_module::setup(OBUS_TYPE_NEEDY, OBUS_NEEDY_ID_DEVELOPMENT);
}


void loop() {
	obus_module::loop();
}
