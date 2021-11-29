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
	/* input parameters */
    const CardId * legal_cards;
    CardId legal_cards_len;
	const GameInfo * game_info;
	const CardInfo * card_info;
    /* additional options */
    uint8_t stop;
	clock_t time_goal;
	uint32_t random_seed;
    thrd_t thread_id;
	/* return values */
    uint32_t mc_sample_calls;
    uint32_t simulate_calls;
	uint32_t results[12];
} worker_data_t;

/** @brief this is the worker thread routine
 *
 * 	@param arg pointer to data structure holding all relevant data and return data fields
 * 	@return returns NULL, returned data is in result field of WorkerData struct
 */
void * WorkerRoutine( worker_data_t * arg ) {
 	#define NSIM 100

    /* get a copy of the game_info and card_info structs,
     * as we need to change it during Simulation */
    GameInfo game_info = *(arg->game_info);

    /* calculate absolute goal time */
	arg->time_goal += clock();

	uint32_t j = 0;
	while ( clock() < arg->time_goal && arg->stop == 0 ) {
        /* get a fresh copy of the game info */
        game_info = *(arg->game_info);

        /* randomly distribute the cards */
        mc_sample(&game_info, arg->card_info, &arg->random_seed);
        arg->mc_sample_calls++;

        /* simulate a game for each legal card and collect the results */
        for ( uint8_t c = 0; c < legal_cards_len; c++ ) {
            GameInfo game_info_copy = game_info;
            arg->result[c] += Simulate(&game_info_copy, arg->legal_cards[c], &arg->random_seed);
            arg->simulate_calls++;
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
CardId GetBestCard( const GameInfo* game_info, const CardInfo* card_info ) {
    /* get a copy of the card info */
    CardInfo card_info = *card_info;

	/* prepare card_info by sorting it */
    sort_and_check(&card_info);

    /* get legal cards for next move */
	CardId legal_cards[12];
	uint8_t legal_cards_len;
    legal_cards_len = GetLegalCards(game_info, legal_cards);

    /* prepare the output */
	uint32_t results[12] = {0};

    /* detect number of processors and launch as many thread. (threads are capped at 16 though) */
	uint8_t thread_num;
	worker_data_t* thread_args;
	thread_num = get_nprocs();
	thread_num = thread_num < 1 ? 1 : thread_num > 16 ? 16 : thread_num;
	thread_args = malloc(sizeof(worker_data_t)*thread_num);
	for ( uint8_t thread_i = 0; thread_i < thread_num; thread_i++ ) {
		/* initialize parameters */
		thread_args[thread_i].legal_cards = legal_cards;
		thread_args[thread_i].legal_cards_len = legal_cards_len;
		thread_args[thread_i].game_info = game_info;
		thread_args[thread_i].card_info = card_info;
        /* initialize options */
        thread_args[thread_i].stop = 0;
		thread_args[thread_i].time_goal = 1*CLOCKS_PER_SEC;
		thread_args[thread_i].random_seed = rand();

        /* initialize return values */
        thread_args[thread_i].mc_sample_calls = 0ul;
        thread_args[thread_i].simulate_calls = 0ul;
        thread_args[thread_i].mc_sample_calls = 0ul;
        memset(&thread_args[thread_i].results, 0, sizeof(thread_args[thread_i].results));

		/* start the worker */
        thrd_create(&thread_args[thread_i].thread_id,
                    (thread_start_t)&WorkerRoutine, &thread_args[thread_i]);
	}
    ??? continue here
	
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

