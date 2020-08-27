#include "obus_can.h"
#include "obus_module.h"

#define RED_LED A4
#define GREEN_LED A5

#define MCP_INT 2

namespace obus_module {


struct obus_can::module this_module;
uint8_t strike_count;
bool active;
uint32_t time_stop_strike_led;

void setup(uint8_t type, uint8_t id) {
	this_module.type = type;
	this_module.id = id;

	obus_can::init();

	strike_count = 0;
	active = false;
	pinMode(RED_LED, OUTPUT);
	pinMode(GREEN_LED, OUTPUT);
	digitalWrite(RED_LED, LOW);
	digitalWrite(GREEN_LED, LOW);
}

bool loopPuzzle(obus_can::message* message) {
	// Check if we need to turn the red "strike" LED back off after
	//  turning it on because of a strike
	if (time_stop_strike_led && time_stop_strike_led > millis()) {
		digitalWrite(RED_LED, LOW);
		time_stop_strike_led = 0;
	}
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
			delay(500);
		}
	}
	if (obus_can::receive(message)) {
		switch(message->msg_type) {
			case OBUS_MSGTYPE_C_GAMESTART:
				active = true;
				callback_game_start();
				break;
			case OBUS_MSGTYPE_C_HELLO:
				obus_can::send_m_hello(this_module);
				break;
			case OBUS_MSGTYPE_C_SOLVED:
			case OBUS_MSGTYPE_C_TIMEOUT:
			case OBUS_MSGTYPE_C_STRIKEOUT:
				active = false;
				callback_game_stop();
				break;
			case OBUS_MSGTYPE_C_ACK:
				break;
			case OBUS_MSGTYPE_C_STATE:
				return true;
		}
		return false;
	}
}

bool loopNeedy(obus_can::message* message) {
	// For now this is the same function
	return loopPuzzle(message);
}

void strike() {
	if (!active) {
		return;
	}
	strike_count++;
	digitalWrite(RED_LED, HIGH);
	time_stop_strike_led = millis() + 1000;
	obus_can::send_m_strike(this_module, strike_count);
}

void solve() {
	if (!active) {
		return;
	}
	obus_can::send_m_solved(this_module);
	digitalWrite(GREEN_LED, HIGH);
	active = false;
}

bool is_active() {
	return active;
}

}
