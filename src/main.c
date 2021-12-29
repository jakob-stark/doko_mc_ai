#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "simulate.h"
#include "mc.h"
#include "core.h"

typedef struct {
    CardId hand[12];
    uint8_t hand_len;
    PlayerId start;
    CardId played[48];
    uint8_t played_len;
} Input;

typedef struct {
    CardId hand[12];
    Score scores[12];
    CardId best_card;
} Output;

int Analyze(const Input * inp, Output * out) {
    GameInfo game_info;
    CardInfo card_info;

    for ( uint8_t p = 0; p < 4; p++ ) {
        game_info.player_cardsets[p] = 0ul;
        game_info.player_scores[p] = 0;
        game_info.player_isre[p] = false;
    }
    game_info.cards_left = 48;
    game_info.next = inp->start;
    game_info.trickscore = 0;
    game_info.tricksuit = NOSUIT;

    for ( uint8_t p = 0; p < 3; p++ ) {
        card_info.player_left[p] = 12;
    }

    /* play each card */
    CardSet cards_left = 0x0000fffffffffffful;
    for ( uint8_t i = 0; i < inp->played_len; i++ ) {
        if ( CARDSHIFT(inp->played[i]) & cards_left ) {
            cards_left &= ~CARDSHIFT(inp->played[i]);
            if ( game_info.next != 0 ) {
                card_info.player_left[game_info.next-1]--;
            }
            PlayCard(&game_info,  inp->played[i]);
        } else {
            return -1;
        }
    }

    /* add hand cards to player0 card set */
    for ( uint8_t i = 0; i < inp->hand_len; i++ ) {
        if ( CARDSHIFT(inp->hand[i]) & cards_left ) {
            cards_left &= ~CARDSHIFT(inp->hand[i]);
            game_info.player_cardsets[0] |= CARDSHIFT(inp->hand[i]);
        } else {
            return -1;
        }
    }

    /* fill in left cards into card_info */
    uint8_t card_id = 0;
    while ( cards_left != 0 ) {
        if ( cards_left & 1 ) {
            for ( uint8_t p = 0; p < 3; p++ ) {
                card_info.scores[card_info.cards_left][p] = 5;
            }
            card_info.ids[card_info.cards_left++] = card_id;
        }
        card_id++;
        cards_left >>= 1;
    }

    /* check if different numbers of cards match */
    if ( card_info.cards_left != card_info.player_left[0] + 
                    card_info.player_left[1] + card_info.player_left[2] ) {
        return -1;
    }

    if ( card_info.cards_left + inp->hand_len + inp->played_len != 48 ) {
        return -1;
    }

    if ( card_info.cards_left != game_info.cards_left ) {
        return -1;
    }

    out->best_card = GetBestCard(&game_info, &card_info);
    return 0;
}

int main(void) {
	GameInfo g;
	CardInfo c;
	return 0;
}
