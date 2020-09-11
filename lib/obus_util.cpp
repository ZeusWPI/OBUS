#include "obus_util.h"


uint32_t unpack_4b_into_u32(uint8_t *data) {
	return
		((uint32_t) data[0] << 0x18) |
		((uint32_t) data[1] << 0x10) |
		((uint32_t) data[2] << 0x08) |
		((uint32_t) data[3]);
}

void pack_u32_into_4b(uint8_t *dest, uint32_t data) {
	dest[0] = (uint8_t) ((data & 0xFF000000) >> 0x18);
	dest[1] = (uint8_t) ((data & 0x00FF0000) >> 0x10);
	dest[2] = (uint8_t) ((data & 0x0000FF00) >> 0x08);
	dest[3] = (uint8_t)  (data & 0x000000FF);
}


uint16_t unpack_2b_into_u16(uint8_t *data) {
	return
		((uint16_t) data[0] << 0x08) |
		((uint16_t) data[1]);
}

void pack_u16_into_2b(uint8_t *dest, uint16_t data) {
	dest[0] = (uint8_t) ((data & 0xFF00) >> 0x08);
	dest[1] = (uint8_t)  (data & 0x00FF);
}
