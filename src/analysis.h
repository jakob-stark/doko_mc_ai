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

int doko_analysis_start(doko_game_info_t* game_info,
                        doko_card_info_t* card_info,
                        doko_player_t computer_player_id,
                        doko_player_t starting_player_id,
                        doko_card_t const cards[12]);
int doko_analysis_move(doko_game_info_t* game_info, doko_card_info_t* card_info,
                       doko_player_t computer_player_id,
                       doko_player_t player_id, doko_card_t card);
int doko_analysis_finish(doko_game_info_t const* game_info, int* points);

/**@}*/

#endif
