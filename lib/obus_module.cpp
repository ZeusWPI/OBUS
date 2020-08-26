#include "obus_can.h"
#include "obus_module.h"

#define RED_LED A4
#define GREEN_LED A5

#define MCP_INT 2

namespace obus_module {


struct obus_can::module this_module;
uint8_t strike_count;
bool running;
bool error;
uint32_t time_stop_strike_led;

void interrupt_can_error() {
	error = true;
}

void setup(uint8_t type, uint8_t id) {
	this_module.type = type;
	this_module.id = id;

	obus_can::init();

	strike_count = 0;
	running = false;
	error = false;
	pinMode(RED_LED, OUTPUT);
	pinMode(GREEN_LED, OUTPUT);
	digitalWrite(RED_LED, LOW);
	digitalWrite(GREEN_LED, LOW);
}

bool loop(obus_can::message* message) {
	// Check if the message buffer overflowed
	if (error) {
		// Loop forever while blinking status led orange
		bool blink = false;
		while (true) {
			digitalWrite(RED_LED, blink);
			digitalWrite(GREEN_LED, blink);
			blink = !blink;
			delay(500);
		}
	}
	if (time_stop_strike_led && time_stop_strike_led > millis()) {
		digitalWrite(RED_LED, LOW);
	}
	// TODO receive CAN frame and call callback functions

}

void strike() {
	strike_count++;
	digitalWrite(RED_LED, HIGH);
	time_stop_strike_led = millis() + 1000;
	obus_can::send_m_strike(this_module, strike_count);
}

void solve() {
	obus_can::send_m_solved(this_module);
	digitalWrite(GREEN_LED, HIGH);
	running = false;
}

}
