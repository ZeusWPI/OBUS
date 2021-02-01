#ifndef OBUS_MODULE_H
#define OBUS_MODULE_H

#include "Arduino.h"
#include <obus_can.h>

#define OBUS_PUZZLE_ID_DEVELOPMENT 255
#define OBUS_NEEDY_ID_DEVELOPMENT  255
#define OBUS_INFO_ID_DEVELOPMENT   255

namespace obus_module {

void setup(uint8_t type, uint8_t id);

void empty_callback_info(uint8_t info_id, uint8_t infomessage[7]);
void empty_callback_state(uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_solved);

bool loopPuzzle(obus_can::message* message, void (*callback_game_start)(), void (*callback_game_stop)(), void (*callback_info)(uint8_t info_id, uint8_t infomessage[7]) = empty_callback_info, void (*callback_state)(uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_solved) = empty_callback_state);

bool loopNeedy(obus_can::message* message, void (*callback_game_start)(), void (*callback_game_stop)(), void (*callback_info)(uint8_t info_id, uint8_t infomessage[7]) = empty_callback_info, void (*callback_state)(uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_solved) = empty_callback_state);

bool loopInfo(obus_can::message* message, int (*info_generator)(uint8_t*));

void strike();

void solve();

bool is_active();

}

#endif /* end of include guard: OBUS_MODULE_H */
