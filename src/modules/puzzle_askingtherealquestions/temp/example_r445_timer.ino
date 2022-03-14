#include "PCF8575.h"

#define RESET_BUTTON_PIN A3
#define START_BUTTON_PIN A0
#define STOP_BUTTON_PIN A2
#define MODE_SWITCH A1

// These are IO expanders, connected to the LED display
PCF8575 pcf_0(0x20);
PCF8575 pcf_1(0x22);

// Goes from 0-9, then a blocky 8, -, then blank
uint8_t display_value[7] = {11, 11, 11, 11, 11, 11, 11};

// Arduino pins the dials are connected to
uint8_t dial_pins[6] = {2, 3, 5, 6, 9, 10};

// Values for analogRead() for the dials. Maps dial position to expected value.
int16_t dial_analog_values[10] = {6, 85, 160, 214, 280, 314, 353, 382, 410, 430};

// warning, the "5"-position is broken on two of the dials and sometimes only retuns some of the bits it's composed of
uint8_t dial_readout[6] = {0};

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
}

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
		for (uint8_t i = 0; i < 10; i++) {
			int16_t difference = abs(measured - dial_analog_values[i]);
			if (difference < minimal_difference) {
				minimal_difference = difference;
				best_candidate = i;
			}
		}
		dial_readout[dial] = best_candidate;
	}
}


void loop()
{
	readDial();
	for (int i = 0; i < 6; i++) {
		display_value[6 - i] = dial_readout[i];
	}
	updateDisplay();
}
