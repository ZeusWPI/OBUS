#ifndef OBUS_UTIL_H
#define OBUS_UTIL_H

#include "Arduino.h"


uint32_t unpack_4b_into_u32(uint8_t *data);
void pack_u32_into_4b(uint8_t *dest, uint32_t data);

uint16_t unpack_2b_into_u16(uint8_t *data);
void pack_u16_into_2b(uint8_t *dest, uint16_t data);


class Debounced {
	private:
		uint16_t press_count;
		bool pressing;
		uint32_t press_last_detected;
		uint32_t debounce;
	public:
		Debounced(uint32_t debounce_time) :
			press_count(0),
			pressing(false),
			press_last_detected(0),
			debounce(debounce_time)
			{};
		Debounced() : Debounced(50) {};

		bool loop(bool currently_down);
		uint16_t get_press_count();
		void reset_press_count();
};

#endif /* end of include guard: OBUS_UTIL_H */
