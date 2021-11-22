
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <pthread.h>
#include <sys/sysinfo.h>

#include "core.h"
#include "random.h"


/** @brief Samples card distribution
 *
 *  This function samples a card distribution according to the information given in card_info.
 *  It doesn't preserve dest nor card_info, so make sure to copy them.
 *
 *  @param dest Destination GameInfo object. Will only add cards to the cardsets of players 1-3. So
 *  			this can be used with an initialized GameInfo object where the remaining card will be
 *  			filled in.
 *  @param card_info Information about which cards are to be dealed and with which scores.
 *  @param random_state pointer to 32bit value used as random state by Random and RandomInt
 */
static void MCSample( GameInfo* dest, CardInfo* card_info, uint32_t* random_state ) {
	/* sample the cards */
	CardId c;
	int8_t i;
	PlayerId p;
	float r;
	while ( card_info->cards_left > 0 ) {
		c = --(card_info->cards_left);
		r = RandomF(random_state) * (card_info->scores[0][c] + card_info->scores[1][c] + card_info->scores[2][c]);
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
static void Prepare( GameInfo* game_info, CardInfo* card_info ) {
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

/** @brief this is the worker thread routine
 *
 * 	@param arg pointer to data structure holding all relevant data and return data fields
 * 	@return returns NULL, returned data is in result field of WorkerData struct
 */
static void * WorkerRoutine( WorkerData * arg ) {
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
    /* detect number of processors and launch as many thread. (threads are capped at 8 though */
	WorkerData* thread_args;
	thread_num = get_nprocs();
	thread_num = thread_num < 1 ? 1 : thread_num > 8 ? 8 : thread_num;
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

#if 0
/** @brief Adds quantity to entry in card_info and renorms the other entries.
 *
 *  @param card_info pointer to CardInfo object to operate on
 *  @param player player id. Note this is NOT in CardInfo player numbering scheme
 *  @param card card to renorm this is normal card id
 *  @param quantity Value to increase. All entries in card_info that match c will be increased
 *  			pass negative value to decrease.
 */
static void Renorm( CardInfo* card_info, PlayerId player, CardId card, float quantity ) {
	uint8_t card_i;

	/* search for entries that match card and count them */
	uint8_t card_num = 0;
	for ( card_i = 0; card_i < card_info->cards_left; card_i++ ) {
		if ( card_info->ids[card_i] == card ) {
			++card_num;
		}
	}

	/* renorm with card_num*quantity */
	for ( card_i = 0; card_i < card_info->cards_left; card_i++ ) {
		if ( card_info->ids[card_i] == card ) {
			card_info->scores[p][i] += quantity;
		} else {
			card_info->scores[p][i] -= quantity * card_info->scores[p][i]
											/ ( card_info->sum[p] - card_info->scores[p][c] );
		}
	}
}
#endif

/** @brief Marks a card for removing into GameInfo.cardset by Prepare.
 * 			
 * 		Sets one entry for card to 1.0 for player and 0.0 for the other players.
 * 		Thus Prepare will remove this entry from the list and add it to the GameInfo cardset member,
 * 		so that PlayCard may be called upon this card.
 *
 * 	@param card_info Pointer to CardInfo object to operate on
 * 	@param player Player Id. Note this is in Range (0-3) and will be converted into internal
 *			player id numbering scheme used for CardInfo objects
 *	@param card Card id
 */
static void RemoveToPlay( CardInfo* card_info, PlayerId player, CardId card ) {
	uint8_t card_i;
	PlayerId p = player-1;
	
	/* search for first occurence of card */
	uint8_t first_i;
	for ( card_i = 0; card_i < card_info->cards_left; card_i++ ) {
		if ( card_info->ids[card_i] == card ) {
			first_i = card_i;
			break;
		}
	}

	/* renorm other entries */
	for ( card_i = 0; card_i < card_info->cards_left; card_i++ ) {
		if ( card_i != first_i ) {
			card_info->scores[p][card_i] -= (1.0 - card_info->scores[p][first_i]) 
			   	* card_info->scores[p][card_i] / ( card_info->sum[p] - card_info->scores[p][first_i] );
			card_info->scores[(p+1)%3][card_i] += card_info->scores[(p+1)%3][first_i]
			   	* card_info->scores[(p+1)%3][card_i]
				/ ( card_info->sum[(p+1)%3] - card_info->scores[(p+1)%3][first_i] );
			card_info->scores[(p+2)%3][card_i] += card_info->scores[(p+2)%3][first_i]
			   	* card_info->scores[(p+2)%3][card_i] 
				/ ( card_info->sum[(p+2)%3] - card_info->scores[(p+2)%3][first_i] );
		}
	}
	card_info->scores[ p     ][first_i] = 1.0; 
	card_info->scores[(p+1)%3][first_i] = 0.0; 
	card_info->scores[(p+2)%3][first_i] = 0.0; 
}

/** @brief Execute a move.
 *
 * 		This will handle all internal things that happen when someone plays a card. In case the machine
 * 		itself plays a card, this simply removes it from the cardset[0] and updates the trick and scores.
 * 		In case of human player this also processes information about the card played, such as for
 * 		example if the player does not have a specific suit.
 *
 * 	@param game_info Pointer to GameInfo object to operate on.
 * 	@param card_info Pointer to CardInfo object to operate on.
 * 	@param card Card Id to play. Will not check if player has card.
 */
void ExecuteMove( GameInfo* game_info, CardInfo* card_info, CardId card ) {
	if ( game_info->next == 0 ) {
		/* perform machine move */
		PlayCard( game_info, card );
	} else {
		/* perform human move */
		RemoveToPlay( card_info, game_info->next, card );
		Prepare( game_info, card_info );
		PlayCard( game_info, card );
	}
}

