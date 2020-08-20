#ifndef OBUS_SHARED_H
#define OBUS_SHARED_H

#include "obus_can.hpp"

#define OBUS_MAX_MODULES      16
#define OBUS_DISC_DURATION     5  // Duration of discovery round in seconds
#define OBUS_UPDATE_INTERVAL 500 // Number of milliseconds between game updates

#define OBUS_DISC_DURATION_MS ((uint32_t) OBUS_DISC_DURATION*1000)

uint16_t full_module_id(struct module mod) {
	return \
		((uint16_t) mod.type << 8) | \
		(uint16_t) mod.id;
}

#endif /* end of include guard: OBUS_DEFS_H */
