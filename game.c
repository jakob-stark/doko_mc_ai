#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
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

/** @brief Samples random float numbers in [0,1) interval
 *
 * @return Random number (float) in [0,1) interval
 */
float Random() {
	static union {
		uint32_t i;
		float f;
	} u;
	u.i = rand() & 0x007fffff | 0x3f800000;
	return u.f-1.0;
}

/** @brief Samples card distribution
 *
 *  This function samples a card distribution according to the information given in card_info.
 *  It does'n preserve dest nor card_info, so make sure to copy them.
 *
 *  @param dest Destination GameInfo object. Will only add cards to the cardsets of players 1-3. So
 *  			this can be used with an initialized GameInfo object where the remaining card will be
 *  			filed in.
 *  @param card_info Information about which cards are to be dealed and with which scores.
 */
void MCSample( GameInfo* dest, CardInfo* card_info ) {
	/* sample the cards */
	CardId c;
	int8_t i;
	PlayerId p;
	float r;
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
		if ( card_info->sum[p] > card_info->scores[p][c] ) {
			for ( i = c - 1; i >= 0; i-- ) {
				card_info->scores[p][i] -= (1.0-card_info->scores[p][c]) * card_info->scores[p][i]
												/ (card_info->sum[p] - card_info->scores[p][c]);
			}
		}
		card_info->sum[p] -= 1.0;
		p = (p+1)%3;
		if ( card_info->sum[p] > card_info->scores[p][c] ) {
			for ( i = c - 1; i >= 0; i-- ) {
				card_info->scores[p][i] += card_info->scores[p][c] * card_info->scores[p][i]
												/ (card_info->sum[p] - card_info->scores[p][c]);
			}
		}
		p = (p+1)%3;
		if ( card_info->sum[p] > card_info->scores[p][c] ) {
			for ( i = c - 1; i >= 0; i-- ) {
				card_info->scores[p][i] += card_info->scores[p][c] * card_info->scores[p][i]
												/ (card_info->sum[p] - card_info->scores[p][c]);
			}
		}
		p = (p+1)%3;

		/* add card to player */
		dest->player_cardsets[p+1] += CARDSHIFT(card_info->ids[c]);
		if ( card_info->ids[c] = CLUB_QUEEN ) {
			dest->player_isre[p+1] = true;
		}
	}
}

void PrintCardInfo( CardInfo* card_info ) {
	
}

void Prepare( GameInfo* game_info, CardInfo* card_info ) {
	/* sort card so that cards with more zero scores appear at the end and will thus selected first */

	/* delete two zero cards and put them into cardsets */
	int8_t i;
	PlayerId p;
	float norms[3] = {0.0, 0.0, 0.0};
	float tmp;

	for ( i = card_info->cards_left - 1; i >= 0; i-- ) {
		for ( p = 0; p < 3; p++ ) {
			if ( card_info->scores[(p+1)%3][i] == 0.0 && card_info->scores[(p+2)%3][i] == 0.0 ) {
				game_info->player_cardsets[p+1] += CARDSHIFT(card_info->ids[i]);
				if ( card_info->ids[i] == CLUB_QUEEN ) {
					game_info->player_isre[p+1] = true;
				}
				norms[p] += 1.0-card_info->scores[p][i];
				card_info->sum[p] -= card_info->scores[p][i];

				memmove( &(card_info->scores[p][i]), &(card_info->scores[p][i+1]),
						sizeof(float)*(card_info->cards_left - i - 1) );
				memmove( &(card_info->scores[(p+1)%3][i]), &(card_info->scores[(p+1)%3][i+1]),
						sizeof(float)*(card_info->cards_left - i - 1) );
				memmove( &(card_info->scores[(p+2)%3][i]), &(card_info->scores[(p+2)%3][i+1]),
						sizeof(float)*(card_info->cards_left - i - 1) );
				memmove( &(card_info->ids[i]), &(card_info->ids[i+1]),
						sizeof(PlayerId)*(card_info->cards_left - i - 1) );

				(card_info->cards_left)--;
				if ( i < card_info->one ) {
					--(card_info->one);
				}
				break;
			}
		}
	}

	/* norm remaining cards */
	for ( p = 0; p < 3; p++ ) {
		for ( i = card_info->cards_left - 1; i >= 0; i-- ) {
			card_info->scores[p][i] -= norms[p] * card_info->scores[p][i] / card_info->sum[p];
		}
		card_info->sum[p] -= norms[p];
	}

	/* sort array */
	for ( i = card_info->one - 1; i >= 0; i-- ) {
		if ( card_info->scores[0][i] == 0.0 || card_info->scores[1][i] == 0.0 ||
					card_info->scores[2][i] == 0.0 ) {
			for ( p = 0; p < 3; p++ ) {
				tmp = card_info->scores[p][i];
				memmove( &(card_info->scores[p][i]), &(card_info->scores[p][i+1]), 
							sizeof(float)*(card_info->one - i - 1) );
				card_info->scores[p][card_info->one-1] = tmp;
			}
			PlayerId tmpid = card_info->ids[i];
			memmove( &(card_info->ids[i]), &(card_info->ids[i+1]), sizeof(PlayerId)*(card_info->one-i-1));
			card_info->ids[card_info->one-1] = tmpid;
			--(card_info->one);
		}
	}
}


void Renorm( CardInfo* card_info, PlayerId player, CardId card, float quantity ) {
}















	


