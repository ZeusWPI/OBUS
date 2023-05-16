#include "obus_can.h"
#include "obus_module.h"

#define PIN_LED_RED 4
#define PIN_LED_GREEN 7

#define BLINK_DELAY_SLOW 1000
#define BLINK_DELAY_FAST 300
#define BLINK_DELAY_VERY_FAST 100

#define MAX_TIME_BETWEEN_CALLS 100

#define COLOR_OFF    ((struct color) {false, false})
#define COLOR_RED    ((struct color) {true,  false})
#define COLOR_GREEN  ((struct color) {false, true})
#define COLOR_YELLOW ((struct color) {true,  true})

namespace obus_module {

struct obus_can::module this_module;
uint8_t strike_count;
bool active;
bool acked_after_last_hello = false;
uint32_t next_loop_call_deadline;

// Current LED status
struct color { bool red; bool green; };
struct color led_color;
//   Keeps track of whether the LED is currently lit, when a blink pattern is active
bool blink_led_lit = false;
int blink_delay = 0;
unsigned long blink_next_time = 0;
uint32_t led_reset_time;


// Used in the puzzle module loop to spread 'register hello' messages
// Used in the info module loop to spread 'info' messages
uint32_t message_spread_timer = 0;

void _setLed(struct color color) {
	led_color = color;
	blink_delay = 0;
	led_reset_time = 0;

	digitalWrite(PIN_LED_RED, color.red ? HIGH : LOW);
	digitalWrite(PIN_LED_GREEN, color.green ? HIGH : LOW);
}

void _ledLoop() {
	// Check if we need to turn the LED back off, e.g. to reset the strike blinker
	if (led_reset_time && millis() > led_reset_time) {
		if (active) {
			_setLed(COLOR_YELLOW);
		} else {
			_setLed(COLOR_OFF);
		}
		led_reset_time = 0;
	}

	// Update blink of status LED
	if (blink_delay && millis() > blink_next_time) {
		blink_led_lit = !blink_led_lit;
		if (blink_led_lit) {
			digitalWrite(PIN_LED_RED, led_color.red ? HIGH : LOW);
			digitalWrite(PIN_LED_GREEN, led_color.green ? HIGH : LOW);
		} else {
			digitalWrite(PIN_LED_RED, false);
			digitalWrite(PIN_LED_GREEN, false);
		}

		blink_next_time = millis() + blink_delay;
	}
}

void _setLedBlink(struct color color, uint16_t delay) {
	led_color = color;
	blink_led_lit = false;
	blink_delay = delay;
	blink_next_time = millis();
	led_reset_time = 0;

	_ledLoop();
}

void blink_error(String message) {
	bool blink = false;
	digitalWrite(PIN_LED_GREEN, LOW);
	while (true) {
		digitalWrite(PIN_LED_RED, blink);
		blink = !blink;
		delay(BLINK_DELAY_VERY_FAST);
		Serial.println(message);
	}
}

void _resetState() {
	strike_count = 0;
	active = false;
	next_loop_call_deadline = 0;
	acked_after_last_hello = false;

	if (this_module.type == OBUS_TYPE_PUZZLE || this_module.type == OBUS_TYPE_NEEDY) {
		pinMode(PIN_LED_RED, OUTPUT);
		pinMode(PIN_LED_GREEN, OUTPUT);

		_setLedBlink(COLOR_GREEN, BLINK_DELAY_SLOW);
	}
}

void setup(uint8_t type, uint8_t id) {
	this_module.type = type;
	this_module.id = id;
	_resetState();

	if (!obus_can::init()) {
		blink_error(F("CAN init failed"));
	}
}

void empty_callback_info(uint8_t info_id, uint8_t infomessage[7]) {
	// Mark arguments as not used
	(void)info_id;
	(void)infomessage;
}

void empty_callback_state(uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_left) {
	// Mark arguments as not used
	(void)time_left;
	(void)strikes;
	(void)max_strikes;
	(void)puzzle_modules_left;
}

bool loopPuzzle(obus_can::message* message, void (*callback_game_start)(uint8_t puzzle_modules), void (*callback_game_stop)(), void (*callback_info)(uint8_t info_id, uint8_t infomessage[7]), void (*callback_state)(uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_left)) {
	// TODO this can be more efficient by only enabling error interrupts and
	//  reacting to the interrupt instead of checking if the flag is set in a loop
	// We will need to fork our CAN library for this, because the needed functions are private.
	// Also, we can't do this by default, because the INT pin is normally not connected to the board
	if (obus_can::is_error_condition()) {

		blink_error("E CAN error " + String(obus_can::get_error_flags(), BIN));
	}

	// Force the user of the library to periodically call loop
	if (next_loop_call_deadline != 0 && millis() > next_loop_call_deadline) {
		blink_error(F("E Missed loop deadline"));
	}
	next_loop_call_deadline = millis() + MAX_TIME_BETWEEN_CALLS;

	bool received_message = false;
	if (obus_can::receive(message)) {
		received_message = true;
		if (is_from_controller(message->from)) {
			uint32_t seed;
			switch (message->msg_type) {
				case OBUS_MSGTYPE_C_GAMESTART:
					if (acked_after_last_hello) {
						active = true;
						_setLed(COLOR_YELLOW);
						callback_game_start(message->gamestatus.puzzle_modules_left);
					}
					break;
				case OBUS_MSGTYPE_C_HELLO:
					_resetState();
					// wait a bit before sending this message
					// so we don't send all the register messages at the same time
					// assuming 50kbps and 4 seconds of hello round (there are 5, but we want to keep a margin)
					// and a message is 112 bits long (maximum length of total frame), we can send
					// about 14000 messages. There are only 3 * 255 possible info module ID's
					// we want to spread them as good as possible in the 4 seconds of transmit time
					// However, just linearly spreading the IDs to the transmit time is suboptimal:
					// the higher IDs will probably never be used
					// We first spread the messages in 16 slots based on the module type and 3 least significant bits,
					// then spread the messages in those slots based on the other 5 bits
					message_spread_timer = millis() + 250 * (((this_module.type & 0b10) << 2) | this_module.id & 0b111) + 8 * (this_module.id >> 3);
					break;
				case OBUS_MSGTYPE_C_SOLVED:
				case OBUS_MSGTYPE_C_TIMEOUT:
				case OBUS_MSGTYPE_C_STRIKEOUT:
					if (acked_after_last_hello) {
						active = false;
						_setLed(COLOR_OFF);
						callback_game_stop();
					}
					break;
				case OBUS_MSGTYPE_C_ACK:
					if (message->payload_address.type == this_module.type && message->payload_address.id == this_module.id) {
						acked_after_last_hello = true;
					}
					break;
				case OBUS_MSGTYPE_C_STATE:
					callback_state(message->gamestatus.time_left, message->gamestatus.strikes, message->gamestatus.max_strikes, message->gamestatus.puzzle_modules_left);
					break;
				case OBUS_MSGTYPE_C_INFOSTART:
					// Add module type and id to seed, to remove correlation in randomness between modules
					seed = message->infostart.seed + ((uint32_t) this_module.type << 8) + ((uint32_t) this_module.id);
					// randomSeed has no effect when called with 0 as seed, so we use
					//  a fallback value that is unlikely to collide with other frequently used seeds
					if (seed == 0) {
						seed--;
					}
					randomSeed(seed);
					break;
				default:
					break;
			}
		} else if (message->from.type == OBUS_TYPE_INFO) {
			uint8_t infobuffer[7] = {0};
			memcpy(infobuffer, message->infomessage.data, message->infomessage.len);
			callback_info(message->from.id, infobuffer);
		}
	}
	if (message_spread_timer != 0 && millis() > message_spread_timer) {
		message_spread_timer = 0;
		obus_can::send_m_hello(this_module);
	}
	_ledLoop();

	return received_message;
}

bool loopNeedy(obus_can::message* message, void (*callback_game_start)(uint8_t puzzle_modules), void (*callback_game_stop)(), void (*callback_info)(uint8_t info_id, uint8_t infomessage[7]), void (*callback_state)(uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_left)) {
	// For now this is the same function
	return loopPuzzle(message, callback_game_start, callback_game_stop, callback_info, callback_state);
}

bool loopInfo(obus_can::message* message, int (*info_generator)(uint8_t*)) {
	bool interesting_message = false;
	if (obus_can::receive(message)) {
		if (is_from_controller(message->from)) {
			switch (message->msg_type) {
				case OBUS_MSGTYPE_C_INFOSTART:
					{
						randomSeed(message->infostart.seed);
						// wait a bit before sending this message
						// so we don't send all the infomessages at the same time
						// assuming 50kbps and 4 seconds of info round (there are 5, but we want to keep a margin)
						// and a message is 112 bits long (maximum length of total frame), we can send
						// about 14000 messages. There are only 255 possible info module ID's
						// we want to spread them as good as possible in the 4 seconds of transmit time
						// However, just linearly spreading the IDs to the transmit time is suboptimal:
						// the higher IDs will probably never be used
						// We first spread the messages in 8 slots based on the 3 least significant bits,
						// then spread the messages in those slots based on the other 5 bits
						// !! Important assumption: the info_generator() module function should return very quickly
						message_spread_timer = millis() + 500 * (this_module.id & 0b111) + 16 * (this_module.id >> 3);
					}
					break;
				case OBUS_MSGTYPE_C_STATE:
					interesting_message = true;
					break;
				default:
					break;
			}
		}
	}

	if (message_spread_timer != 0 && millis() > message_spread_timer) {
		message_spread_timer = 0;
		uint8_t info_message[OBUS_PAYLD_INFO_MAXLEN];
		int len = info_generator(info_message);
		obus_can::send_i_infomessage(this_module, info_message, len);
	}

	return interesting_message;
}

void strike() {
	if (!active) {
		return;
	}
	strike_count++;
	_setLedBlink(COLOR_RED, BLINK_DELAY_FAST);
	led_reset_time = millis() + 2000;
	obus_can::send_m_strike(this_module, strike_count);
}

void solve() {
	if (!active) {
		return;
	}
	obus_can::send_m_solved(this_module);
	active = false;
	_setLed(COLOR_GREEN);
}

bool is_active() {
	return active;
}

}
