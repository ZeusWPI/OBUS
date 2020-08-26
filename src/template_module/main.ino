// (c) {YEAR}, {AUTHOR}
// See the LICENSE file for conditions for copying

#include <obus_module.h>

void setup() {
	Serial.begin(9600);

	// Choose one
	// Puzzle: a module that must be solved
	obusmodule_init(OBUS_TYPE_PUZZLE, /* Retrieve ID from MOANA */);
	// Needy: a module that periodically requires an action not to get strikes
	// obusmodule_init(OBUS_TYPE_NEEDY, /* Retrieve ID from MOANA */);
}


void loop() {
	obusmodule_loop();
}
