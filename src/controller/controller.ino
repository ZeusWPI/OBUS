#include <obus_can.h>


#define STATE_INACTIVE 0
#define STATE_HELLO    1
#define STATE_GAME     2

#define OBUS_MAX_STRIKES    3 // Number of strikes allowed until game over
#define OBUS_GAME_DURATION 10 // Duration of the game in seconds

#define OBUS_MAX_MODULES      16
#define OBUS_DISC_DURATION     5 // Duration of discovery round in seconds
#define OBUS_UPDATE_INTERVAL 500 // Number of milliseconds between game updates

#define OBUS_GAME_DURATION_MS ((uint32_t) OBUS_GAME_DURATION*1000)
#define OBUS_DISC_DURATION_MS ((uint32_t) OBUS_DISC_DURATION*1000)

#define DIVIDE_CEIL(dividend, divisor) ((dividend + (divisor - 1)) / divisor)
#define MAX_AMOUNT_PUZZLES 256 // The ID of a puzzle is uint8


uint8_t state = STATE_INACTIVE;
struct module connected_modules_ids[OBUS_MAX_MODULES];
uint8_t nr_connected_modules;
uint8_t strikes;

// Bitvector for checking if game is solved or not
// 32 bits per uint32 bitvector field
#define N_UNSOLVED_PUZZLES DIVIDE_CEIL(MAX_AMOUNT_PUZZLES, 32)
uint32_t unsolved_puzzles[N_UNSOLVED_PUZZLES];

// Timers
uint32_t hello_round_start;
uint32_t game_start;
uint32_t last_update;

struct module this_module = {
	.type = OBUS_TYPE_CONTROLLER,
	.id = OBUS_CONTROLLER_ID
};


void setup() {
	Serial.begin(9600);
	obuscan_init();

	state = STATE_INACTIVE;
}


bool check_solved() {
	for (uint8_t i = 0; i < N_UNSOLVED_PUZZLES; i++) {
		if (unsolved_puzzles[i] != 0) {
			return false;
		}
	}
	return true;
}


void add_module_to_bit_vector(uint8_t module_id) {
	uint8_t byte_index = module_id >> 3;
	uint8_t bit_index = module_id & 0x07;
	unsolved_puzzles[byte_index] |= 0x1 << bit_index;
}


void solve_module_in_bit_vector(uint8_t module_id) {
	uint8_t byte_index = module_id >> 3;
	uint8_t bit_index = module_id & 0x07;
	unsolved_puzzles[byte_index] &= ~(0x1 << bit_index);
}


void start_hello() {
	state = STATE_HELLO;
	hello_round_start = millis();
	nr_connected_modules = 0;

	// Zero bit vectors
	for (uint8_t i = 0; i < N_UNSOLVED_PUZZLES; i++) {
		unsolved_puzzles[i] = 0;
	}

	obuscan_send_c_hello(this_module);

	Serial.println(F("  Start of discovery round"));
}


uint16_t full_module_id(struct module mod) {
	return \
		((uint16_t) mod.type << 8) | \
		(uint16_t) mod.id;
}


void receive_hello() {
	struct obus_message msg;
	uint32_t current_time = millis();

	if (obuscan_receive(&msg)) {
		if (msg.msg_type == OBUS_MSGTYPE_M_HELLO) {
			Serial.print("  Registered module ");
			Serial.println(full_module_id(msg.from));

			if (nr_connected_modules < OBUS_MAX_MODULES) {
				connected_modules_ids[nr_connected_modules] = msg.from;
				nr_connected_modules++;

				if (msg.from.type == OBUS_TYPE_PUZZLE) {
					add_module_to_bit_vector(full_module_id(msg.from));
				}
			}

			obuscan_send_c_ack(this_module);
			Serial.println("  ACK");
		}
	} else if (current_time - hello_round_start > OBUS_DISC_DURATION_MS) {
		Serial.println("  End of discovery round");
		initialize_game();
	}
}


void initialize_game() {
	strikes = 0;
	game_start = millis();

	last_update = game_start;
	state = STATE_GAME;

	Serial.println("  Game started");

	obuscan_send_c_gamestart(this_module, OBUS_GAME_DURATION_MS, strikes, OBUS_MAX_STRIKES);
}


void receive_module_update() {
	struct obus_message msg;

	if (obuscan_receive(&msg)) {

		switch (msg.msg_type) {
			case OBUS_MSGTYPE_M_STRIKE:
				// TODO check idempotency ID
				strikes++;
				break;

			case OBUS_MSGTYPE_M_SOLVED:
				solve_module_in_bit_vector(full_module_id(msg.from));
				break;

			default:
				Serial.print(F("W Ignoring msg "));
				Serial.println(msg.msg_type);
				break;
		}

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
		obuscan_send_c_solved(this_module, time_left, strikes, OBUS_MAX_STRIKES);
		state = STATE_INACTIVE;
		return;
	}
	if (time_left == 0) {
		Serial.println("  Time's up");
		obuscan_send_c_timeout(this_module, time_left, strikes, OBUS_MAX_STRIKES);
		state = STATE_INACTIVE;
		return;
	}
	if (strikes >= OBUS_MAX_STRIKES) {
		Serial.println("  Strikeout");
		obuscan_send_c_strikeout(this_module, time_left, strikes, OBUS_MAX_STRIKES);
		state = STATE_INACTIVE;
		return;
	}

	if (last_update + OBUS_UPDATE_INTERVAL <= current_time) {
		obuscan_send_c_state(this_module, time_left, strikes, OBUS_MAX_STRIKES);
		last_update = current_time;
	}
}


void loop() {
	switch (state) {
		case STATE_INACTIVE:
			start_hello();
			break;

		case STATE_HELLO:
			receive_hello();
			break;

		case STATE_GAME:
			game_loop();
			break;
	}
}
