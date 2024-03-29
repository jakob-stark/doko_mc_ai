#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <threads.h>

#include <sys/sysinfo.h>
#include <time.h>

#include "core.h"
#include "mc.h"
#include "log.h"

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
	float time_goal;
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
int WorkerRoutine( worker_data_t * arg ) {
 	#define NSIM 100
    #define ELAPSED ((finish.tv_sec - start.tv_sec) + 1.0e-9 * (finish.tv_nsec - start.tv_nsec))

    /* get a copy of the game_info and card_info structs,
     * as we need to change it during Simulation */
    GameInfo game_info = *(arg->game_info);

    /* record start time */
    struct timespec start, finish;
    clock_gettime(CLOCK_MONOTONIC, &start);
    finish = start;

    while ( ELAPSED < arg->time_goal && arg->stop == 0 ) {
        /* get a fresh copy of the game info */
        game_info = *(arg->game_info);

        /* randomly distribute the cards */
        mc_sample(&game_info, arg->card_info, &arg->random_seed);
        arg->mc_sample_calls++;

        /* simulate a game for each legal card and collect the results */
        for ( uint8_t c = 0; c < arg->legal_cards_len; c++ ) {
            GameInfo game_info_copy = game_info;
            arg->results[c] += Simulate(&game_info_copy, arg->legal_cards[c], &arg->random_seed);
            arg->simulate_calls++;
		}
        clock_gettime(CLOCK_MONOTONIC, &finish);
	}

    arg->time_goal = ELAPSED;
	return 0;
}

/** @brief gets best card. This is the main routine of the ai. It creates worker threads and
 * 			manages all central important stuff.
 *	@param game_info pointer to GameInfo object
 *	@param card_info holds dat concerning the card distribution probabilities
 *	@return returns id of best card.
 */
CardId GetBestCard( const GameInfo* game_info, const CardInfo* card_info ) {
#define MULTITHREADED
    /* get a copy of the card info */
    CardInfo card_info_copy = *card_info;

	/* prepare card_info by sorting it */
    sort_and_check(&card_info_copy);

    /* get legal cards for next move */
	CardId legal_cards[12];
	uint8_t legal_cards_len;
    legal_cards_len = GetLegalCards(game_info, legal_cards);

    /* prepare the output */
	uint32_t results[12] = {0};

    /* detect number of processors and launch as many thread. (threads are capped at 16 though) */
	uint8_t thread_num;
	worker_data_t* thread_args;
#ifdef MULTITHREADED
	thread_num = get_nprocs();
	thread_num = thread_num < 1 ? 1 : thread_num > 16 ? 16 : thread_num;
#else
    thread_num = 1;
#endif
	thread_args = malloc(sizeof(worker_data_t)*thread_num);
	for ( uint8_t thread_i = 0; thread_i < thread_num; thread_i++ ) {
		/* initialize parameters */
		thread_args[thread_i].legal_cards = legal_cards;
		thread_args[thread_i].legal_cards_len = legal_cards_len;
		thread_args[thread_i].game_info = game_info;
		thread_args[thread_i].card_info = &card_info_copy;
        /* initialize options */
        thread_args[thread_i].stop = 0;
		thread_args[thread_i].time_goal = 1.;
		thread_args[thread_i].random_seed = rand();

        /* initialize return values */
        thread_args[thread_i].mc_sample_calls = 0ul;
        thread_args[thread_i].simulate_calls = 0ul;
        thread_args[thread_i].mc_sample_calls = 0ul;
        memset(&thread_args[thread_i].results, 0, sizeof(thread_args[thread_i].results));

		/* start the worker */
#ifdef MULTITHREADED
        thrd_create(&thread_args[thread_i].thread_id,
                    (int (*)(void *))&WorkerRoutine, &thread_args[thread_i]);
#else
        thread_args[thread_i].thread_id = 0;
        WorkerRoutine(&thread_args[thread_i]);
#endif
	}
	
	/* join worker threads */
    uint32_t mc_sample_calls = 0, simulate_calls = 0;
    float total_time = 0.0;
	for ( uint8_t thread_i = 0; thread_i < thread_num; thread_i++ ) {
#ifdef MULTITHREADED
		thrd_join(thread_args[thread_i].thread_id, NULL);
#endif
		for ( uint8_t c = 0; c < legal_cards_len; c++ ) {
		   results[c] += thread_args[thread_i].results[c];
		}
        mc_sample_calls += thread_args[thread_i].mc_sample_calls;
        simulate_calls += thread_args[thread_i].simulate_calls;
        total_time += thread_args[thread_i].time_goal;
	}

	/* find best card in card list */
	uint32_t max_score = 0;
	CardId max_id = 0;
	for ( uint8_t c = 0; c < legal_cards_len; c++ ) {
        print_log(LOG_INFO, "%.2f points expected for %s",
                (double)results[c]/(double)mc_sample_calls, card_names_long[legal_cards[c]]);
		if ( results[c] > max_score ) {
			max_score = results[c];
			max_id = legal_cards[c];
		}
	}
    print_log(LOG_INFO, "Simulated %lu games during %lu samplings in %hhux%0.3f s",
            simulate_calls, mc_sample_calls, thread_num, total_time/thread_num);
    print_log(LOG_INFO, "%.2f points expected for %s",
            (double)max_score/(double)mc_sample_calls, card_names_long[max_id]);
	return max_id;
#undef MULTITHREADED
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

        /* remove the played card from the list of cards */
        for ( uint8_t c = 0; c < card_info->cards_left; c++ ) {
            if ( card_info->ids[c] == card ) {
                memmove(&card_info->ids[c], &card_info->ids[c+1], sizeof(card_info->ids[0])*(card_info->cards_left - c - 1));
                memmove(&card_info->scores[c], &card_info->scores[c+1], sizeof(card_info->scores[0])*(card_info->cards_left - c - 1));
                card_info->cards_left--;
                card_info->player_left[game_info->next - 1]--;
                break;
            }
        }

		PlayCard( game_info, card );

        /* fixme dirty fix for that play card subtracts from cardset */
        for ( uint8_t p = 1; p < 4; p++ ) {
            game_info->player_cardsets[p] = 0ul;
        }

	}
}

