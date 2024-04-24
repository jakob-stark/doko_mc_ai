#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "simulate.h"
#include "mc.h"
#include "core.h"
#include "ab.h"

GameInfo t_game_info_1 = {
    .player_cardsets = {
        CARDSHIFT(HEART_QUEEN_L) | CARDSHIFT(CLUB_JACK_L),
        CARDSHIFT(SPADE_TEN_L) | CARDSHIFT(SPADE_TEN_H),
        CARDSHIFT(CLUB_TEN_L)  | CARDSHIFT(CLUB_TEN_H) ,
        CARDSHIFT(DIAMOND_QUEEN_H) | CARDSHIFT(CLUB_JACK_H)
    },
    .player_scores   = {0,0,0,0},
    .player_isre     = {true, true, false, false},
    .cards_left      = 8,
    .next            = 0,
    .trickscore      = 0,
    .tricksuit       = NOSUIT,
    .trickwinnercard = 0,
    .trickwinner     = 0
};

GameInfo t_game_info_2 = {
    .player_cardsets = {
        0x0000e001f000f000ul,
        0x00000f000f000f00ul,
        0x000000f000f000f0ul,
        0x0000100e000f000ful,
    },
    .player_scores   = {0,0,0,0},
    .player_isre     = {true, false, false, true},
    .cards_left      = 48,
    .next            = 0,
    .trickscore      = 0,
    .tricksuit       = NOSUIT,
    .trickwinnercard = 0,
    .trickwinner     = 0
};

void print_table_usage(void);

int main(void) {
    GameInfo game_info = t_game_info_2;

    Score score = AlphaBetaEvaluate(&game_info, CLUB_QUEEN_H, 0, 240);
    printf("score %hhu\n", score);
    print_table_usage();

    return 0;
}
