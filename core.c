#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <pthread.h>
#include <sys/sysinfo.h>

#include "core.h"
#include "random.h"

const char * const card_names[24] = { 
	"cn", "ck", "ct", "ca", "sn", "sk", "st", "sa", "hn", "hk", "ha",
	"dn", "dk", "dt", "da", "dj", "hj", "sj", "cj", "dq", "hq", "sq", "cq", "ht"
};

const char * const card_names_long[24] = {
    "club nine", "club king", "club ten", "club ace",
    "spade nine", "spade king", "spade ten", "spade ace",
    "heard nine", "heart king", "heart ace",
    "diamond nine", "diamond king", "diamond ten", "diamond ace",
    "diamond jack", "heart jack", "spade jack", "club jack",
    "diamond queen", "heart queen", "spade queen", "club queen",
    "heart ten"
};

static const Score card_values[24] = {
	0, 4, 10, 11, 0, 4, 10, 11, 0, 4, 11, 0, 4, 10, 11,	2, 2, 2, 2, 3, 3, 3, 3, 10
};

static const Suit card_suits[24] = {
	CLUB, CLUB, CLUB, CLUB, SPADE, SPADE, SPADE, SPADE, HEART, HEART, HEART,
	TRUMP, TRUMP, TRUMP, TRUMP, TRUMP, TRUMP, TRUMP, TRUMP, TRUMP, TRUMP, TRUMP, TRUMP, TRUMP
};

static const CardSet suit_sets[6] = {
	0x00000000000000fful,
	0x000000000000ff00ul,
	0x00000000003f0000ul,
	0x0000000000000000ul,
	0x0000ffffffc00000ul,
	0x0000fffffffffffful
};

/** @brief Simulates a random game
 *
 * @param game_info pointer to GameInfo struct to simulate
 * @param random_state pointer to 32bit random state used by Random and RandomInt
 * @return Score value for player 0
 */
SimulationResult RandomSimulate( const GameInfo* game_info_, uint32_t* random_state ) {
    GameInfo game_info;
    SimulationResult result = {0,0};
	CardId legal_cards[12];
    CardId played_cards[48];
    CardId next_card;
	uint8_t legal_cards_len;
    /* get a copy of the game_info */
    game_info = *game_info_;
	while ( game_info.cards_left > 0 ) {
		/* determine legal cards to play */
        legal_cards_len = GetLegalCards(&game_info, legal_cards);
		/* determine random legal card to play, store it and play it */
        next_card = legal_cards[RandomC(random_state, legal_cards_len)];
        played_cards[game_info.cards_left-1] = next_card;
		PlayCard(&game_info, next_card);
	}
	/* return score for party of player 0 */
	PlayerId p;
	for ( p = 0; p < 4; p++ ) {
		if ( game_info.player_isre[p] == game_info.player_isre[0] ) {
			result.player_score += game_info.player_scores[p];
		}
	}
    result.first_played_card = played_cards[game_info_->cards_left-1];
	return result;
}


/** @brief get legal cards for next player
 *
 *  @param game_info pointer to the GameInfo object to get the legal cards from
 *  @param legal_cards pointer to array where the result is stored
 *  @return number of legal cards found and stored in legal_cards
 */
uint8_t GetLegalCards( const GameInfo* game_info, CardId legal_cards[12] ) {
    CardSet legal_card_set;
    uint8_t legal_cards_len;
    uint8_t legal_card_id;
    /* determine legal cards to play */
    legal_card_set = game_info->player_cardsets[game_info->next] & suit_sets[game_info->tricksuit];
    if ( legal_card_set == 0 ) {
        /* if no card is legal, the player may choose freely which card to play */
        legal_card_set = game_info->player_cardsets[game_info->next];
    }
    legal_cards_len = 0;
    legal_card_id = 0;
    while ( legal_card_set != 0 ) {
        switch ( legal_card_set % 4 ) {
            case 2:
                legal_cards[legal_cards_len++] = legal_card_id;
            case 1:
                legal_cards[legal_cards_len++] = legal_card_id;
        }
        legal_card_id++;
        legal_card_set >>= 2;
    }
    return legal_cards_len;
}
    

/** @brief performs a card play
 *
 *  @param game_info pointer to GameInfo struct to operate on. Will not be preserved!
 *  @param card Id of card to play. Will not check for consistency. If the next player
 *  			does not hold this card, the behaviour is undefined.
 */
void PlayCard( GameInfo* game_info, CardId card ) {
	/* remove card from players hand */
	game_info->player_cardsets[game_info->next] -= CARDSHIFT(card);
	--(game_info->cards_left);

	/* add value to score */
	game_info->trickscore += card_values[card];

	if ( game_info->tricksuit == NOSUIT ) {
		/* no card on trick */
		game_info->tricksuit = card_suits[card];
		game_info->trickwinnercard = card;
		game_info->trickwinner = game_info->next;
	} else {
		/* test if new card is winning */
		if ( card_suits[card] == game_info->tricksuit || card_suits[card] == TRUMP ) {
			if ( card > game_info->trickwinnercard || ( card == HEART_TEN && game_info->cards_left > 4 ) ) {
				game_info->trickwinnercard = card;
				game_info->trickwinner = game_info->next;
			}
		}
	}

	/* test if trick is full */
	if ( game_info->cards_left % 4 == 0 ) {
		game_info->player_scores[game_info->trickwinner] += game_info->trickscore;
		game_info->trickscore = 0;
		game_info->tricksuit = NOSUIT;
		game_info->next = game_info->trickwinner;
	} else {
		game_info->next = (game_info->next + 1) % 4;
	}
}

