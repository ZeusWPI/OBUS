#ifndef OBUS_MODULE_H
#define OBUS_MODULE_H

#include "Arduino.h"
#include <obus_can.h>

#define OBUS_PUZZLE_ID_DEVELOPMENT 255
#define OBUS_NEEDY_ID_DEVELOPMENT  255
#define OBUS_INFO_ID_DEVELOPMENT   255

namespace obus_module {

void setup(uint8_t type, uint8_t id);

bool loopPuzzle(obus_can::message* message, void (*callback_game_start)(), void (*callback_game_stop)());

bool loopNeedy(obus_can::message* message, void (*callback_game_start)(), void (*callback_game_stop)());

bool loopInfo(obus_can::message* message, int (*info_generator)(char*));

void strike();

void solve();

bool is_active();

}

#endif /* end of include guard: OBUS_MODULE_H */
