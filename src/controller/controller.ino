#include "shared.hpp"
#include "obus_can.hpp"


#define STATE_INACTIVE 0
#define STATE_HELLO    1
#define STATE_GAME     2

#define OBUS_MAX_STRIKES    3   // Number of strikes allowed until game over
#define OBUS_GAME_DURATION 10 // Duration of the game in seconds


#define OBUS_GAME_DURATION_MS ((uint32_t) OBUS_GAME_DURATION*1000)


uint8_t state = STATE_INACTIVE;
struct module connected_modules_ids[OBUS_MAX_MODULES];
uint8_t nr_connected_modules;
uint8_t strikes;

// Bit vectors for checking if game is solved or not
uint8_t unsolved_puzzles[32]; // 256 bits

// TIMERS
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


uint8_t check_solved() {
	uint8_t solved = 1;
	for (uint8_t i = 0; i < 32; i++) {
		if (unsolved_puzzles != 0) {
			solved = 0;
			break;
		}
	}
	return solved;
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
	for (uint8_t i = 0; i < 32; i++) {
		unsolved_puzzles[i] = 0;
	}

	struct obus_message msg = obuscan_msg_c_hello(this_module);
	obuscan_send(&msg);

	Serial.println(F("Start of discovery round"));
}


void send_ack() {
	struct obus_message msg = obuscan_msg_c_ack(this_module);
	obuscan_send(&msg);
}


void receive_hello() {
	struct obus_message msg;
	uint32_t current_time = millis();

	if (obuscan_receive(&msg)) {
		if (msg.msg_type ==  OBUS_MSGTYPE_M_HELLO) {
			Serial.print("Registered module ");
			Serial.println(full_module_id(msg.from));
			connected_modules_ids[nr_connected_modules] = msg.from;
			nr_connected_modules++;

			if (msg.from.type == OBUS_TYPE_PUZZLE) {
				add_module_to_bit_vector(full_module_id(msg.from));
			}

			send_ack();
			Serial.println("ACK");
		}
	} else if (current_time - hello_round_start > OBUS_DISC_DURATION_MS) {
		Serial.println("End of discovery round");
		initialize_game();
	}
}


void initialize_game() {
	strikes = 0;
	game_start = millis();

	last_update = game_start;
	state = STATE_GAME;

	Serial.println("Game started");

	send_game_update(OBUS_MSGTYPE_C_GAMESTART, OBUS_GAME_DURATION_MS);
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


void send_game_update(uint8_t msg_type, uint32_t time_left) {
	Serial.print(F("Send "));
	Serial.print(msg_type);
	Serial.print(F(": "));
	Serial.print(time_left);
	Serial.print(F(", "));
	Serial.print(strikes);
	Serial.print(F("/"));
	Serial.println(OBUS_MAX_STRIKES);

	struct obus_message msg = obuscan_msg_c_payld_gamestatus(
			this_module, false, msg_type, time_left, strikes, OBUS_MAX_STRIKES);
	obuscan_send(&msg);
}


void game_loop() {
	uint32_t current_time = millis();
	uint32_t time_elapsed =  current_time - game_start;
	uint32_t time_left =
		OBUS_GAME_DURATION_MS < time_elapsed ? 0 : OBUS_GAME_DURATION_MS - time_elapsed;
		// We cannot check for '<= 0' in an uint type so we check the terms prior to subtraction

	receive_module_update();

	if (check_solved()) {
		Serial.println("Game solved");
		send_game_update(OBUS_MSGTYPE_C_SOLVED, time_left);
		state = STATE_INACTIVE;
		return;
	} else if (time_left == 0) {
		Serial.println("Time's up");
		send_game_update(OBUS_MSGTYPE_C_TIMEOUT, time_left);
		state = STATE_INACTIVE;
		return;
	} else if (strikes >= OBUS_MAX_STRIKES) {
		Serial.println("Strikeout");
		send_game_update(OBUS_MSGTYPE_C_STRIKEOUT, time_left);
		state = STATE_INACTIVE;
		return;
	}

	if (last_update + OBUS_UPDATE_INTERVAL <= current_time) {
		send_game_update(OBUS_MSGTYPE_C_STATE, time_left);
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
