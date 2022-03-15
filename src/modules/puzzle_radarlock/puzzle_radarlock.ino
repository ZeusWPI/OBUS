// (c) 2022, Xander & Adrien {AUTHORS}
// See the LICENSE file for conditions for copying

#include <obus_module.h>
#include <Servo.h>

const int trigPin = PD5; // Trigger Pin of Ultrasonic Sensor
const int echoPin = PD3; // Echo Pin of Ultrasonic Sensor
const int servoPin = PD2; // Pin for Servo
const int submit_button = PD6; 

// ------Length of arrays should be consistent
const int ledPins[2] = {14,15}; // Led Pins

const int distances[2] = {10,10}; //test distances
const int angles[2] = {45,135};
bool user_solved[2] = {false, false};
//----------------

bool solved = false;
bool pressed = false;

int distance_bias;

const int number_of_distances = (sizeof(distances) / sizeof(distances[0]));

Servo servo;

void setup() {
	Serial.begin(115200);

	// Choose one
	// Puzzle: a module that must be solved
	obus_module::setup(OBUS_TYPE_PUZZLE, 5);
	// Needy: a module that periodically requires an action not to get strikes
	// obus_module::setup(OBUS_TYPE_NEEDY, OBUS_NEEDY_ID_DEVELOPMENT);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 

  // Set all leds mode to output
  for (int led : ledPins) {
    pinMode(led, OUTPUT);
  }    

  servo.attach(servoPin);
  pinMode(submit_button, INPUT_PULLUP);

}

obus_can::message message;

void loop() {
	bool is_message_valid = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);
	// bool is_message_valid = obus_module::loopNeedy(&message, callback_game_start, callback_game_stop);

  // 5-175 degrees
  if (solved) {
    return;
  }
  
  for(int ix = 0; ix < number_of_distances; ix++){
    distance_bias = 0.5 * distances[ix]; // room for error marge in distance
    
    servo.write(angles[ix]);
    timermil(30);
    int distance = calculateDistance();
    Serial.print(angles[ix]);
    Serial.print(":");
    Serial.print(distance);
    Serial.println();

    // turn led on if object is at least detected in range 
    if (distance < 80) {
      digitalWrite(ledPins[ix], HIGH);
    } else { // else turn led off
      digitalWrite(ledPins[ix], LOW);
    }

    // set angle solved if object is in desired range
    if (abs(distance - distances[ix]) < distance_bias){
      user_solved[ix] = true;
    } else {
      user_solved[ix] = false;
    }

     timermil(3000); // wait 3000 milliseconds
  }
}

// delay in microseconds for Ultrasonic sensor
void timermicro(long duration){
  long start =  micros();
  while (micros() - start < duration){}
}

// delay in milliseconds
void timermil(long duration){
  long start =  millis();
  while (millis() - start < duration){
        bool is_message_valid = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);
        if (digitalRead(submit_button) == 0){
          if(!pressed){
            submitPress();
          }
          pressed = true;
        } else {
          pressed = false;
        }
    }
}

void submitPress(){
    int iy = 0;
    for(iy=0; iy < number_of_distances && user_solved[iy] == true; iy++) {}
    if (iy == number_of_distances){
      // you only get here if every angle is solved and submit button is pressed
      solved = true;
      Serial.println("SOLVED (= ");
      obus_module::solve();
    } else{
      Serial.println("WRONG )= ");
      obus_module::strike();
  }
}


// calculate the distance measured by the Ultrasonic sensor
int calculateDistance(){ 
  digitalWrite(trigPin, LOW);   
  timermicro(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH); 
  timermicro(10);
  digitalWrite(trigPin, LOW);

  int duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  int distance = duration / 29 / 2; // convert to cm 
  return distance;
}


void callback_game_start(uint8_t puzzle_modules) {

}

void callback_game_stop() {

}

