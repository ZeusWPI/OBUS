// (c) 2022, Midgard
// See the LICENSE file for conditions for copying

#include <obus_module.h>

#define PIN_GAUGE   3
#define PIN_SLIDER A7

#define ADJUSTMENT_PERIOD_MS 15000
#define SLEEP_PERIOD_MS       5000

#define ARRAY_LENGTH(array) ((sizeof array)/(sizeof (array[0])))

uint32_t deadline = 0;
int16_t target = 0;
int16_t previousTarget = 0;
uint8_t moved = 0;


void setup() {
	Serial.begin(115200);

	obus_module::setup(OBUS_TYPE_NEEDY, OBUS_NEEDY_ID_DEVELOPMENT);

	pinMode(PIN_GAUGE, OUTPUT);
	pinMode(PIN_SLIDER, INPUT);

	target = getSliderValue();
	previousTarget = target;
}

uint16_t stops[] { 0, 18, 147, 526, 900, 1008, 1024 };
uint16_t getSliderValue() {
	uint16_t value = analogRead(PIN_SLIDER) + 1;
	for (int i = 1; i < ARRAY_LENGTH(stops); i++) {
		if (value <= stops[i]) return (value - stops[i - 1]) * 170 / (stops[i] - stops[i-1]) + (i-1)*170 - 1;
	}
}

void gauge(unsigned char value) {
	// analogWrite accepts values from 0 to 255
	analogWrite(PIN_GAUGE, value);
}

obus_can::message message;

bool is_in_tune(int16_t target, int16_t sliderValue) {
	return abs(target - sliderValue) <= 20;
}

/*
int amountOfLedsToLight(timeLeft, target, sliderValue) {
	if (is_in_tune(target, sliderValue)) {
		return 0;
	}
	if (timeLeft <= ADJUSTMENT_PERIOD_MS * 2 / 4) return 3;
	if (timeLeft <= ADJUSTMENT_PERIOD_MS * 3 / 4) return 2;
	if (timeLeft <= ADJUSTMENT_PERIOD_MS * 4 / 4) return 1;
}
*/


void loop() {
	bool is_message_valid = obus_module::loopNeedy(&message, callback_game_start, callback_game_stop);

	int16_t sliderValue = getSliderValue();
	int16_t gaugeValue = previousTarget - sliderValue;

	int32_t timeLeft = deadline - millis();
	if (timeLeft <= ADJUSTMENT_PERIOD_MS) {
		// Require a movement being detected for at least 2 successive loops, to combat jitter
		if (moved >= 2) {
			gaugeValue = target - sliderValue;
		} else {
			if (abs(sliderValue - previousTarget) > 2) moved++;
			else moved = 0;

			gaugeValue = (ADJUSTMENT_PERIOD_MS - timeLeft) * 127 / ADJUSTMENT_PERIOD_MS;
			if (target > previousTarget) {
				gaugeValue *= -1;
			}
		}
	}

	/*amountLeds = amountOfLedsToLight(timeLeft, target, sliderValue);*/

	if (timeLeft < 0) {
		deadline = millis() + ADJUSTMENT_PERIOD_MS + SLEEP_PERIOD_MS;
		previousTarget = sliderValue;
		do {
			target = random(1023 - 2*30) + 30;
		} while(abs(target - previousTarget) < 50);
		moved = 0;

		if (!is_in_tune(target, sliderValue)) {
			obus_module::strike();
			target = sliderValue;
		}
	}

	if (gaugeValue > 127) gaugeValue = 127;
	else if (gaugeValue < -127) gaugeValue = -127;
	gauge(gaugeValue + 127);

	delay(50);
}

void callback_game_start(uint8_t puzzle_modules) {

}

void callback_game_stop() {
	((void (*)(void))0)();
}
