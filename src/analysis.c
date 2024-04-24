#include "analysis.h"
#include <stdio.h>

int analyze(InputInfo const* input, GameInfo* game_info, CardInfo* card_info) {
    /* check input */
    if (input->starting_player_id >= 4 || input->computer_player_id >= 4) {
        printf("%d %d\n", input->starting_player_id, input->computer_player_id);
        return -1;
    }
    if (input->played_cards_len > 48) {
        return -1;
    }
    for (uint8_t i = 0; i < input->played_cards_len; i++) {
        if (!CARD_VALID(input->played_cards[i])) {
            return -1;
        }
    }
    if (input->computer_player_hand_len > 12) {
        return -1;
    }
    for (uint8_t i = 0; i < input->computer_player_hand_len; i++) {
        if (!CARD_VALID(input->computer_player_hand[i])) {
            return -1;
        }
    }

    /* initialize game and card info structs */
    for (uint8_t p = 0; p < 4; p++) {
        game_info->player_cardsets[p] = 0ul;
        game_info->player_scores[p] = 0;
        game_info->player_isre[p] = false;
    }
    game_info->cards_left = 48;
    game_info->next =
        (4 + input->starting_player_id - input->computer_player_id) % 4;
    game_info->trickscore = 0;
    game_info->tricksuit = NOSUIT;

    for (uint8_t p = 0; p < 3; p++) {
        card_info->player_left[p] = 12;
    }
    card_info->cards_left = 0;

    /* play each card */
    CardSet cards_left = 0x0000fffffffffffful;
    for (uint8_t i = 0; i < input->played_cards_len; i++) {
        CardId c = input->played_cards[i];
        if (CARDSHIFT(c) & cards_left) {
            cards_left &= ~CARDSHIFT(c);
            if (game_info->next != 0) {
                card_info->player_left[game_info->next - 1]--;
            }
            PlayCard(game_info, c);
        } else {
            return -1;
        }
    }

    /* add hand cards to player0 card set */
    for (uint8_t i = 0; i < input->computer_player_hand_len; i++) {
        CardId c = input->computer_player_hand[i];
        if (CARDSHIFT(c) & cards_left) {
            cards_left &= ~CARDSHIFT(c);
            game_info->player_cardsets[0] |= CARDSHIFT(c);
        } else {
            return -1;
        }
    }

    /* fill in left cards into card_info */
    uint8_t card_id = 0;
    while (cards_left != 0) {
        if (cards_left & 1) {
            for (uint8_t p = 0; p < 3; p++) {
                card_info->scores[card_info->cards_left][p] = 5;
            }
            card_info->ids[card_info->cards_left++] = card_id;
        }
        card_id++;
        cards_left >>= 1;
    }

    /* check if different numbers of cards match */
    if (card_info->cards_left != card_info->player_left[0] +
                                     card_info->player_left[1] +
                                     card_info->player_left[2]) {
        return -1;
    }

    if (card_info->cards_left + input->computer_player_hand_len +
            input->played_cards_len !=
        48) {
        return -1;
    }

    if (card_info->cards_left + input->computer_player_hand_len !=
        game_info->cards_left) {
        return -1;
    }

    if (game_info->next != 0) {
        return -1;
    }

    return 0;
}
