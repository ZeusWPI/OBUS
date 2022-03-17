// (c) 2022, Midgard
// See the LICENSE file for conditions for copying

#include <obus_module.h>

#define PIN_GAUGE   3
#define PIN_LED     2
#define PIN_SLIDER A7

#define ADJUSTMENT_PERIOD_MS 15000
#define SLEEP_PERIOD_MS      random(20000, 30000)

#define SLIDER_EDGE_AVOIDANCE 50

#define ARRAY_LENGTH(array) ((sizeof array)/(sizeof (array[0])))

uint32_t deadline = 0;
int16_t target = 0;
int16_t previousSliderValue = 0;
int16_t previousTarget = 0;
uint8_t moved = 0;


void setup() {
	Serial.begin(115200);
	Serial.println("Needy finetuner by Midgard");

	obus_module::setup(OBUS_TYPE_NEEDY, 0);

	pinMode(PIN_GAUGE, OUTPUT);
	pinMode(PIN_LED, OUTPUT);
	pinMode(PIN_SLIDER, INPUT);

	previousSliderValue = getSliderValue();
	target = previousSliderValue;
	previousTarget = target;
}

uint16_t stops[] { 0, 18, 147, 526, 900, 1008, 1024 };
uint16_t getSliderValue() {
	uint16_t value = analogRead(PIN_SLIDER) + 1;
	for (int i = 1; i < ARRAY_LENGTH(stops); i++) {
		if (value <= stops[i]) return (value - stops[i - 1]) * 170 / (stops[i] - stops[i-1]) + (i-1)*170 - 1;
	}
	return 0;
}

void gauge(unsigned char value) {
	// analogWrite accepts values from 0 to 255
	analogWrite(PIN_GAUGE, value);
}

obus_can::message message;

bool is_in_tune(int16_t target, int16_t sliderValue) {
	return abs(target - sliderValue) <= 20;
}


int ledPattern(int32_t timeLeft, int16_t target, int16_t sliderValue) {
	if (is_in_tune(target, sliderValue)) {
		return 0;
	}
	if (timeLeft <= ADJUSTMENT_PERIOD_MS / 4 * 1) return 3;
	if (timeLeft <= ADJUSTMENT_PERIOD_MS / 4 * 2) return 3;
	if (timeLeft <= ADJUSTMENT_PERIOD_MS / 4 * 3) return 2;
	if (timeLeft <= ADJUSTMENT_PERIOD_MS / 4 * 4) return 1;
	return 0;
}


int ledCycle = 0;

void loop() {
	bool is_message_valid = obus_module::loopNeedy(&message, callback_game_start, callback_game_stop);

	int16_t sliderValue = (getSliderValue() + previousSliderValue) / 2;
	previousSliderValue = getSliderValue();
	int16_t gaugeValue = previousTarget - sliderValue;

	int32_t timeLeft = deadline - millis();
	if (timeLeft <= ADJUSTMENT_PERIOD_MS) {
		// Require a movement being detected for at least 2 successive loops, to combat jitter
		if (moved >= 3) {
			gaugeValue = target - sliderValue;
		} else {
			if (abs(sliderValue - previousTarget) > 4) moved++;
			else moved = 0;

			gaugeValue = (ADJUSTMENT_PERIOD_MS - timeLeft) * 107 / ADJUSTMENT_PERIOD_MS + 20;
			if (target < previousTarget) {
				gaugeValue *= -1;
			}
		}
	}

	ledCycle++;
	bool ledOn = false;
	if (ledCycle > 0b1111) ledCycle = 0;
	switch (ledPattern(timeLeft, target, sliderValue)) {
		case 1: if ((ledCycle >> 3) & 1) ledOn = true; break;
		case 2: if ((ledCycle >> 2) & 1) ledOn = true; break;
		case 3: if ((ledCycle >> 1) & 1) ledOn = true; break;
		case 4: if ( ledCycle       & 1) ledOn = true; break;
		default: break;
	}
	if (ledOn) digitalWrite(PIN_LED, 1);
	else digitalWrite(PIN_LED, 0);

	if (timeLeft < 0) {
		Serial.println("---");

		if (!is_in_tune(target, sliderValue)) {
			obus_module::strike();
			Serial.println("Deadline missed, strike!");
		}

		deadline = millis() + ADJUSTMENT_PERIOD_MS + SLEEP_PERIOD_MS;
		previousTarget = sliderValue;
		do {
			target = random(1023 - 2*SLIDER_EDGE_AVOIDANCE) + SLIDER_EDGE_AVOIDANCE;
		} while(abs(target - previousTarget) < 50);
		moved = 0;
	}

	if (gaugeValue > 127) gaugeValue = 127;
	else if (gaugeValue < -127) gaugeValue = -127;
	gauge(gaugeValue + 127);

	delay(40);
}

void callback_game_start(uint8_t puzzle_modules) {

}

void callback_game_stop() {
	((void (*)(void))0)();
}
