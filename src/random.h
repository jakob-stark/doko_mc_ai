#ifndef RANDOM_H
#define RANDOM_H

/** @brief Samples random float numbers in [0,1) interval
 *
 *  @param a upper edge of sample interval
 *  @return Random number (float) in [0,1) interval
 */
float RandomF(uint32_t* state);

/** @brief Samples a random integer number in [0,a) interval
 *
 *  @param state the random engine state to advance
 *  @param a upper edge of sample interval
 *  @return Random integer number between 0 (inclusive) and a (exclusive)
 */
uint8_t RandomC(uint32_t* state, uint8_t a);

#endif
