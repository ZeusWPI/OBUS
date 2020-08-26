#ifndef OBUS_MODULE_H
#define OBUS_MODULE_H

#include "Arduino.h"
#include <obus_can.h>

namespace obus_module {

void setup(uint8_t type, uint8_t id);

void loop();

void strike();

void solve();

}

#endif /* end of include guard: OBUS_MODULE_H */
