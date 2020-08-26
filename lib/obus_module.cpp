#include "obus_can.h"
#include "obus_module.h"

namespace obus_module {

struct obus_can::module this_module;
uint8_t strike_count;


void setup(uint8_t type, uint8_t id) {
	this_module.type = type;
	this_module.id = id;

	obus_can::init();

	strike_count = 0;
}

void loop() {

}

void strike() {
	strike_count++;
	obus_can::send_m_strike(this_module, strike_count);
}

void solve() {
	obus_can::send_m_solved(this_module);
}

}
