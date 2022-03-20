// (c) 2022, redfast00, jasper_janin, robbe7730
// See the LICENSE file for conditions for copying

#include <obus_module.h>
#include <obus_util.h>
#include "LedControl.h"
#include "ezButton.h"

#define LED_DIN_PIN 3
#define LED_CLK_PIN 2
#define LED_CS_PIN 5

#define UP_BUTTON A0
#define DOWN_BUTTON A1
#define LEFT_BUTTON A2
#define RIGHT_BUTTON A3

#define BLINK_INTERVAL 250

// yellow = up A0
Debounced upButton = Debounced();
// blue = down A1
Debounced downButton = Debounced();
// orange = left A2
Debounced leftButton = Debounced();
// green = right A3
Debounced rightButton = Debounced();

int current_x;
int current_y;

int target_x;
int target_y;
int blink_timeout = 0;
long last_time = 0;

int selected_maze;

bool led_state = true;

bool playing = true;

#define NUM_MAZES 1
#define MAZE_SIZE 64

// From bottom to top
uint8_t mazes[MAZE_SIZE*NUM_MAZES] = {
  // MAZE 1
  0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
  0b1111, 0b0011, 0b1110, 0b0111, 0b1011, 0b0010, 0b0110, 0b1111,
  0b1111, 0b0001, 0b0010, 0b1100, 0b0011, 0b0100, 0b1101, 0b1111,
  0b1111, 0b0101, 0b1001, 0b0010, 0b0100, 0b1001, 0b0110, 0b1111,
  0b1111, 0b0101, 0b0011, 0b1100, 0b1001, 0b0110, 0b1101, 0b1111,
  0b1111, 0b1101, 0b0101, 0b1011, 0b1010, 0b0000, 0b0110, 0b1111,
  0b1111, 0b1011, 0b1000, 0b1110, 0b1011, 0b1100, 0b1101, 0b1111,
  0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111
};

// Position of the 3 lights on the edge, packed into one uint8
// 0bxxxxyyyy
uint8_t edge_points[NUM_MAZES * 3] = {
  // MAZE 1
  0b01100111, // (6, 7)
  0b00010000, // (1, 0)
  0b01100000, // (6, 0)
};

LedControl lc = LedControl(LED_DIN_PIN, LED_CLK_PIN, LED_CS_PIN, 1);

void(* crash) (void) = 0;

void setup() {
	lc.shutdown(0, false);
	lc.setIntensity(0, 3);
	lc.clearDisplay(0);
	Serial.begin(115200);
	obus_module::setup(OBUS_TYPE_PUZZLE, 0);

  selected_maze = 0;
  current_x = 1;
  current_y = 1;
  target_x = 6;
  target_y = 6;

  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
}

bool connected(int x, int y, int new_x, int new_y) {
  if (x < 0 || new_x < 0 || y < 0 || y < 0 || x > 7 || new_x > 7 || y > 7 || new_y > 7) {
    return false;
  }

  if (x == new_x && y == new_y) {
    return true;
  }
  
  uint8_t walls = mazes[selected_maze*MAZE_SIZE + y*8 + x];

  int dx = new_x - x;
  int dy = new_y - y;

  if ((dx != 0 && dy != 0) || dx > 1 || dx < -1 || dy > 1 || dy < -1) {
    return false;
  } 
  
  if (dy == 1) {
    return ((walls >> 3) & 1) == 0;
  } else if (dx == 1) {
    return ((walls >> 2) & 1) == 0;
  } else if (dy == -1) {
    return ((walls >> 1) & 1) == 0;
  } else if (dx == -1) {
    return (walls & 1) == 0;
  }

  return false;
}

void buttonLoop() {
  int new_x = current_x;
  int new_y = current_y;
  if (upButton.loop(!digitalRead(UP_BUTTON))) {
    new_y += 1;
  }
  if (downButton.loop(!digitalRead(DOWN_BUTTON))) {
    new_y -= 1;
  }
  if (leftButton.loop(!digitalRead(LEFT_BUTTON))) {
    new_x -= 1;
  }
  if (rightButton.loop(!digitalRead(RIGHT_BUTTON))) {
    new_x += 1;
  }

  if (!connected(current_x, current_y, new_x, new_y)) {
    obus_module::strike();
  } else {
    lc.setLed(0, current_x, current_y, false);
    current_x = new_x;
    current_y = new_y;
  }

  if (current_x == target_x && current_y == target_y) {
    Serial.print("Solved!");
    obus_module::solve();
    crash();
  }
}

void clear_screen() {
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      lc.setLed(0, x, y, false);
    }
  }
}

void draw_edges() {
  for (int i = 0; i < 3; i++) {
    uint8_t pos = edge_points[selected_maze * 3 + i];
    uint8_t x = pos >> 4;
    uint8_t y = pos & 0xf;

    lc.setLed(0, x, y, true);
  }
}

obus_can::message message;

void loop() {
	bool is_message_valid = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);

  buttonLoop();

  size_t m = millis();
  blink_timeout -= m - last_time;
  last_time = m;

  if (blink_timeout <= 0) {
    led_state = !led_state;
    blink_timeout = BLINK_INTERVAL;
  }

  lc.setLed(0, target_x, target_y, led_state);

  lc.setLed(0, current_x, current_y, true);
}

void callback_game_start(uint8_t puzzle_modules) {
  selected_maze = random(NUM_MAZES);

  current_x = random(1, 7);
  current_y = random(1, 7);

  target_x = current_x;
  while (abs(target_x - current_x) <= 2) {
    target_x = random(1, 7);
  }

  target_y = current_y;
  while (abs(target_y - current_y) <= 2) {
    target_y = random(1, 7);
  }

  clear_screen();

  draw_edges();
}

void callback_game_stop() {
    ((void (*)(void))0)();
}
