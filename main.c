#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "game.h"

CardId CardFromString( const char input[2] ) {
	CardId c;
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
	game_info->player_cardsets[0] = 0ul;
	game_info->player_cardsets[1] = 0ul;
	game_info->player_cardsets[2] = 0ul;
	game_info->player_cardsets[3] = 0ul;

	game_info->player_scores[0] = 0;
	game_info->player_scores[1] = 0;
	game_info->player_scores[2] = 0;
	game_info->player_scores[3] = 0;
	
	game_info->player_isre[0] = false;
	game_info->player_isre[1] = false;
	game_info->player_isre[2] = false;
	game_info->player_isre[3] = false;

	game_info->cards_left = 48;
	game_info->next = 0;
	game_info->trickscore = 0;
	game_info->tricksuit = NOSUIT;

	printf("Enter beginning player :")
	scanf("%hhu", &(game_info->next))
 	
	for ( int i = 0; i < 12; i++ ) {
		CardId c;

		char input[3];
		printf("Enter next card :");
		scanf("%2s", input);
		c = CardFromString(input);
		game_info->player_cardsets[0] += CARDSHIFT(c);
		if ( c == CLUB_QUEEN ) {
			game_info->player_isre[0] = true;
		}
	}

	card_info->cards_left = 0;
	CardSet set = game_info->player_cardsets[0];
	for ( int i = 0; i < 24; i++ ) {
		switch ( set % 4 ) {
			case 0:
				card_info->scores[0][card_info->cards_left] = 1.0/3.0;
				card_info->scores[1][card_info->cards_left] = 1.0/3.0;
				card_info->scores[2][card_info->cards_left] = 1.0/3.0;
				card_info->ids[card_info->cards_left] = i;
				card_info->cards_left += 1;
			case 1:
				card_info->scores[0][card_info->cards_left] = 1.0/3.0;
				card_info->scores[1][card_info->cards_left] = 1.0/3.0;
				card_info->scores[2][card_info->cards_left] = 1.0/3.0;
				card_info->ids[card_info->cards_left] = i;
				card_info->cards_left += 1;
		}
		set >>= 2;
	}
	card_info->one = card_info->cards_left;
	card_info->sum[0] = 12.0;
	card_info->sum[1] = 12.0;
	card_info->sum[2] = 12.0;
}	

int main(int argc, char ** argv) {
	GameInfo g;
	CardInfo c;
	CardId card;
	init(&g, &c);
	while ( g.cards_left > 0 ) {
		printf("Player %d plays\n", (int)g.next);
		if ( g.next == 0 ) {
			card = GetBestCard(&g, &c);
			printf("%s\n", card_names[card]);
		} else {
			char input[3];
			scanf("%2s", input);
			card = CardFromString(input);
		}
		ExecuteMove( &g, &c, card );
	}
	return 0;
}
