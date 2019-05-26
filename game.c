#include <stdlib.h>
#include "game.h"

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

/** @brief Simulates a random game
 *
 * @param game_info pointer to GameInfo struct to operate on. game_info will not be preserved!
 * @return Score value for Re.
 */
Score Simulate( GameInfo* game_info ) {
	CardSet legal_card_set;
	CardId legal_cards[12];
	uint8_t legal_cards_len;
	uint8_t legal_card_id;
	while ( game_info->cards_left > 0 ) {
		/* determine legal cards to play */
		legal_card_set = game_info->player_cardsets[game_info->next] & suit_sets[game_info->tricksuit];
		if ( legal_card_set == 0 ) {
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
		/* determine random legal card to play and play it */
		PlayCard( game_info, legal_cards[rand() % legal_cards_len] );
	}
	/* return score for re */
	PlayerId p;
	Score result = 0;
	for ( p = 0; p < 4; p++ ) {
		if ( GET_RE(game_info->player_cardsets[p]) ) {
			result += game_info->player_scores[p];
		}
	}
	return result;
}

