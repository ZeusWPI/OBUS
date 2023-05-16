// (c) 2020, redfast00
// See the LICENSE file for conditions for copying

/* Format of the info message

length = 7 bytes
id = 1

message:
X X X X X X X
------------- > the serial number (consisting of 7 ascii characters), not null-terminated

The serial number consists of capital letters and numbers, and is guaranteed to contain
at least one capital letter and one number.
*/

#include <Wire.h>
#include <obus_module.h>
#include <LiquidCrystal_I2C.h>

#define SERIAL_NUMBER_SIZE 7
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

uint8_t serial_number[SERIAL_NUMBER_SIZE] = {'A', '0', '0', '0', '0', '0', '0'};

// true when we need to update the LCD display
bool render_now = true;


void setup() {
	Serial.begin(115200);
	lcd.init(); // initialize the lcd
	lcd.backlight();
	lcd.setCursor(0, 0);
	lcd.print("init");
	obus_module::setup(OBUS_TYPE_INFO, 1);
}

obus_can::message message;

void loop() {
	obus_module::loopInfo(&message, info_generator);
	if (render_now) {
		render_now = false;
		lcd.home();
		lcd.clear();
		lcd.print("Serial Number");
		lcd.setCursor(0, 1);
		for (int i = 0; i < SERIAL_NUMBER_SIZE; i++) {
			lcd.write(serial_number[i]);
		}
	}
}

int info_generator(uint8_t* buffer) {
	uint8_t location_of_digit = random(SERIAL_NUMBER_SIZE);
	uint8_t location_of_letter = random(SERIAL_NUMBER_SIZE);
	if (location_of_digit == location_of_letter) {
		location_of_letter = (location_of_letter + 1) % SERIAL_NUMBER_SIZE;
	}
	for (int i = 0; i < SERIAL_NUMBER_SIZE; i++) {
		if (i == location_of_digit) {
			serial_number[i] = '0' + random(10);
		} else if (i == location_of_letter) {
			serial_number[i] = 'A' + random(26);
		} else {
			uint8_t generated = random(26 + 10);
			if (generated < 26)
				serial_number[i] = 'A' + generated;
			else
				serial_number[i] = '0' + (generated - 26);
		}
	}
	memcpy(buffer, serial_number, SERIAL_NUMBER_SIZE);
	render_now = true;
	return SERIAL_NUMBER_SIZE;
}
