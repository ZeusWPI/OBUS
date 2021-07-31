// (c) 2020, redfast00
// See the LICENSE file for conditions for copying

#include <obus_module.h>
#include <LiquidCrystal.h>

// Pins for the Hitachi HD44780 LCD controller
#define PIN_LCD_RS     7 // Register Select. RS=0: Command, RS=1: Data
#define PIN_LCD_ENABLE 6 // Enable (aka Clock). A falling edge on this pin triggers execution
#define PIN_LCD_D0     5
#define PIN_LCD_D1     4
#define PIN_LCD_D2     3
#define PIN_LCD_D3     2

#define SERIAL_NUMBER_SIZE 7

uint8_t serial_number[SERIAL_NUMBER_SIZE];

// true when we need to update the LCD display
bool render_now = false;
LiquidCrystal lcd(
		PIN_LCD_RS,
		PIN_LCD_ENABLE,
		PIN_LCD_D0,
		PIN_LCD_D1,
		PIN_LCD_D2,
		PIN_LCD_D3
);


void setup() {
	Serial.begin(115200);
	lcd.begin(16, 2);
	lcd.clear();
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
