#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "mc.h"
#include "simulate.h"

/**
 * @defgroup analysis analysis
 *
 * Detailed description
 *
 * @todo add more documentation
 *
 * @{
 */

/**
 * @brief Structure to hold all the input information, that can be passed to
 *        the program
 */
typedef struct {
    uint8_t computer_player_id;      /**< which of the players is the one we are
                                        going to simulate */
    uint8_t starting_player_id;      /**< which of the players began the game */
    CardId played_cards[48];         /**< all the played cards in order */
    uint8_t played_cards_len;        /**< length of the played cards array */
    CardId computer_player_hand[12]; /**< current hand cards of the computer
                                        player */
    uint8_t
        computer_player_hand_len; /**< length of the current hand cards array */
} InputInfo;

/** @brief Analyzes the input
 *
 *  This function is used to convert the input into the internal data
 *  structures of type GameInfo and CardInfo. If the input describes a valid
 *  card distribution, the function fills the game_dest and card_info
 *  structures with appropriate data to be passed to the core functions. If the
 *  input describes an invalid game history (e.g. invalid card distribution or
 *  errorneous play of one of the players) the function returns a nonzero
 *  value.
 *
 *  @param input Pointer to input, that is to be analyzed.
 *  @param[out] game_info This structure is filled with GameInfo data.
 *  @param[out] card_info This structure is filled with CardInfo data.
 *  @return 0 on success, a nonzero value if the input is invalid.
 */
int analyze(InputInfo const* input, GameInfo* game_info, CardInfo* card_info);

/**@}*/

#endif
