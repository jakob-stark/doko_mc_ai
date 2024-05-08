#include "analysis.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int doko_analysis_start(doko_game_info_t* game_info,
                        doko_card_info_t* card_info,
                        doko_player_t computer_player_id,
                        doko_player_t starting_player_id,
                        doko_card_t const cards[12]) {
    /* check input - we use asserts here, as values out of range are not a game
     * play issue, but a bug in the calling code */
    assert(game_info != nullptr);
    assert(card_info != nullptr);
    assert(computer_player_id < 4);
    assert(starting_player_id < 4);
    for (doko_count_t i = 0; i < 12; i++) {
        assert(DOKO_CARD_VALID(cards[i]));
        assert(cards[i] % 2 == 0);
    }

    /* initialize game info */
    DOKO_FOR_EACH_PLAYER(p) {
        game_info->player_cardsets[p] = 0ul;
        game_info->player_scores[p] = 0;
        game_info->player_isre[p] = false;
    }
    game_info->cards_left = 48;
    game_info->next = (4 + starting_player_id - computer_player_id) % 4;
    game_info->trickscore = 0;
    game_info->tricksuit = NOSUIT;

    /* add starting cards to computer player */
    doko_cardset_t cards_left = 0x0000fffffffffffful;
    for (doko_count_t i = 0; i < 12; i++) {
        doko_cardset_t card_l = DOKO_CARDSHIFT(cards[i]);
        doko_cardset_t card_h = DOKO_CARDSHIFT(cards[i] + 1);
        if (card_l & cards_left) {
            cards_left &= ~card_l;
            game_info->player_cardsets[0] |= card_l;
        } else if (card_h & cards_left) {
            cards_left &= ~card_h;
            game_info->player_cardsets[0] |= card_h;
        } else {
            // card too often
            return -1;
        }
    }

    /* initialize card info */
    for (uint8_t p = 0; p < 3; p++) {
        card_info->player_left[p] = 12;
    }

    /* fill in the remaining cards into the card info struct */
    for (doko_card_t card_id = 0; cards_left != 0;
         card_id++, cards_left >>= 1) {
        if (cards_left & 1) {
            for (uint8_t p = 0; p < 3; p++) {
                card_info->likeliness[card_info->cards_left][p] = DUNNO;
            }
            card_info->ids[card_info->cards_left++] = card_id;
        }
    }

    assert(card_info->cards_left == 36);

    return 0;
}

int doko_analysis_move(doko_game_info_t* game_info, doko_card_info_t* card_info,
                       doko_player_t computer_player_id,
                       doko_player_t player_id, doko_card_t card) {
    /* check input - we use asserts here, as values out of range are not a game
     * play issue, but a bug in the calling code */
    assert(game_info != nullptr);
    assert(card_info != nullptr);
    assert(computer_player_id < 4);
    assert(player_id < 4);
    assert(DOKO_CARD_VALID(card));
    assert(card % 2 == 0);

    doko_player_t player = (4 + player_id - computer_player_id) % 4;

    /* check if the player if the actual next player */
    if (player != game_info->next) {
        /* not that players turn */
        fprintf(stderr, "wrong players turn\n");
        return -1;
    }

    if (player == 0) {
        /* check if the card if still available and if yes if high or low */
        if (DOKO_CARDSHIFT(card) & game_info->player_cardsets[0]) {
            doko_play_card(game_info, card);
            return 0;
        }
        if (DOKO_CARDSHIFT(card + 1) & game_info->player_cardsets[0]) {
            doko_play_card(game_info, card + 1);
            return 0;
        }
        fprintf(stderr, "card not in my hand: %s\n",
                doko_card_names_long[card]);
        // card not in hand
    } else {
        /* check if the card if still available and if yes if high or low */
        for (uint8_t card_index = 0; card_index < card_info->cards_left;
             card_index++) {
            if (DOKO_CARD_EQUAL(card_info->ids[card_index], card)) {
                doko_play_card(game_info, card_info->ids[card_index]);

                /* remove the card from cardinfo */
                card_info->player_left[player - 1]--;
                card_info->cards_left--;

                card_info->ids[card_index] =
                    card_info->ids[card_info->cards_left];
                memcpy(card_info->likeliness[card_index],
                       card_info->likeliness[card_info->cards_left],
                       sizeof(card_info->likeliness[0]));

                return 0;
            }
        }
        fprintf(stderr, "card already played too often: %s\n",
                doko_card_names_long[card]);
    }
    return -1;
}

int doko_analysis_finish(doko_game_info_t const* game_info, int* points) {
    doko_score_t re_score = 0;
    uint8_t re_count = 0;
    DOKO_FOR_EACH_PLAYER(p) {
        if (game_info->player_isre[p]) {
            re_score += game_info->player_scores[p];
            re_count++;
        }
    }
    int re_points = +(re_score == 240)  // black
                    + (re_score >= 210) // no 30
                    + (re_score >= 180) // no 60
                    + (re_score >= 150) // no 90
                    + (re_score > 120)  // > won
                    - (re_score <= 120) // lost against contra
                    - (re_score < 120)  // < lost
                    - (re_score < 90)   // no 90
                    - (re_score < 60)   // no 60
                    - (re_score < 30)   // no 30
                    - (re_score == 0)   // black
        ;
    if (game_info->player_isre[0]) {
        if (re_count == 1) {
            *points = re_points * 3;
        } else {
            *points = re_points;
        }
    } else {
        *points = -re_points;
    }
    return 0;
}
