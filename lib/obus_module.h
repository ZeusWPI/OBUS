#ifndef OBUS_MODULE_H
#define OBUS_MODULE_H

#include "Arduino.h"
#include <obus_can.h>

void callback_game_start();

void callback_game_stop();

namespace obus_module {

void setup(uint8_t type, uint8_t id);

bool loopPuzzle(obus_can::message* message);

bool loopNeedy(obus_can::message* message);

void strike();

void solve();

bool is_active();

}

#endif /* end of include guard: OBUS_MODULE_H */
