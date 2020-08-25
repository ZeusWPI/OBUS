// (c) {YEAR}, {AUTHOR}
// See the LICENSE file for conditions for copying

#include <obus_module.h>

void setup() {
	Serial.begin(9600);
	obus_module_init();
}


void loop() {
	obus_module_loop();
}
