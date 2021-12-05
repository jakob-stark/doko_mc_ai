#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "simulate.h"
#include "mc.h"
#include "core.h"

CardId CardFromString( const char input[2] ) {
	CardId c = INVALID;
		switch ( input[0] ) {
			case 'd':
				switch ( input[1] ) {
					case 'n': c = DIAMOND_NINE ; break;
					case 'j': c = DIAMOND_JACK ; break;
					case 'q': c = DIAMOND_QUEEN; break;
					case 'k': c = DIAMOND_KING ; break;
					case 't': c = DIAMOND_TEN  ; break;
					case 'a': c = DIAMOND_ACE  ; break;
				}
				break;
			case 'h':
				switch ( input[1] ) {
					case 'n': c = HEART_NINE ; break;
					case 'j': c = HEART_JACK ; break;
					case 'q': c = HEART_QUEEN; break;
					case 'k': c = HEART_KING ; break;
					case 't': c = HEART_TEN  ; break;
					case 'a': c = HEART_ACE  ; break;
				}
				break;
			case 's':
				switch ( input[1] ) {
					case 'n': c = SPADE_NINE ; break;
					case 'j': c = SPADE_JACK ; break;
					case 'q': c = SPADE_QUEEN; break;
					case 'k': c = SPADE_KING ; break;
					case 't': c = SPADE_TEN  ; break;
					case 'a': c = SPADE_ACE  ; break;
				}
				break;
			case 'c':
				switch ( input[1] ) {
					case 'n': c = CLUB_NINE ; break;
					case 'j': c = CLUB_JACK ; break;
					case 'q': c = CLUB_QUEEN; break;
					case 'k': c = CLUB_KING ; break;
					case 't': c = CLUB_TEN  ; break;
					case 'a': c = CLUB_ACE  ; break;
				}
				break;
		}
	return c;
}

void init( GameInfo* game_info, CardInfo* card_info ) {
    PlayerId p;

    /* initialize game_info */
    for ( p = 0; p < 4; p++ ) {
	    game_info->player_cardsets[p] = 0ul;
	    game_info->player_scores[p] = 0;
	    game_info->player_isre[p] = false;
    }

    game_info->cards_left = 48;
	game_info->next = 4;
	game_info->trickscore = 0;
	game_info->tricksuit = NOSUIT;

	printf("Enter beginning player :\n");
	scanf("%hhu", &(game_info->next));
    printf("Player %hhu begins\n", game_info->next);
 	
    printf("Enter cards for player 0\n");
	for ( int i = 0; i < 12; i++ ) {
	    CardId c;
	    char input[3];
		printf("Enter next card :\n");
		scanf("%2s", input);
		c = CardFromString(input);
        printf("Got card '%s'\n", card_names[c]);

		game_info->player_cardsets[0] += CARDSHIFT(c);
	}

    /* initialize card_info */
    for ( p = 0; p < 3; p++ ) {
        card_info->player_left[p] = 12;
    }

	CardSet set = game_info->player_cardsets[0];
    card_info->cards_left = 0;
	for ( uint8_t c = 0; c < 24; c++ ) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
		switch ( (set >> 2*c) & 0x3 ) {
			case 0:
                for ( p = 0; p < 3; p++ ) {
				    card_info->scores[card_info->cards_left][p] = 5;
                }
				card_info->ids[card_info->cards_left] = c;
				card_info->cards_left += 1;
			case 1:
                for ( p = 0; p < 3; p++ ) {
				    card_info->scores[card_info->cards_left][p] = 5;
                }
				card_info->ids[card_info->cards_left] = c;
				card_info->cards_left += 1;
		}
#pragma GCC diagnostic pop
	}

    /* perform checks */
}	

int main(void) {
	GameInfo g;
	CardInfo c;
	CardId card;
	init(&g, &c);
	while ( g.cards_left > 0 ) {
		printf("Player %hhu plays\n", g.next);
		if ( g.next == 0 ) {
			card = GetBestCard(&g, &c);
			printf("%s\n", card_names[card]);
		} else {
            printf("Enter card\n");
			char input[3];
			scanf("%2s", input);
			card = CardFromString(input);
            printf("Got card '%s'\n", card_names[card]);
		}
		ExecuteMove( &g, &c, card );
	}
	return 0;
}
