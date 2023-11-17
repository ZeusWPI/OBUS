// (c) 2022, redfast00
// See the LICENSE file for conditions for copying
#include <obus_module.h>
#include <obus_util.h>
#include <Adafruit_GFX.h>                                                                           // Adafruit graphics library
#include <Adafruit_ST7735.h>                                                                        // Adafruit ST7735 display library
#include <SPI.h>


#define TFT_RST 5 // reset line (optional, pass -1 if not used)
#define TFT_CS 6  // chip select line
#define TFT_DC 9  // data/command line
#define YES_BUTTON A0
#define NO_BUTTON A1

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

uint16_t from_rgb(uint32_t rgb) {
	uint16_t red = (rgb >> 16) & 0xff;
	uint16_t green = (rgb >> 8) & 0xff;
	uint16_t blue = rgb & 0xff;

	return ((blue >> 3) << 11) | ((green >> 2) << 5) | (red >> 3);
}

#define AMOUNT_COLORS 7

const char* words[AMOUNT_COLORS] = {
	"ROOD",
	"GROEN",
	"BLAUW",
	"WIT",
	"PAARS",
	"GRIJS",
	"GEEL"
};
#define RED_IDX 0
#define GREEN_IDX 1
#define BLUE_IDX 2
#define WHITE_IDX 3
#define PURPLE_IDX 4
#define GREY_IDX 5
#define YELLOW_IDX 6

const uint32_t colors[AMOUNT_COLORS] = {
	0xff0000,
	0x00ff00,
	0x0000ff,
	0xffffff,
	0x640064,
	0x7f7f7f,
	0xffff33
};
#define PUZZLE_SIZE 8
uint8_t puzzle_colors[PUZZLE_SIZE] = {0};
uint8_t puzzle_words[PUZZLE_SIZE] = {0};
uint8_t press_idx = 0;
bool press_yes = false;

#define LOOP_IDX for (int i = 0; i < PUZZLE_SIZE; i++)
#define LOOP_JDX for (int j = 0; i < PUZZLE_SIZE; j++)

void generate_problem() {
	for (int i = 0; i < PUZZLE_SIZE; i++) {
		do {
			puzzle_colors[i] = random(AMOUNT_COLORS);
			puzzle_words[i] = random(AMOUNT_COLORS);
			// Assign random color and word; re-roll if the color and word are same as the one before
		} while (i > 0 && (puzzle_colors[i] == puzzle_colors[i-1] && puzzle_words[i] == puzzle_words[i-1]));
	}

	switch (puzzle_colors[0]) {
		case RED_IDX: { // red as first color
			LOOP_IDX {
				if (puzzle_colors[i] == puzzle_words[i]) {
					press_idx = i;
				 	press_yes = false;
					return;
				}
			}
			//
			int sum = 0;
			uint8_t possible_index = 0;
			LOOP_IDX {
				if (puzzle_colors[i] == RED_IDX) {
					sum++;
					possible_index = i;
				}
			}
			if (sum >= 2) {
				press_idx = possible_index;
				press_yes = true;
				return;
			}
			//
			press_idx = PUZZLE_SIZE - 1;
			press_yes = false;
			return;
		}
		case GREEN_IDX: { // green as first color
			// Set random (non-first) color to purple
			puzzle_words[random(1, PUZZLE_SIZE)] = PURPLE_IDX;
			// Color is blue and words is green
			LOOP_IDX {
				if (puzzle_colors[i] == BLUE_IDX && puzzle_words[i] == GREEN_IDX) {
					LOOP_JDX  {
						if (puzzle_colors[j] == BLUE_IDX) {
							press_idx = j;
							press_yes = false;
							return;
						}
					}
				}
			}
			// Color is red and (word is grey or yellow)
			LOOP_IDX {
				if (puzzle_colors[i] == RED_IDX && (puzzle_words[i] == GREY_IDX || puzzle_words[i] == YELLOW_IDX)) {
					press_idx = i;
					press_yes = false;
					return;
				}
			}
			// Word is grey and next color is white or green
			for (int i = 0; i < PUZZLE_SIZE - 1; i++) {
				if (puzzle_words[i] == GREY_IDX && (puzzle_colors[i+1] == WHITE_IDX || puzzle_colors[i+1] == GREEN_IDX)) {
					LOOP_JDX {
						if (puzzle_words[j] == GREY_IDX) {
							press_idx = j;
							press_yes = true;
							return;
						}
					}
				}
			}
			LOOP_IDX {
				if (puzzle_words[i] == PURPLE_IDX) {
					press_idx = i;
					press_yes = false;
					return;
				}
			}
			// should never be reached
			return;
		}
		case BLUE_IDX: {
			int ctr = 0;
			LOOP_IDX {
				if (puzzle_colors[i] != puzzle_words[i]) {
					ctr++;
					if (ctr == 4) {
						press_idx = i;
						press_yes = true;
						return;
					}
				}
			}
			for (int i = 0; i < PUZZLE_SIZE - 1; i++) {
				if (puzzle_colors[i] == puzzle_colors[i+1]) {
					press_idx = 2;
					press_yes = false;
					return;
				}
			}
			LOOP_IDX {
				if (puzzle_colors[i] == BLUE_IDX) {
					press_idx = i;
					press_yes = (i == 0);
				}
			}
			return;
		}
		case WHITE_IDX: {
			if (puzzle_words[3] == puzzle_colors[1] || (puzzle_words[3] == puzzle_colors[5])) {
				if (random(2)) {
					puzzle_words[PUZZLE_SIZE - 1] = GREEN_IDX;
				} else {
					puzzle_colors[PUZZLE_SIZE - 1] = GREEN_IDX;
				}
				LOOP_IDX {
					if (puzzle_words[i] == GREEN_IDX || puzzle_colors[i] == GREEN_IDX) {
						press_idx = i;
						press_yes = false;
						return;
					}
				}
			}
			int yellow_ctr = 0;
			int purple_ctr = 0;
			int last_yellow = 0;
			LOOP_IDX {
				if (puzzle_words[i] == YELLOW_IDX) {
					yellow_ctr++;
					last_yellow = i;
				}
				if (puzzle_colors[i] == PURPLE_IDX) {
					purple_ctr++;
				}
				if (yellow_ctr > purple_ctr) {
					press_idx = last_yellow;
					press_yes = false;
					return;
				}
			}
			press_idx = 3;
			press_yes = (puzzle_colors[4] == BLUE_IDX);
			return;
		}
		case PURPLE_IDX: {
			// set one word to white
			puzzle_words[random(PUZZLE_SIZE)] = WHITE_IDX;

			//
			if (puzzle_colors[1] == RED_IDX) {
				press_idx = 1;
				press_yes = true;
				return;
			} else if (puzzle_colors[2] == RED_IDX) {
				press_idx = 2;
				press_yes = true;
				return;
			}
			//
			LOOP_IDX {
				if (puzzle_words[i] == GREY_IDX && puzzle_colors[i] != WHITE_IDX) {
					press_idx = i;
					press_yes = false;
					return;
				}
			}
			//
			LOOP_IDX {
				// Guaranteed to happen
				if (puzzle_words[i] == WHITE_IDX) {
					press_idx = i;
					press_yes = false;
					return;
				}
			}
			return;
		}
		case GREY_IDX: {
			// set one color to purple
			puzzle_colors[random(1, PUZZLE_SIZE)] = PURPLE_IDX;
			for (int i = 0; i < PUZZLE_SIZE - 1; i++) {
				if (puzzle_colors[i] == puzzle_colors[i+1]) {
					LOOP_JDX {
						if (puzzle_colors[j] == GREY_IDX) {
							press_idx = j;
							press_yes = true;
						}
					}
					return; // last grey color
				}
			}
			// press first yellow color and 3 4
			if (puzzle_words[3] == puzzle_colors[4]) {
				LOOP_IDX {
					if (puzzle_colors[i] == YELLOW_IDX) {
						press_idx = i;
						press_yes = true;
						return;
					}
				}
			}
			// last time color is purple
			LOOP_IDX {
				if (puzzle_colors[i] == PURPLE_IDX) {
					press_idx = i;
					press_yes = false;
				}
			}
			return;
		}
		case YELLOW_IDX: {
			puzzle_colors[random(1, PUZZLE_SIZE)] = RED_IDX;
			LOOP_IDX {
				if (puzzle_colors[i] == puzzle_words[i]) {
					press_idx = i;
				 	press_yes = true;
					return;
				}
			}
			//
			int sum = 0;
			uint8_t possible_index = 0;
			LOOP_IDX {
				if (puzzle_colors[i] == YELLOW_IDX) {
					sum++;
					possible_index = i;
				}
			}
			if (sum >= 2) {
				press_idx = possible_index;
				press_yes = true;
				return;
			}
			// guaranteed to happen
			LOOP_IDX {
				if (puzzle_colors[i] == RED_IDX) {
					press_idx = i;
					press_yes = false;
					return;
				}
			}
			return;
		}
	}

}

Debounced yes_button;
Debounced no_button;

void setup() {
	Serial.begin(115200);
	tft.initR (INITR_MINI160x80);
	tft.setRotation (3);
	tft.invertDisplay(true);
	tft.setTextSize(4);
	pinMode(YES_BUTTON, INPUT_PULLUP);
	pinMode(NO_BUTTON, INPUT_PULLUP);
	obus_module::setup(OBUS_TYPE_PUZZLE, 6);
}

obus_can::message message;

uint8_t current_color_index = PUZZLE_SIZE;
uint32_t next_screen_update = 0;
bool solved = false;

void loop() {
	obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop);
	update_screen();
	if (obus_module::is_active()) {
		yes_button.loop(digitalRead(YES_BUTTON) == LOW);
		no_button.loop(digitalRead(NO_BUTTON) == LOW);
		// If both pressed, strike
		if (yes_button.get_press_count() && no_button.get_press_count()) {
			obus_module::strike();
		} else if (yes_button.get_press_count())
		{
			if (press_yes && current_color_index == press_idx) {
				obus_module::solve();
				solved = true;
				next_screen_update = 0;
			} else {
				obus_module::strike();
			}
		} else if (no_button.get_press_count()) {
			if ((!press_yes) && current_color_index == press_idx) {
				obus_module::solve();
				solved = true;
				next_screen_update = 0;
			} else {
				obus_module::strike();
			}
		}
		yes_button.reset_press_count();
		no_button.reset_press_count();
	}

}

void update_screen() {
	if (obus_module::is_active()) {
		if (millis() > next_screen_update) {
			tft.fillScreen(0); // make screen black
			current_color_index = (current_color_index + 1) % (PUZZLE_SIZE + 1);
			if (current_color_index < PUZZLE_SIZE) {
				tft.setCursor(10, 25);
				tft.setTextColor(from_rgb(colors[puzzle_colors[current_color_index]]));
				tft.print(words[puzzle_words[current_color_index]]);
			}
			next_screen_update = millis() + 1000;
		}
	} else {
		if (millis() > next_screen_update) {
			if (solved) {
				tft.fillScreen(from_rgb(colors[GREEN_IDX]));
			} else {
				tft.fillScreen(0);
			}
			next_screen_update = millis() + 1000;
		}
	}
}

void callback_game_start(uint8_t puzzle_modules) {
	generate_problem();
	next_screen_update = 0; // update screen now
}

void (*reset)(void) = 0;

void callback_game_stop() {
	reset();
}
