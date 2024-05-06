#include "analysis.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

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

int analysis_start(GameInfo* game_info, CardInfo* card_info,
                   PlayerId computer_player_id, PlayerId starting_player_id,
                   CardId const cards[12]) {
    /* check input - we use asserts here, as values out of range are not a game
     * play issue, but a bug in the calling code */
    assert(game_info != nullptr);
    assert(card_info != nullptr);
    assert(computer_player_id < 4);
    assert(starting_player_id < 4);
    for (uint8_t i = 0; i < 12; i++) {
        assert(CARD_VALID(cards[i]));
        assert(cards[i] % 2 == 0);
    }

    /* initialize game info */
    for (uint8_t p = 0; p < 4; p++) {
        game_info->player_cardsets[p] = 0ul;
        game_info->player_scores[p] = 0;
        game_info->player_isre[p] = false;
    }
    game_info->cards_left = 48;
    game_info->next = (4 + starting_player_id - computer_player_id) % 4;
    game_info->trickscore = 0;
    game_info->tricksuit = NOSUIT;

    /* add starting cards to computer player */
    CardSet cards_left = 0x0000fffffffffffful;
    for (uint8_t i = 0; i < 12; i++) {
        CardSet card_l = CARDSHIFT(cards[i]);
        CardSet card_h = CARDSHIFT(cards[i] + 1);
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
    for (CardId card_id = 0; cards_left != 0; card_id++, cards_left >>= 1) {
        assert(card_info->cards_left < 36);
        if (cards_left & 1) {
            for (uint8_t p = 0; p < 3; p++) {
                card_info->scores[card_info->cards_left][p] = 5;
            }
            card_info->ids[card_info->cards_left++] = card_id;
        }
    }

    assert(card_info->cards_left == 36);

    return 0;
}

int analysis_move(GameInfo* game_info, CardInfo* card_info,
                  PlayerId computer_player_id, PlayerId player_id,
                  CardId card) {
    /* check input - we use asserts here, as values out of range are not a game
     * play issue, but a bug in the calling code */
    assert(game_info != nullptr);
    assert(card_info != nullptr);
    assert(computer_player_id < 4);
    assert(player_id < 4);
    assert(CARD_VALID(card));
    assert(card % 2 == 0);

    PlayerId player = (4 + player_id - computer_player_id) % 4;

    /* check if the player if the actual next player */
    if (player != game_info->next) {
        /* not that players turn */
        return -1;
    }

    if (player == 0) {
        /* check if the card if still available and if yes if high or low */
        if (CARDSHIFT(card) & game_info->player_cardsets[0]) {
            PlayCard(game_info, card);
            return 0;
        }
        if (CARDSHIFT(card + 1) & game_info->player_cardsets[0]) {
            PlayCard(game_info, card + 1);
            return 0;
        }
        // card not in hand
    } else {
        /* check if the card if still available and if yes if high or low */
        for (uint8_t card_index = 0; card_index < card_info->cards_left;
             card_index++) {
            if (CARD_EQUAL(card_info->ids[card_index], card)) {
                PlayCard(game_info, card_info->ids[card_index]);

                /* remove the card from cardinfo */
                card_info->player_left[player - 1]--;
                card_info->cards_left--;

                card_info->ids[card_index] =
                    card_info->ids[card_info->cards_left];
                memcpy(card_info->scores[card_index],
                       card_info->scores[card_info->cards_left],
                       sizeof(card_info->scores[0]));

                return 0;
            }
        }
        // card not found
    }
    return -1;
}
