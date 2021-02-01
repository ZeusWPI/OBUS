#include <obus_can.h>
#include <TM1638plus.h>


#define STATE_INACTIVE 0
#define STATE_INFO     1
#define STATE_HELLO    2
#define STATE_GAME     3
#define STATE_GAMEOVER 4

#define OBUS_MAX_STRIKES    3 // Number of strikes allowed until game over
#define OBUS_GAME_DURATION 60 // Duration of the game in seconds

#define OBUS_MAX_MODULES      16
#define OBUS_INFO_DURATION     3 // Duration of discovery round in seconds
#define OBUS_DISC_DURATION     5 // Duration of discovery round in seconds
#define OBUS_UPDATE_INTERVAL 500 // Number of milliseconds between game updates

#define OBUS_GAME_DURATION_MS ((uint32_t) OBUS_GAME_DURATION*1000)
#define OBUS_DISC_DURATION_MS ((uint32_t) OBUS_DISC_DURATION*1000)
#define OBUS_INFO_DURATION_MS ((uint32_t) OBUS_INFO_DURATION*1000)

#define DIVIDE_CEIL(dividend, divisor) ((dividend + (divisor - 1)) / divisor)
#define MAX_AMOUNT_PUZZLES 256 // The ID of a puzzle is uint8


uint8_t state = STATE_INACTIVE;
struct obus_can::module connected_modules_ids[OBUS_MAX_MODULES];
uint8_t nr_connected_modules;
uint8_t nr_connected_puzzles;
uint8_t nr_solved_puzzles;
uint8_t strikes;

// Bitvector for checking if game is solved or not
// 8 bits per uint8 bitvector field
#define N_UNSOLVED_PUZZLES DIVIDE_CEIL(MAX_AMOUNT_PUZZLES, 8)
uint8_t unsolved_puzzles[N_UNSOLVED_PUZZLES];

// Timers
uint32_t hello_round_start;
uint32_t info_round_start;
uint32_t game_start;
uint32_t last_draw;
uint32_t last_update;

struct obus_can::module this_module = {
	.type = OBUS_TYPE_CONTROLLER,
	.id = OBUS_CONTROLLER_ID
};


// For the display/button chip
#define  STROBE_TM 4
#define  CLOCK_TM 6
#define  DIO_TM 7
#define  HI_FREQ false // If using a high freq CPU > ~100 MHZ set to true.
TM1638plus tm(STROBE_TM, CLOCK_TM , DIO_TM, HI_FREQ);


void setup() {
	Serial.begin(19200);
	obus_can::init();

	state = STATE_INACTIVE;

	tm.displayBegin();
}


bool check_solved() {
	for (uint8_t i = 0; i < N_UNSOLVED_PUZZLES; i++) {
		if (unsolved_puzzles[i] != 0) {
			return false;
		}
	}
	return true;
}


void add_puzzle_to_bit_vector(uint8_t module_id) {
	uint8_t byte_index = module_id >> 3;
	uint8_t bit_index = module_id & 0x07;
	unsolved_puzzles[byte_index] |= 0x1 << bit_index;
}


void solve_puzzle_in_bit_vector(uint8_t module_id) {
	uint8_t byte_index = module_id >> 3;
	uint8_t bit_index = module_id & 0x07;
	if (unsolved_puzzles[byte_index] & (0x1 << bit_index)) {
		nr_solved_puzzles++;
	}
	unsolved_puzzles[byte_index] &= ~(0x1 << bit_index);
}

void start_info() {
	state = STATE_INFO;
	info_round_start = millis();
	obus_can::send_c_infostart(this_module);
	Serial.println(F("  Start of info round"));
	tm.displayText("InFO");
}

void wait_info() {
	struct obus_can::message msg;
	obus_can::receive(&msg);

	if (millis() - info_round_start > OBUS_INFO_DURATION_MS) {
		start_hello();
	}
}


void start_hello() {
	state = STATE_HELLO;
	hello_round_start = millis();
	nr_connected_modules = 0;
	nr_connected_puzzles = 0;

	// Zero bit vectors
	for (uint8_t i = 0; i < N_UNSOLVED_PUZZLES; i++) {
		unsolved_puzzles[i] = 0;
	}

	obus_can::send_c_hello(this_module);

	Serial.println(F("  Start of discovery round"));
	tm.displayText("dISCOvEr");
}


uint16_t full_module_id(struct obus_can::module mod) {
	return \
		(((uint16_t) mod.type) << 8) | \
		((uint16_t) mod.id);
}


void receive_hello() {
	struct obus_can::message msg;
	uint32_t current_time = millis();

	if (obus_can::receive(&msg)) {
		if (msg.msg_type == OBUS_MSGTYPE_M_HELLO) {
			if (nr_connected_modules < OBUS_MAX_MODULES) {
				Serial.print(F("  Registered module "));
				Serial.println(full_module_id(msg.from));

				connected_modules_ids[nr_connected_modules] = msg.from;
				nr_connected_modules++;

				if (msg.from.type == OBUS_TYPE_PUZZLE) {
					nr_connected_puzzles++;
					add_puzzle_to_bit_vector(msg.from.id);
				}

				char buffer[10];
				snprintf(buffer, 10, "%02d oF %02d", nr_connected_modules, OBUS_MAX_MODULES);
				tm.displayText(buffer);
			} else {
				Serial.println(F("W Max # modules reached"));
			}

			obus_can::send_c_ack(this_module, msg.from);
			Serial.println("  ACK");
		}

	} else if (current_time - hello_round_start > OBUS_DISC_DURATION_MS) {
		if (nr_connected_puzzles == 0) {
			hello_round_start = current_time;
			obus_can::send_c_hello(this_module);
			Serial.println(F("  No puzzle modules, resend hello"));
		} else {
			Serial.println(F("  End of discovery round"));
			initialize_game();
		}
	}
}


void initialize_game() {
	strikes = 0;
	nr_solved_puzzles = 0;
	game_start = millis();

	last_draw = 0;
	last_update = game_start;
	state = STATE_GAME;

	Serial.println("  Game started");

	draw_display(millis(), OBUS_GAME_DURATION_MS);
	obus_can::send_c_gamestart(this_module, OBUS_GAME_DURATION_MS, strikes, OBUS_MAX_STRIKES, nr_solved_puzzles);
}


void receive_module_update() {
	struct obus_can::message msg;

	if (obus_can::receive(&msg)) {

		switch (msg.msg_type) {
			case OBUS_MSGTYPE_M_STRIKE:
				// TODO check idempotency ID
				strikes++;
				break;

			case OBUS_MSGTYPE_M_SOLVED:
				solve_puzzle_in_bit_vector(full_module_id(msg.from));
				break;

			default:
				Serial.print(F("W Ignoring msg "));
				Serial.println(msg.msg_type);
				break;
		}

	}
}


void draw_display(uint32_t current_time, uint32_t time_left) {
	if (last_draw + 100 <= current_time) {
		// +25 to avoid rounding down when the loop runs early
		int totaldecisec = (time_left + 25) / 100;
		int decisec = totaldecisec % 10;
		int seconds = (totaldecisec / 10) % 60;
		int minutes = (totaldecisec / 10 / 60) % 60;
		int hours = totaldecisec / 10 / 60 / 60;
		char buffer[10];
		snprintf(buffer, 10, "%01dh%02d %02d.%01d", hours, minutes, seconds, decisec);
		tm.displayText(buffer);
		last_draw = current_time;
	}
}


void game_loop() {
	uint32_t current_time = millis();
	uint32_t time_elapsed =  current_time - game_start;
	uint32_t time_left =
		OBUS_GAME_DURATION_MS < time_elapsed ? 0 : OBUS_GAME_DURATION_MS - time_elapsed;
		// We cannot check for '<= 0' in an uint type so we check the terms prior to subtraction

	receive_module_update();

	if (check_solved()) {
		Serial.println("  Game solved");
		obus_can::send_c_solved(this_module, time_left, strikes, OBUS_MAX_STRIKES, nr_solved_puzzles);
		state = STATE_GAMEOVER;
		tm.displayText("dISArmEd");
		return;
	}
	if (time_left == 0) {
		Serial.println("  Time's up");
		obus_can::send_c_timeout(this_module, time_left, strikes, OBUS_MAX_STRIKES, nr_solved_puzzles);
		state = STATE_GAMEOVER;
		tm.displayText(" boo   t");
		// m
		tm.display7Seg(4, 0b01010100);
		tm.display7Seg(5, 0b01000100);
		return;
	}
	if (strikes >= OBUS_MAX_STRIKES) {
		Serial.println("  Strikeout");
		obus_can::send_c_strikeout(this_module, time_left, strikes, OBUS_MAX_STRIKES, nr_solved_puzzles);
		state = STATE_GAMEOVER;
		tm.displayText(" boo   S");
		// m
		tm.display7Seg(4, 0b01010100);
		tm.display7Seg(5, 0b01000100);
		return;
	}

	draw_display(current_time, time_left);

	if (last_update + OBUS_UPDATE_INTERVAL <= current_time) {
		obus_can::send_c_state(this_module, time_left, strikes, OBUS_MAX_STRIKES, nr_solved_puzzles);
		last_update = current_time;
	}
}


void loop() {
	switch (state) {
		case STATE_INACTIVE:
			start_info();
			break;

		case STATE_INFO:
			wait_info();
			break;

		case STATE_HELLO:
			receive_hello();
			break;

		case STATE_GAME:
			game_loop();
			break;

		case STATE_GAMEOVER:
			break;
	}
}
