#ifndef GAME_H
#define GAME_H

#include <stdint.h>

enum {CLUB=0, SPADE, HEART, DIAMOND, TRUMP, NOSUIT};
typedef uint8_t Suit;

typedef uint8_t Score;

enum {	CLUB_NINE=0, CLUB_KING, CLUB_TEN, CLUB_ACE,
		SPADE_NINE, SPADE_KING, SPADE_TEN, SPADE_ACE,
		HEART_NINE, HEART_KING, HEART_ACE,
		DIAMOND_NINE, DIAMOND_KING, DIAMOND_TEN, DIAMOND_ACE,
		DIAMOND_JACK, HEART_JACK, SPADE_JACK, CLUB_JACK,
		DIAMOND_QUEEN, HEART_QUEEN, SPADE_QUEEN, CLUB_QUEEN,
		HEART_TEN
};
typedef uint8_t CardId;
#define CARDSHIFT(card) (1ul << 2*(card))

typedef uint8_t PlayerId;
typedef uint64_t CardSet;

#define RE 0x0080000000000000ul
#define GET_RE(cardset) ((cardset) & RE)
#define SET_RE(cardset) ((cardset) &= ~RE)
#define UNSET_RE(cardset) ((cardset) |= RE)

typedef struct {
	CardSet  player_cardsets[4];
	Score    player_scores[4];
	uint8_t  cards_left;

	PlayerId next;

	Score    trickscore;
	Suit     tricksuit;
	CardId   trickwinnercard;
	PlayerId trickwinner;
} GameInfo;

//extern const Score card_values[24];
//extern const Score card_suits[24];

#endif

