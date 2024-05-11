#include "ismcts.h"
#include "mc.h"
#include <stdio.h>

/* tests for the mc module */

void print_card_info(doko_card_info_t const* card_info) {
    uint8_t p, c;

    printf("cards_left      \t= %u\n", card_info->cards_left);

    printf("player_left[] =\n { ");
    for (p = 0; p < 3; p++) {
        printf("%u ", card_info->player_left[p]);
    }
    printf("}\n");

    printf("ids[] =\n { ");
    for (c = 0; c < card_info->cards_left; c++) {
        printf("%u ", card_info->ids[c]);
    }
    printf("}\n");

    printf("likeliness[] =\n { ");
    for (c = 0; c < card_info->cards_left; c++) {
        printf("{ ");
        for (p = 0; p < 3; p++) {
            printf("%u ", card_info->likeliness[c][p]);
        }
        printf("} ");
    }
    printf("}\n");
}

void print_game_info(doko_game_info_t const* game_info) {
    uint8_t p;

    printf("player_cardsets[] =\n { ");
    for (p = 0; p < 4; p++) {
        printf("0x%lx ", game_info->player_cardsets[p]);
    }
    printf("}\n");
}

doko_game_info_t const null_game = {
    .player_cardsets = {0ul, 0ul, 0ul, 0ul},
    .player_scores = {0, 0, 0, 0},
    .player_isre = {false, false, false, false},
    .cards_left = 0,
    .next = 0,
    .trickscore = 0,
    .tricksuit = 0,
    .trickwinnercard = 0,
    .trickwinner = 0,
};

doko_game_info_t const last_trick_gi = {
    .player_cardsets = {DOKO_CARDSHIFT(SPADE_JACK_L), 0ul, 0ul, 0ul},
    .player_scores = {0, 0, 0, 0},
    .player_isre = {true, true, false, false},
    .cards_left = 4,
    .next = 0,
    .trickscore = 0,
    .tricksuit = NOSUIT,
    .trickwinnercard = INVALID,
    .trickwinner = 0,
};

doko_card_info_t const last_trick_ci = {
    .cards_left = 3,
    .player_left = {1, 1, 1},
    .ids = {DIAMOND_JACK_L, HEART_JACK_L, CLUB_JACK_L},
    .likeliness = {{4, 4, 4}, {4, 4, 4}, {4, 4, 4}},
};

doko_card_info_t ci = {
    6,
    {2, 2, 2},
    {0, 1, 2, 3, 4, 5},
    {{0, 5, 5}, {5, 5, 5}, {5, 3, 5}, {5, 5, 5}, {4, 0, 5}, {5, 0, 0}},
};

doko_card_info_t ci1 = {
    6,
    {2, 2, 2},
    {0, 1, 2, 3, 4, 5},
    {{5, 5, 5}, {5, 5, 5}, {5, 5, 5}, {5, 5, 5}, {5, 5, 5}, {5, 5, 5}},
};

int main(void) {
    doko_random_state_t rs = 1;

    printf("last trick game:\n");
    doko_game_info_t gi = last_trick_gi;
    doko_card_info_t ci = last_trick_ci;

    doko_sort_and_check(&ci);
    doko_tree_node_t* tree = doko_ismcts_tree_new();

    for (int i = 0; i < 1000; i++) {
        doko_game_info_t gic = gi;
        doko_mc_sample(&gic, &ci, &rs);
        doko_ismcts_run(tree, &gic, &rs);
    }

    float max_score = 0.0f;
    doko_card_t max_card = INVALID;

    auto nnodes = doko_ismcts_tree_check(tree);
    max_card = doko_ismcts_get_best_card(tree, &max_score);

    doko_ismcts_tree_free(tree);

    printf("chosen %s with a score of %f\n", doko_card_names[max_card / 2],
           max_score);
}
