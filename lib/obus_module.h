#ifndef OBUS_MODULE_H
#define OBUS_MODULE_H

#include "Arduino.h"
#include <obus_can.h>

#define OBUS_PUZZLE_ID_DEVELOPMENT 255
#define OBUS_NEEDY_ID_DEVELOPMENT  255
#define OBUS_INFO_ID_DEVELOPMENT   255

namespace obus_module {

void setup(uint8_t type, uint8_t id);

void loop();

void strike();

void solve();

}

#endif /* end of include guard: OBUS_MODULE_H */
