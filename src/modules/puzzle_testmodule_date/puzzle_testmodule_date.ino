// (c) 2020, timpy
// See the LICENSE file for conditions for copying

#include <obus_module.h>
#include <ezButton.h>


// yellow
#define DATE_CLOCK_PIN 6
// orange
#define DATE_DATA_PIN 7
// green
#define DATE_READ_PIN 5

// Solve button
#define DATE_SOLVE_BTN 3

#define DATE_MODE_STOP 1
#define DATE_MODE_1S 2
#define DATE_MODE_10S 4
#define DATE_MODE_1M 8
#define DATE_MODE_10M 16
#define DATE_MODE_TIME_IN 32
#define DATE_MODE_DATE_IN 64
#define DATE_MODE_REAL_OUT 128
#define DATE_MODE_NORM_OUT 0


uint8_t correct_code[4] = {
	DATE_MODE_STOP,
	12,
	34,
	56
};

ezButton solve_button(DATE_SOLVE_BTN);


void setup() {
	Serial.begin(115200);
	obus_module::setup(OBUS_TYPE_PUZZLE, 123);
	solve_button.setDebounceTime(10);
	solve_button.setCountMode(COUNT_RISING);
	setup_date_module();
}

obus_can::message message;

void loop() {
	bool received = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);
	// TODO handle update frames (not needed for this module, but could be useful as example code)
	solve_button.loop();
	if (solve_button.getCount() > 0) {
		uint8_t data[4];
		read_from_date_module(data);

		if (check_date_code(data)) {
			Serial.println("SOLVED!");
			obus_module::solve();
		} else {
			Serial.println("STRIKE!");
			obus_module::strike();
		}
		solve_button.resetCount();
	}
}

void setup_date_module() {
	pinMode(DATE_CLOCK_PIN, OUTPUT);
	pinMode(DATE_DATA_PIN, INPUT);
	pinMode(DATE_READ_PIN, OUTPUT);
}

uint8_t read_value_from_date_module(uint8_t bit_order) {

	digitalWrite(DATE_CLOCK_PIN, LOW);
	uint8_t value = 0;
	for (int i = 0; i < 8; i++) {
		uint8_t read_bit = digitalRead(DATE_DATA_PIN);
		if (bit_order == LSBFIRST)
			value |= read_bit << i;
		else
			value |= read_bit << (7 - i);
		digitalWrite(DATE_CLOCK_PIN, HIGH);
		delayMicroseconds(3);
		digitalWrite(DATE_CLOCK_PIN, LOW);
	}
	return value;
}

void read_from_date_module(uint8_t* data_out) {
	digitalWrite(DATE_READ_PIN, HIGH);
	delayMicroseconds(3);
	digitalWrite(DATE_READ_PIN, LOW);

	for (int i = 0; i < 4; i++) {
		data_out[i] = read_value_from_date_module(LSBFIRST);
	}

	for (int i = 1; i < 4; i++) {
		// Convert raw data to human interpretable number
		data_out[i] = 10*(data_out[i] & 0x0F) + ((data_out[i] & 0xF0) >> 4);
	}


	//for (int i = 0; i < 4; i++) {
	//  Serial.print(data_out[i]);
	//  Serial.print(" ");
	//}

	//Serial.println();
	digitalWrite(DATE_CLOCK_PIN, LOW);
}

bool check_date_code(uint8_t* code) {
		for (int i = 0; i < 4; i++) {
			if (code[i] != correct_code[i]) {
				return false;
			}
		}
		return true;
}

void callback_game_start(uint8_t puzzle_modules_connected) {
	// Intentionally emtpy
	(void)puzzle_modules_connected;
}

void callback_game_stop() {
	// Intentionally empty
}
