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
bool gInputPolarityExpected;
float gCoarseGainValues[] = {10,30,100,300,1000,3000};
float gShapingTimeValues[] = {0.25,1,1.5,4,5,6};

bool gRateExpected;
bool gModeExpected;
bool gThresholdExpected;
bool gPurExpected;

bool gRateBeginState;
bool gModeBeginState;
bool gThresholdBeginState;
bool gPurBeginState;

float magicFrequency(){
  int sign = -1;
  if(gInputPolarityExpected) { sign = 1; }
  return sign * gCoarseGainValues[gCoarseGainExpected] * gShapingTimeValues[gShapingTimeExpected];
}

int firstDigit(int x){
  int y = abs(x);
  while(y>9) {
    y /= 10;
  }
  return y;
}

bool floatEq(float x, float desired){
  return x >= desired -1 and x <= desired + 1;
}

void setExpectedButtons(){
  
  // default is all off
  gRateExpected = false;
gModeExpected = false;
gThresholdExpected = false;
gPurExpected = false;

float freq = magicFrequency();
if( freq >=59 and freq <= 351) { 
  if(gCoarseGainValues[gCoarseGainExpected] <=301) {
    gRateExpected = true;  
  }else{
    gPurExpected = true;
  }
  }

float coarse = gCoarseGainValues[gCoarseGainExpected];
  if((freq >= -144 and freq <= 4751) or floatEq(coarse,300)) {
    gModeExpected = false;
  }else{
    gModeExpected = true;
  }

  if(freq <= -3001 or freq >= 0) {
    gThresholdExpected = false;
  }

  int closestInt = freq;
  float shaping = gShapingTimeValues[gShapingTimeExpected];
  if(firstDigit(closestInt) % 2 == 0){
    if(floatEq(shaping, 1) or floatEq(shaping, 4) or floatEq(shaping, 6)){
      gPurExpected = true;     
    }
  }
  
  // at bottom, cause irregardles of other rules
  if(freq >= 3999 or floatEq(shaping, 6)){ gPurExpected = false;}
}

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

  setExpectedButtons();
  gRateBeginState = digitalRead(RESTORER_RATE_PIN);
  gModeBeginState = digitalRead(RESTORER_MODE_PIN);
  gThresholdBeginState = digitalRead(THRESHOLD_PIN);
  gPurBeginState = digitalRead(PUR_PIN);
}




obus_can::message message;

bool gSecondStage = false;

void loop() {
	obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);
  if(!gSecondStage){
	bool var_value = (gInputPolarityExpected == digitalRead(INPUT_POLARITY_PIN) and get_resistor_network_pin_index(COARSE_GAIN_NETWORK_PIN) == gCoarseGainExpected and get_resistor_network_pin_index(SHAPING_TIME_NETWORK_PIN) == gShapingTimeExpected);
  if(var_value) { 
    gSecondStage = true;
  }
  digitalWrite(VAR_LED, var_value);
  digitalWrite(DISC_LED, false);
  }else{
    bool correct = gRateExpected == digitalRead(RESTORER_RATE_PIN) and gModeExpected == digitalRead(RESTORER_MODE_PIN) and gThresholdExpected == digitalRead(THRESHOLD_PIN) and gPurExpected == digitalRead(PUR_PIN);
    digitalWrite(VAR_LED, true);
  digitalWrite(DISC_LED, correct);
  }
}

void callback_game_start(uint8_t puzzle_modules) {
	// just instantly solve
	obus_module::solve();
}

void callback_game_stop() {

}
