#ifndef AB_H
#define AB_H

#include "simulate.h"

doko_score_t AlphaBetaEvaluate(doko_game_info_t const* game_info_in,
                               doko_card_t next_card, doko_score_t alpha,
                               doko_score_t beta);

#endif // AB_H
