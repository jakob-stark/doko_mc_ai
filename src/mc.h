#ifndef MC_H
#define MC_H

#include "simulate.h"

/** @defgroup mc mc
 *
 *   This monte carlo module contains functions to randomlu distribute the
 *   remaining cards to the other players
 * @{
 */

enum {
    IMPOSSIBLE = 0,
    UNLIKELY = 3,
    DUNNO = 4,
    LIKELY = 5,
};
typedef uint8_t doko_likeliness_t;

/**
 * @brief Structure to hold all information about unknown cards
 */
typedef struct {
    /* these are the facts */
    doko_count_t cards_left;     /**< total number of card left in the game */
    doko_count_t player_left[3]; /**< how many cards each player has left */

    doko_card_t ids[36];        /**< the ids of the cards left in the game */
    doko_likeliness_t likeliness[36][3]; /**< how likely is it that each player has each
                                   card. Values are 0,3,4 or 5 */
} doko_card_info_t;

/** @brief prepares the card_info structure
 *
 *  This function checks if card_info is valid and consitent. If it is, the
 *  cards are sorted ascending by their contraints, so that the mc sampler
 *  deals the cards with most contraints first.
 *
 *  @param card_info Structure to operate on
 *  @return 0 on sucesss, a nonzero value if the card_info struct is
 *      inconsistent
 */
int doko_sort_and_check(doko_card_info_t* card_info);

/** @brief Samples card distribution
 *
 *  This function samples a card distribution according to the information
 *  given in card_info. It doesn't preserve dest nor card_info, so make sure to
 *  copy them.
 *
 *  @param dest Destination GameInfo object. Will only add cards to the
 *      cardsets of players 1-3. So this can be used with an initialized
 *      GameInfo object where the remaining card will be filled in.
 *  @param card_info Information about which cards are to be dealed and with
 *      which scores.
 *  @param random_state pointer to 32bit value used as random state by the
 *      random generator
 */
void doko_mc_sample(doko_game_info_t* dest, doko_card_info_t const* card_info,
                    doko_random_state_t* random_state);

/**@}*/

#endif
