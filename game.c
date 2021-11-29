#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <threads.h>

#include <sys/sysinfo.h>

#include "game.h"

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

