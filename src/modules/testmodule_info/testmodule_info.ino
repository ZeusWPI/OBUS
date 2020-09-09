// (c) 2020, redfast00
// See the LICENSE file for conditions for copying

#include <obus_module.h>
#include <LiquidCrystal.h>


uint8_t serial_number[7];
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);


void setup() {
	Serial.begin(115200);
	obus_module::setup(OBUS_TYPE_INFO, OBUS_INFO_ID_DEVELOPMENT);
	info_generator(serial_number);
	lcd.begin(16, 2);
	lcd.clear();
}

obus_can::message message;

void loop() {
	bool is_message_valid = obus_module::loopInfo(&message, info_generator);
	lcd.home();
	for (int i = 0; i < 7; i++) {
		lcd.write(serial_number[i]);
	}
}

int info_generator(uint8_t* buffer) {
	serial_number[0] = random('A', 'Z');
	serial_number[1] = random('A', 'Z');
	serial_number[2] = random('A', 'Z');
	serial_number[3] = random('0', '9');
	serial_number[4] = random('0', '9');
	serial_number[5] = random('A', 'Z');
	serial_number[6] = random('0', '9');
	memcpy(buffer, serial_number, 7);
	lcd.clear();
	return 7;
}
