// (c) 2022, redfast00
// See the LICENSE file for conditions for copying

#include <obus_module.h>

// The shaping multiplier button has a bad contact when held down: wiggling the button will break/make contact
// #define SHAPING_MULTIPLIER_PIN 10

#define INPUT_POLARITY_PIN A2
#define RESTORER_RATE_PIN A3
#define RESTORER_MODE_PIN A4
#define THRESHOLD_PIN A5
#define PUR_PIN 9
#define VAR_LED 5
#define DISC_LED 6
#define COARSE_GAIN_NETWORK_PIN A0
#define SHAPING_TIME_NETWORK_PIN A1

// Gets the position of a dial, given the pin connected to the resistor network
uint8_t get_resistor_network_pin_index(uint8_t pin) {
	int expected_values[6] = {0, 254, 407, 509, 582, 638};
	int measured = analogRead(pin);
	int minimal_difference = 1024;
	uint8_t best_candidate = 0;
	for (uint8_t i = 0; i < 6; i++) {
		int difference = abs(measured - expected_values[i]);
		if (difference < minimal_difference) {
			minimal_difference = difference;
			best_candidate = i;
		}
	}
	return best_candidate;
}

int gCoarseGainExpected;
int gShapingTimeExpected;

void setup() {
	Serial.begin(115200);
	obus_module::setup(OBUS_TYPE_PUZZLE, 1);

	// Switches and buttons
	pinMode(INPUT_POLARITY_PIN, INPUT_PULLUP);
	pinMode(RESTORER_RATE_PIN, INPUT_PULLUP);
	pinMode(RESTORER_MODE_PIN, INPUT_PULLUP);
	pinMode(THRESHOLD_PIN, INPUT_PULLUP);
	pinMode(PUR_PIN, INPUT_PULLUP);

	// LEDs
	pinMode(VAR_LED, OUTPUT);
	pinMode(DISC_LED, OUTPUT);

	// resistor networks
	pinMode(COARSE_GAIN_NETWORK_PIN, INPUT);
	pinMode(SHAPING_TIME_NETWORK_PIN, INPUT);
  randomSeed(12);
  gCoarseGainExpected = random(0, 6);
  gShapingTimeExpected = random(0, 6);
  Serial.print(gCoarseGainExpected);
  Serial.print(gShapingTimeExpected);
}



obus_can::message message;



void loop() {
	obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);
   
	// Some demo code to show everything works
	bool var_value = false; //digitalRead(INPUT_POLARITY_PIN) ^ digitalRead(RESTORER_MODE_PIN) ^ digitalRead(RESTORER_RATE_PIN) ^ digitalRead(THRESHOLD_PIN);
	bool disc_value = (get_resistor_network_pin_index(COARSE_GAIN_NETWORK_PIN) == gCoarseGainExpected and get_resistor_network_pin_index(SHAPING_TIME_NETWORK_PIN) == gShapingTimeExpected); //(get_resistor_network_pin_index(COARSE_GAIN_NETWORK_PIN) % 2) ^ (get_resistor_network_pin_index(SHAPING_TIME_NETWORK_PIN) % 2) ^ digitalRead(PUR_PIN);
	digitalWrite(VAR_LED, var_value);
	digitalWrite(DISC_LED, disc_value);
  
}

void callback_game_start(uint8_t puzzle_modules) {
	// just instantly solve
	obus_module::solve();
}

void callback_game_stop() {

}
