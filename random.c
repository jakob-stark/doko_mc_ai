#include <stdint.h>
#include "random.h"

float RandomF(uint32_t* state) {
	union {
		uint32_t i;
		float f;
	} u;
	u.i = *state;
	u.i ^= u.i << 13;
	u.i ^= u.i >> 17;
	u.i ^= u.i << 5;
	*state = u.i;
	u.i = u.i & 0x007fffff | 0x3f800000;
	return u.f-1.0;
}

uint8_t RandomC(uint32_t* state, uint8_t a) {
	uint32_t i;
	i = *state;
	i ^= i << 13;
	i ^= i >> 17;
	i ^= i << 5;
	*state = i;
	return i % a;
}

