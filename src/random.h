#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

/**
 * @defgroup random random
 *
 * A random number generator module
 *
 * @{
 */

typedef uint32_t doko_random_state_t;
typedef uint32_t doko_random_bits_t;

/**
 * @brief Samples random float numbers in [0,1) interval
 *
 * @param state the random engine state to advance
 * @return Random number (float) in [0,1) interval
 */
float doko_random_float(doko_random_state_t* state);

/**
 * @brief Samples a random integer number in [0,a) interval
 *
 * @param state the random engine state to advance
 * @param a upper edge of sample interval
 * @return Random integer number between 0 (inclusive) and a (exclusive)
 */
uint8_t doko_random_uint8(doko_random_state_t* state, uint8_t a);

/**
 * @brief Samples a random 32bit value
 *
 * @param state the random engine state to advance
 * @return a random 32 value
 */
doko_random_bits_t doko_random_bits(doko_random_state_t* state);

/**@}*/

#endif
