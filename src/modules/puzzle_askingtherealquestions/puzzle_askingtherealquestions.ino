#include "PCF8575.h"
#include <obus_module.h>
#include <ezButton.h>

#define RESET_BUTTON_PIN A3
#define START_BUTTON_PIN A0
#define STOP_BUTTON_PIN A2
#define MODE_SWITCH A1

#define OBUS_PUZZLE_ID 3

// These are IO expanders, connected to the LED display
PCF8575 pcf_0(0x20);
PCF8575 pcf_1(0x22);

ezButton stop_button(STOP_BUTTON_PIN);
ezButton reset_button(RESET_BUTTON_PIN);
ezButton start_button(START_BUTTON_PIN);
ezButton switch_button(MODE_SWITCH);

uint8_t generatedSolutionNumber[6] = {0, 0, 0, 0, 0, 0}; // TODO

enum ValueState {
	WRONG_VALUE = 0,
	CORRECT_VALUE_WRONG_POSITION = 1,
	CORRECT_VALUE_CORRECT_POSITION = 2
};
uint8_t wrongValue = 0;
uint8_t correctValueWrongPosition = 1;
uint8_t correctValueCorrectPosition = 2;

/*
Eerste 6 display 
	-> een random serie 
	   van bovenstaande getallen die matchen met de input
Laatste display -> tries
dials -> guess
*/

/*
	0-9, 
	10     -> blocky 8
	11, 12 -> blank
	13     -> -
	14, 15 -> blank
*/
uint8_t control_value[6] = {0, 0, 0, 0, 0, 0};
uint8_t display_value[7] = {13, 13, 13, 13, 13, 13, 13};

// Arduino pins the dials are connected to
uint8_t dial_pins[6] = {2, 3, 5, 6, 9, 10};

// Values for analogRead() for the dials. Maps dial position to expected value.
int16_t dial_analog_values[10] = {6, 85, 160, 214, 280, 314, 353, 382, 410, 430};

// warning, the "5"-position is broken on two of the dials and sometimes only retuns some of the bits it's composed of
uint8_t dial_readout[6] = {0};


#include <stdlib.h>

/* Arrange the N elements of ARRAY in random order.
   Only effective if N is much smaller than RAND_MAX;
   if this may not be the case, use a better random
   number generator. 
   https://benpfaff.org/writings/clc/shuffle.html
*/
void shuffle(uint8_t *array, size_t n)
{
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i+=1) {
          size_t j = random(n);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

void setup()
{
	Serial.begin(115200);
	pcf_0.begin();
	pcf_1.begin();
	Serial.print("Connected? : ");
	Serial.println(pcf_0.isConnected());
	Serial.println(pcf_1.isConnected());
	delay(1000);
	for (int i = 0; i < 6; i++) {
		pinMode(dial_pins[i], OUTPUT);
		digitalWrite(dial_pins[i], LOW);
	}
	pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
	pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
	pinMode(START_BUTTON_PIN, INPUT_PULLUP);
	pinMode(MODE_SWITCH, INPUT_PULLUP);

	stop_button.setDebounceTime(100);

	// Choose one
	// Puzzle: a module that must be solved
	obus_module::setup(OBUS_TYPE_PUZZLE, OBUS_PUZZLE_ID);
	callback_game_start(0); // TODO remove
}

obus_can::message message;

void updateDisplay() {
	uint8_t shift_to_pin[4] = {2, 0, 1, 3};
	for (uint8_t display = 0; display < 7; display++) {
		for (uint8_t shift = 0; shift < 4; shift++) {
			uint8_t pin = shift_to_pin[shift];
			if (display >= 4) {
				pcf_1.write((display - 4) * 4 + pin, 0 < (display_value[display] & (1 << shift)));
			} else {
				pcf_0.write(display * 4 + pin, 0 < (display_value[display] & (1 << shift)));
			}
		}
	}
}

void readDial() {
	// Reads all dials by turning on dial pins one by one, then measuring the
	//  voltage in the resistor network
	for (int dial = 0; dial < 6; dial++) {
		digitalWrite(dial_pins[dial], HIGH);
		delay(1);
		int16_t measured = analogRead(A7);
		digitalWrite(dial_pins[dial], LOW);
		int16_t minimal_difference = 1024;
		uint8_t best_candidate = 0;
		for(uint8_t i = 0; i < 10; i++) {
			int16_t difference = abs(measured - dial_analog_values[i]);
			if (difference < minimal_difference) {
				minimal_difference = difference;
				best_candidate = i;
			}
		}
		dial_readout[dial] = best_candidate;
	}
}


uint8_t groups[3][3] = {
	{0, 1, 2},
	{1, 2, 3},
	{3, 2, 1}
};

uint8_t group_offsets[3][6] = {
	{0, 0, 0, 0, 0, 0},
	{1, 2, 3, 4, 5, 0},
	{4, 3, 5, 0 ,2 ,1}
};

void loop()
{
	bool is_message_valid = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop, callback_game_info, callback_game_state);
	
	readDial();

	stop_button.loop();
	start_button.loop();

	// switch_button.loop();

	// Serial.println("switch state");
	// Serial.println(switch_button.getState());

	if (stop_button.getCount() > 0) {
		obus_module::strike();
		display_value[0] += 1;
	}
	stop_button.resetCount();
	int group = 0;

	if (start_button.getCount() > 0) {
		bool all_correct = true;
		for(int i = 0; i < 6; i += 1) {
			int valueState = 0;
			if(dial_readout[5-i] == generatedSolutionNumber[i]){
				valueState = CORRECT_VALUE_CORRECT_POSITION;
			} else {
				all_correct = false;
				size_t index = 0;
				while(index < 6 && dial_readout[5-i] != generatedSolutionNumber[index]) {
					index += 1;
				}
				if(index < 6) {
					valueState = CORRECT_VALUE_WRONG_POSITION;
				} else {
					valueState = WRONG_VALUE;
				}
			}

			display_value[i+1] = valueState;
		}
		if (all_correct) {
			obus_module::solve();
		}

		// Serial.println("Entry:");
		// for(int i = 0; i < 6; i += 1) {
		// 	Serial.print("[");
		// 	Serial.print(i);
		// 	Serial.print("]: ");
		// 	Serial.println(display_value[i]);
		// }
	}
	start_button.resetCount();

	/*
	for (int i = 0; i < 6; i++) {
		display_value[6 - i] = dial_readout[i];
	}*/
	updateDisplay();
}

void callback_game_start(uint8_t puzzle_modules) {
	Serial.println("Solution:");
	for(int i = 0; i < 6; i += 1) {
		generatedSolutionNumber[i] = random(10);
		Serial.print("[");
		Serial.print(i);
		Serial.print("]: ");
		Serial.println(generatedSolutionNumber[i]);
	}

}

void callback_game_stop() {
	((void (*)(void))0)();
}

void callback_game_info(uint8_t info_id, uint8_t infomessage[7]){
	Serial.println("INFO");
	Serial.println(info_id);
	// Info modules
}

void callback_game_state(uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_left){
	Serial.println("Game state:");
	Serial.print("\tTime left: ");
	Serial.println(time_left);
	Serial.print("\tStrikes: ");
	Serial.println(strikes);
	Serial.print("\tMax strikes: ");
	Serial.println(max_strikes);
}