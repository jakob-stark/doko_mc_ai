#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <pthread.h>
#include <sys/sysinfo.h>

#include "game.h"

const char * const card_names[24] = { 
	"cn", "ck", "ct", "ca", "sn", "sk", "st", "sa", "hn", "hk", "ha",
	"dn", "dk", "dt", "da", "dj", "hj", "sj", "cj", "dq", "hq", "sq", "cq", "ht"
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

/** @brief Samples random float numbers in [0,1) interval
 *
 * @return Random number (float) in [0,1) interval
 */
static float Random(uint32_t* state) {
	union {
		uint32_t i;
		float f;
	} u;
	u.i = *state;
	u.i ^= u.i << 13;
	u.i ^= u.i >> 17;
	u.i ^= u.i << 5;
	*state = u.i;
	u.i = u.i & 0x007fffff | 0x3f800000;
	return u.f-1.0;
}

/** @brief Samples a random integer number in [0,a) interval
 *
 * @param a upper edge of sample interval
 * @return Random integer number between 0 (inclusive) and a (exclusive)
 */
static uint8_t RandomInt(uint32_t* state, uint8_t a) {
	uint32_t i;
	i = *state;
	i ^= i << 13;
	i ^= i >> 17;
	i ^= i << 5;
	*state = i;
	return i % a;
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

/** @brief Simulates a random game
 *
 * @param game_info pointer to GameInfo struct to operate on. game_info will not be preserved!
 * @param random_state pointer to 32bit random state used by Random and RandomInt
 * @return Score value for player 0
 */
Score Simulate( GameInfo* game_info, uint32_t* random_state ) {
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
		PlayCard( game_info, legal_cards[RandomInt(random_state, legal_cards_len)] );
	}
	/* return score for party of player 0 */
	PlayerId p;
	Score result = 0;
	for ( p = 0; p < 4; p++ ) {
		if ( game_info->player_isre[p] == game_info->player_isre[0] ) {
			result += game_info->player_scores[p];
		}
	}
	return result;
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
 *  @param random_state pointer to 32bit value used as random state by Random and RandomInt
 */
void MCSample( GameInfo* dest, CardInfo* card_info, uint32_t* random_state ) {
	/* sample the cards */
	CardId c;
	int8_t i;
	PlayerId p;
	float r;
	while ( card_info->cards_left > 0 ) {
		c = --(card_info->cards_left);
		r = Random(random_state) * (card_info->scores[0][c] + card_info->scores[1][c] + card_info->scores[2][c]);
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
		if ( card_info->ids[c] == CLUB_QUEEN ) {
			dest->player_isre[p+1] = true;
		}
	}
}

/** @brief prepares card_info and game_info for MC sampling
 *
 * 		this removes all entries in card_info which contain two zeros and
 * 		deals them to the corresponding player. Additionally this sorts the
 * 		entries in such manner that all entries which have one zero appear at
 * 		the end of the list. This makes MCSample more stable
 *
 * 	@param game_info pointer to GameInfo object to operate on. Only cardsets of
 * 		players will be moified
 * 	@param card_info pointer to CardInfo object to operate on.
 */
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

/** @brief holds all data a worker thread needs
 */
typedef struct {
	/* thread uid */
	pthread_t thread;
	/* parameters */
	const CardId * card_list;
	uint8_t card_list_len;
	const GameInfo * game_info_in;
	const CardInfo * card_info_in;
	clock_t time_goal;
	uint32_t random_seed;
	/* return values */
	uint32_t result[12];
	/* static data not initialized */
	GameInfo game_info_tmp;
	GameInfo game_info_tmp2;
	CardInfo card_info_tmp;
} WorkerData;

#include "cardinfo.c"

/** @brief this is the worker thread routine
 *
 * 	@param arg pointer to data structure holding all relevant data and return data fields
 * 	@return returns NULL, returned data is in result field of WorkerData struct
 */
void * WorkerRoutine( WorkerData * arg ) {
 	#define NSIM 100
	uint8_t card_list_i;
	arg->time_goal += clock();

	uint32_t j = 0;
	while ( clock() < arg->time_goal ) {
		arg->game_info_tmp = *arg->game_info_in;
		arg->card_info_tmp = *arg->card_info_in;
		MCSample( &arg->game_info_tmp, &arg->card_info_tmp, &arg->random_seed );
		for ( card_list_i = 0; card_list_i < arg->card_list_len; card_list_i++ ) {
			for ( j = 0; j < NSIM; j++ ) {
				arg->game_info_tmp2 = arg->game_info_tmp;
				PlayCard( &arg->game_info_tmp2, arg->card_list[card_list_i] );
				arg->result[card_list_i] += Simulate( &arg->game_info_tmp2, &arg->random_seed );
			}
		}
	}
	return NULL;
}

/** @brief gets best card. This is the main routine of the ai. It creates worker threads and
 * 			manages all central important stuff.
 *	@param game_info pointer to GameInfo object
 *	@param card_info holds dat concerning the card distribution probabilities
 *	@return returns id of best card.
 */
CardId GetBestCard( GameInfo* game_info, CardInfo* card_info ) {
	/* prepare objects */
	Prepare( game_info, card_info );
	
	CardId card_list[12];
	uint32_t card_scores[12] = {0};
	uint8_t card_list_len = 0;

	uint8_t card_list_i;

	/* find list of legal cards */
	CardId card_id = 0;
	CardSet card_set  = game_info->player_cardsets[0] & suit_sets[game_info->tricksuit];
	if ( card_set == 0 ) {
		card_set = game_info->player_cardsets[0];
	}
	while ( card_set != 0 ) {
		switch ( card_set % 4 ) {
			case 2:
				card_list[card_list_len++] = card_id;
			case 1:
				card_list[card_list_len++] = card_id;
		}
		card_id++;
		card_set >>= 2;
	}

	/* start worker threads */
	uint8_t thread_i;
	uint8_t thread_num;
	WorkerData* thread_args;
	thread_num = get_nprocs();
	thread_num = thread_num < 1 ? 1 : thread_num > 8 ? 8 : thread_num;
	thread_num = 4;
	thread_args = malloc(sizeof(WorkerData)*thread_num);
	for ( thread_i = 0; thread_i < thread_num; thread_i++ ) {
		/* initialize args */
		thread_args[thread_i].card_list = card_list;
		thread_args[thread_i].card_list_len = card_list_len;
		thread_args[thread_i].game_info_in = game_info;
		thread_args[thread_i].card_info_in = card_info;
		thread_args[thread_i].time_goal = 8*CLOCKS_PER_SEC*thread_num;
		thread_args[thread_i].random_seed = rand();
		for ( card_list_i = 0; card_list_i < card_list_len; card_list_i++ ) {
			thread_args[thread_i].result[card_list_i] = 0ul;
		}
		/* start the worker */
		pthread_create( &(thread_args[thread_i].thread), NULL, (void*(*)(void*))&WorkerRoutine, &(thread_args[thread_i]) );
		//WorkerRoutine( &(thread_args[thread_i]) );
	}
	
	/* join worker threads */
	for ( thread_i = 0; thread_i < thread_num; thread_i++ ) {
		pthread_join( thread_args[thread_i].thread, NULL );
		for ( card_list_i = 0; card_list_i < card_list_len; card_list_i++ ) {
		   card_scores[card_list_i] += 	thread_args[thread_i].result[card_list_i];
		}
	}

	/* find best card in card list */
	uint32_t max_score = 0;
	CardId max_id;
	for ( card_list_i = 0; card_list_i < card_list_len; card_list_i++ ) {
		if ( card_scores[card_list_i] > max_score ) {
			max_score = card_scores[card_list_i];
			max_id = card_list[card_list_i];
		}
	}
	return max_id;
}



