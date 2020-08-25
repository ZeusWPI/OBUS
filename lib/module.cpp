#include "obus_can.h"
#include "module.h"


struct module this_module;
uint8_t strike_count;


void obusmodule_setup(uint8_t type, uint8_t id) {
	this_module.type = type;
	this_module.id = id;

	obuscan_init();

	strike_count = 0;
}

void obusmodule_loop() {

}

void obusmodule_strike() {
	strike_count++;
	obuscan_send_m_strike(this_module, strike_count);
}

void obusmodule_solve() {
	obuscan_send_m_solved(this_module);
}
