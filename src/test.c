#include "mc.h"
#include <stdio.h>

/* tests for the mc module */

void print_card_info(CardInfo const* card_info) {
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

    printf("scores[] =\n { ");
    for (c = 0; c < card_info->cards_left; c++) {
        printf("{ ");
        for (p = 0; p < 3; p++) {
            printf("%u ", card_info->scores[c][p]);
        }
        printf("} ");
    }
    printf("}\n");
}

void print_game_info(GameInfo const* game_info) {
    uint8_t p;

    printf("player_cardsets[] =\n { ");
    for (p = 0; p < 4; p++) {
        printf("0x%lx ", game_info->player_cardsets[p]);
    }
    printf("}\n");
}

GameInfo gi = {{0ul, 0ul, 0ul, 0ul}, {0}, {0}, 0, 0, 0, 0, 0, 0};

CardInfo ci = {
    6,
    {2, 2, 2},
    {0, 1, 2, 3, 4, 5},
    {{0, 5, 5}, {5, 5, 5}, {5, 3, 5}, {5, 5, 5}, {4, 0, 5}, {5, 0, 0}}};

CardInfo ci1 = {
    6,
    {2, 2, 2},
    {0, 1, 2, 3, 4, 5},
    {{5, 5, 5}, {5, 5, 5}, {5, 5, 5}, {5, 5, 5}, {5, 5, 5}, {5, 5, 5}}};

uint32_t rs = 1;

int main(void) {
    print_card_info(&ci);
    printf("sorting...\n");
    sort_and_check(&ci);
    print_card_info(&ci);
    printf("\n");
    printf("0x%x\n", rs);
    mc_sample(&gi, &ci, &rs);
    printf("\n");
    printf("0x%x\n", rs);
    mc_sample(&gi, &ci, &rs);
    printf("\n");
    printf("0x%x\n", rs);
    mc_sample(&gi, &ci, &rs);
}
