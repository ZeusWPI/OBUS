/* Format of the info message

length = 1 byte
id = 3

message:
X
-> binary value

There are 4 bits in the value that indicate 

VGA:      8-bit
Serial:   4-bit
MIDI:     2-bit
Parallel: 1-bit

If a bit is 0, this means the port is shown, if it is 1, the port is hidden.

For example, if the value is 7 (in binary: 0111), that means that only the VGA port is visible.
*/


#include <Stepper.h>
#include <obus_module.h>

const int stepsPerRevolution = 2038;

#define IN_ONE 2
#define IN_TWO 3
#define IN_THREE 5
#define IN_FOUR 6

Stepper stepper = Stepper(stepsPerRevolution, IN_ONE, IN_THREE, IN_TWO, IN_FOUR);

#define LIMIT_SWITCH_PIN 9

bool needs_calibration = false;
int calib_steps_to_do = 0;
int steps_to_do = 0;

void updatemotor() {
  if (calib_steps_to_do > 0) {
    stepper.step(1);
    calib_steps_to_do--;
  }
  else if (needs_calibration) {
    if (digitalRead(LIMIT_SWITCH_PIN)) {
      // Button not pushed down
      stepper.step(1);
    } else {
      // Button pushed down for first time, will be at 0 within stepsPerRevolution/64
      needs_calibration = false;
    }
  } else {
    if (steps_to_do > 0) {
      stepper.step(1);
      steps_to_do--;
    } else {
      digitalWrite(IN_ONE, LOW);
      digitalWrite(IN_TWO, LOW);
      digitalWrite(IN_THREE, LOW);
      digitalWrite(IN_FOUR, LOW);
    }
  }
}

void setPosition(uint8_t index) {
  needs_calibration = true;
  int slotsToTurn = (16 - index) % 16;
  steps_to_do = stepsPerRevolution/64 + (slotsToTurn*(stepsPerRevolution/16));
  if (!digitalRead(LIMIT_SWITCH_PIN)) {
    calib_steps_to_do = stepsPerRevolution / 16;
  }
}

void setup() {
  Serial.begin(115200);
  stepper.setSpeed(5);
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  obus_module::setup(OBUS_TYPE_INFO, 3);
}

uint8_t port_position = 0;
obus_can::message message;
void loop() {
  obus_module::loopInfo(&message, info_generator);
  updatemotor();
}

int info_generator(uint8_t* buffer) {
	uint8_t location = random(0), 15);
  buffer[0] = location;
  setPosition(location);
	return 1;
}