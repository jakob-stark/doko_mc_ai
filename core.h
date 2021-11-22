#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>

enum {CLUB=0, SPADE, HEART, DIAMOND, TRUMP, NOSUIT};
typedef uint8_t Suit;

typedef uint8_t Score;

enum {	CLUB_NINE=0, CLUB_KING, CLUB_TEN, CLUB_ACE,
		SPADE_NINE, SPADE_KING, SPADE_TEN, SPADE_ACE,
		HEART_NINE, HEART_KING, HEART_ACE,
		DIAMOND_NINE, DIAMOND_KING, DIAMOND_TEN, DIAMOND_ACE,
		DIAMOND_JACK, HEART_JACK, SPADE_JACK, CLUB_JACK,
		DIAMOND_QUEEN, HEART_QUEEN, SPADE_QUEEN, CLUB_QUEEN,
		HEART_TEN, INVALID=255
};
typedef uint8_t CardId;
#define CARDSHIFT(card) (1ul << 2*(card))

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
SimulationResult Simulate( const GameInfo* game_info, uint32_t* random_state );

#endif

