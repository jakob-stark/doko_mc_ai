#ifndef AB_H
#define AB_H

#include "simulate.h"

Score AlphaBetaEvaluate(GameInfo const* game_info_in, CardId next_card,
                        Score alpha, Score beta);

#endif // AB_H
