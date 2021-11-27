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
        CLUB_KING       =  2,
        CLUB_TEN        =  3,
        CLUB_ACE        =  4,
		SPADE_NINE      =  5,
        SPADE_KING      =  6,
        SPADE_TEN       =  7,
        SPADE_ACE       =  8,
		HEART_NINE      =  9,
        HEART_KING      = 10,
        HEART_ACE       = 11,
		DIAMOND_NINE    = 12,
        DIAMOND_KING    = 13,
        DIAMOND_TEN     = 14,
        DIAMOND_ACE     = 15,
		DIAMOND_JACK    = 16,
        HEART_JACK      = 17,
        SPADE_JACK      = 18,
        CLUB_JACK       = 19,
		DIAMOND_QUEEN   = 20,
        HEART_QUEEN     = 21,
        SPADE_QUEEN     = 22,
        CLUB_QUEEN      = 23,
		HEART_TEN       = 24,
        INVALID         = 255
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

typedef struct {
    Score player_score;
    CardId first_played_card;
} SimulationResult;

extern const char * const card_names[24];
extern const char * const card_names_long[24];

uint8_t GetLegalCards( const GameInfo* game_info, CardId legal_cards[12] );
void PlayCard( GameInfo* game_info, CardId card );
SimulationResult RandomSimulate( const GameInfo* game_info, uint32_t* random_state );

#endif

