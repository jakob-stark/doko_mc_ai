#include <stdlib.h>
#include <string.h>
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
		if ( game_info->player_isre[p] ) {
			result += game_info->player_scores[p];
		}
	}
	return result;
}

float Metric(float x) {
	return x;
//	return 6*x*(1-x);
}

float Random() {
	static union {
		uint32_t i;
		float f;
	} u;
	u.i = rand() & 0x007fffff | 0x3f800000;
	return u.f-1.0;
}

void MCSample( GameInfo* dest, CardInfo* card_info ) {
	/* sample the cards */
	CardId c;
	int8_t i;
	PlayerId p;
	float r;
	float m_sum[3];
	while ( card_info->cards_left > 0 ) {
		c = --(card_info->cards_left);
		r = Random() * (card_info->scores[0][c] + card_info->scores[1][c] + card_info->scores[2][c]);
		if ( r < card_info->scores[0][c] ) {
			p = 0;
		} else if ( r < card_info->scores[0][c] + card_info->scores[1][c] ) {
			p = 1;
		} else {
			p = 2;
		}
		
		/* norm scores */
		m_sum[p] = card_info->metric_sum[p] - Metric( card_info->scores[p][c] );
		if ( m_sum[p] != 0 ) {
			card_info->metric_sum[p] = 0;
			for ( i = c - 1; i >= 0; i-- ) {
				card_info->scores[p][i] -= (1.0-card_info->scores[p][c]) * Metric( card_info->scores[p][i] )
												/ m_sum[p];
				card_info->metric_sum[p] += Metric( card_info->scores[p][i] );
			}
		}
		p = (p+1)%3;
		m_sum[p] = card_info->metric_sum[p] - Metric( card_info->scores[p][c] );
		if ( m_sum[p] != 0 ) {
			card_info->metric_sum[p] = 0;
			for ( i = c - 1; i >= 0; i-- ) {
				card_info->scores[p][i] += card_info->scores[p][c] * Metric( card_info->scores[p][i] )
												/ m_sum[p];
				card_info->metric_sum[p] += Metric( card_info->scores[p][i] );
			}
		}
		p = (p+1)%3;
		m_sum[p] = card_info->metric_sum[p] - Metric( card_info->scores[p][c] );
		if ( m_sum[p] != 0 ) {
			card_info->metric_sum[p] = 0;
			for ( i = c - 1; i >= 0; i-- ) {
				card_info->scores[p][i] += card_info->scores[p][c] * Metric( card_info->scores[p][i] )
												/ m_sum[p];
				card_info->metric_sum[p] += Metric( card_info->scores[p][i] );
			}
		}
		p = (p+1)%3;

		/* add card to player */
		dest->player_cardsets[p+1] += CARDSHIFT(card_info->ids[c]);
	}
}



