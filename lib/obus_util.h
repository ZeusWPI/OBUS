#ifndef OBUS_UTIL_H
#define OBUS_UTIL_H

#include "Arduino.h"


uint32_t unpack_4b_into_u32(uint8_t *data);
void pack_u32_into_4b(uint8_t *dest, uint32_t data);

uint16_t unpack_2b_into_u16(uint8_t *data);
void pack_u16_into_2b(uint8_t *dest, uint16_t data);


#endif /* end of include guard: OBUS_UTIL_H */
