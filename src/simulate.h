#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>

enum {  CLUB            = 0,
        SPADE           = 1,
        HEART           = 2,
        DIAMOND         = 3,
        TRUMP           = 4,
        NOSUIT          = 5
};
typedef uint8_t Suit;

enum {	CLUB_NINE       =  0,
        CLUB_KING       =  1,
        CLUB_TEN        =  2,
        CLUB_ACE        =  3,
		SPADE_NINE      =  4,
        SPADE_KING      =  5,
        SPADE_TEN       =  6,
        SPADE_ACE       =  7,
		HEART_NINE      =  8,
        HEART_KING      =  9,
        HEART_ACE       = 10,
		DIAMOND_NINE    = 11,
        DIAMOND_KING    = 12,
        DIAMOND_TEN     = 13,
        DIAMOND_ACE     = 14,
		DIAMOND_JACK    = 15,
        HEART_JACK      = 16,
        SPADE_JACK      = 17,
        CLUB_JACK       = 18,
		DIAMOND_QUEEN   = 19,
        HEART_QUEEN     = 20,
        SPADE_QUEEN     = 21,
        CLUB_QUEEN      = 22,
		HEART_TEN       = 23,
        INVALID         = 24
};
typedef uint8_t CardId;
#define CARDSHIFT(card) (1ul << 2*(card))
#define CARD_VALID(card) ((card) <= HEART_TEN)

typedef uint8_t Score;
typedef uint8_t PlayerId;
typedef uint64_t CardSet;

typedef struct {
	CardSet  player_cardsets[4];
	Score    player_scores[4];
	bool	 player_isre[4];
	uint8_t  cards_left;

	PlayerId next;

	Score    trickscore;
	Suit     tricksuit;
	CardId   trickwinnercard;
	PlayerId trickwinner;
} GameInfo;

extern const char * const card_names[25];
extern const char * const card_names_long[25];

uint8_t GetLegalCards( const GameInfo* game_info, CardId legal_cards[12] );
void PlayCard( GameInfo* game_info, CardId card );
Score Simulate( const GameInfo* game_info, CardId next_card, uint32_t* random_state );

#endif

