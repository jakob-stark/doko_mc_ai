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

/**
 * @brief Samples random float numbers in [0,1) interval
 *
 * @param state the random engine state to advance
 * @return Random number (float) in [0,1) interval
 */
float random_float(uint32_t* state);

/**
 * @brief Samples a random integer number in [0,a) interval
 *
 * @param state the random engine state to advance
 * @param a upper edge of sample interval
 * @return Random integer number between 0 (inclusive) and a (exclusive)
 */
uint8_t random_uint8(uint32_t* state, uint8_t a);

/**
 * @brief Samples a random 32bit value
 *
 * @param state the random engine state to advance
 * @return a random 32 value
 */
uint32_t random_bits(uint32_t* state);

/**@}*/

#endif
