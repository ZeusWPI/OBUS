#include "obus_can.h"
#include "obus_module.h"

#define RED_LED A4
#define GREEN_LED A5

#define BLINK_DELAY_SLOW 1000
#define BLINK_DELAY_NORMAL 500
#define BLINK_DELAY_FAST 300

#define MCP_INT 2

#define COLOR_OFF    ((struct color) {false, false})
#define COLOR_RED    ((struct color) {true,  false})
#define COLOR_GREEN  ((struct color) {false, true})
#define COLOR_YELLOW ((struct color) {true,  true})

namespace obus_module {

struct obus_can::module this_module;
uint8_t strike_count;
bool active;

// Current LED status
struct color { bool red; bool green; };
struct color led_color;
//   Keeps track of whether the LED is currently lit, when a blink pattern is active
bool blink_led_lit = false;
int blink_delay = 0;
unsigned long blink_next_time = 0;
uint32_t led_reset_time;


void _setLed(struct color color) {
	led_color = color;
	blink_delay = 0;
	led_reset_time = 0;

	digitalWrite(RED_LED, color.red ? HIGH : LOW);
	digitalWrite(GREEN_LED, color.green ? HIGH : LOW);
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
			digitalWrite(RED_LED, led_color.red ? HIGH : LOW);
			digitalWrite(GREEN_LED, led_color.green ? HIGH : LOW);
		} else {
			digitalWrite(RED_LED, false);
			digitalWrite(GREEN_LED, false);
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


void _resetState() {
	strike_count = 0;
	active = false;

	if (this_module.type == OBUS_TYPE_PUZZLE || this_module.type == OBUS_TYPE_NEEDY) {
		pinMode(RED_LED, OUTPUT);
		pinMode(GREEN_LED, OUTPUT);

		_setLedBlink(COLOR_GREEN, BLINK_DELAY_SLOW);
	}
}

void setup(uint8_t type, uint8_t id) {
	this_module.type = type;
	this_module.id = id;

	obus_can::init();

	_resetState();
}

bool loopPuzzle(obus_can::message* message, void (*callback_game_start)(), void (*callback_game_stop)()) {
	// TODO this can be more efficient by only enabling error interrupts and
	//  reacting to the interrupt instead of checking if the flag is set in a loop
	// We will need to fork our CAN library for this, because the needed functions
	//  are private
	if (obus_can::is_error_condition()) {
		bool blink = false;
		while (true) {
			digitalWrite(RED_LED, blink);
			digitalWrite(GREEN_LED, blink);
			blink = !blink;
			delay(BLINK_DELAY_NORMAL);
		}
	}

	bool interesting_message = false;
	if (obus_can::receive(message)) {
		if (message->from.type == OBUS_TYPE_CONTROLLER && message->from.id == 0) {
			switch (message->msg_type) {
				case OBUS_MSGTYPE_C_GAMESTART:
					active = true;
					_setLed(COLOR_YELLOW);
					callback_game_start();
					break;
				case OBUS_MSGTYPE_C_HELLO:
					_resetState();
					obus_can::send_m_hello(this_module);
					break;
				case OBUS_MSGTYPE_C_SOLVED:
				case OBUS_MSGTYPE_C_TIMEOUT:
				case OBUS_MSGTYPE_C_STRIKEOUT:
					active = false;
					_setLed(COLOR_OFF);
					callback_game_stop();
					break;
				case OBUS_MSGTYPE_C_ACK:
					break;
				case OBUS_MSGTYPE_C_STATE:
					interesting_message = true;
					break;
				default:
					break;
			}
		}
	}

	_ledLoop();

	return interesting_message;
}

bool loopNeedy(obus_can::message* message, void (*callback_game_start)(), void (*callback_game_stop)()) {
	// For now this is the same function
	return loopPuzzle(message, callback_game_start, callback_game_stop);
}

bool loopInfo(obus_can::message* message, int (*info_generator)(uint8_t*)) {
	bool interesting_message = false;
	if (obus_can::receive(message)) {
		if (message->from.type == OBUS_TYPE_CONTROLLER && message->from.id == 0) {
			switch (message->msg_type) {
				case OBUS_MSGTYPE_C_INFOSTART:
					{
						uint8_t info_message[OBUS_PAYLD_INFO_MAXLEN];
						int len = info_generator(info_message);
						obus_can::send_i_infomessage(this_module, info_message, len);
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
