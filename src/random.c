#include "random.h"

#include <assert.h>
#include <stdint.h>

/**
 * @addtogroup random
 *
 * Both functions are based on the same xorshift prg by George Marsaglia.  A
 * detailed description may be found at
 * [Wikipedia](https://en.wikipedia.org/wiki/Xorshift). Here, the basic 32bit
 * variant is used, which should suffice for our mc sampling.
 */

static uint32_t random(uint32_t* state) {
    uint32_t i = *state;
    i ^= i << 13;
    i ^= i >> 17;
    i ^= i << 5;
    return *state = i;
}

float random_float(uint32_t* state) {
    union {
        uint32_t i;
        float f;
    } u;
    u.i = random(state);

    /**
     * @note
     * **Implementation detail**:
     * the following transforms the 32bit random number in a IEEE754 single
     * precision floating point number between 1 and 2 by setting setting
     * the sign bit (msb) to zero and the exponent (8 bits following the
     * sign bit)
     *
     *     seee eeee emmm mmmm mmmm mmmm mmmm mmmm
     *     |\--------/\--------------------------/
     *     0   127         randomly filled
     *
     * the floating point number is
     *
     *     (-1)^s * 2^((e)_2 - 127) * (1.m)_2
     *      = (-1)^0 * 2^0 * (1.m)_2
     *      = (1.m)_2
     */
    u.i = (u.i & 0x007fffff) | 0x3f800000;
    return u.f - 1.0f;
}

uint8_t random_uint8(uint32_t* state, uint8_t a) {
    uint32_t i = random(state);
    return i % a;
}

uint32_t random_bits(uint32_t* state) {
    return random(state);
}
