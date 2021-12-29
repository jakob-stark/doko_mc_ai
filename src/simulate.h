#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>

/** @defgroup core core
 *
 *   The core module
 *
 *   Detailed core module description
 *   @todo add more documentation
 *
 *  @{
 */

enum {  CLUB            = 0,
        SPADE           = 1,
        HEART           = 2,
        DIAMOND         = 3,
        TRUMP           = 4,
        NOSUIT          = 5
};
typedef uint8_t Suit;

enum {	CLUB_NINE_L     =  0,
        CLUB_NINE_H     =  1,
        CLUB_KING_L     =  2,
        CLUB_KING_H     =  3,
        CLUB_TEN_L      =  4,
        CLUB_TEN_H      =  5,
        CLUB_ACE_L      =  6,
        CLUB_ACE_H      =  7,
		SPADE_NINE_L    =  8,
		SPADE_NINE_H    =  9,
        SPADE_KING_L    = 10,
        SPADE_KING_H    = 11,
        SPADE_TEN_L     = 12,
        SPADE_TEN_H     = 13,
        SPADE_ACE_L     = 14,
        SPADE_ACE_H     = 15,
		HEART_NINE_L    = 16,
		HEART_NINE_H    = 17,
        HEART_KING_L    = 18,
        HEART_KING_H    = 19,
        HEART_ACE_L     = 20,
        HEART_ACE_H     = 21,
		DIAMOND_NINE_L  = 22,
		DIAMOND_NINE_H  = 23,
        DIAMOND_KING_L  = 24,
        DIAMOND_KING_H  = 25,
        DIAMOND_TEN_L   = 26,
        DIAMOND_TEN_H   = 27,
        DIAMOND_ACE_L   = 28,
        DIAMOND_ACE_H   = 29,
		DIAMOND_JACK_L  = 30,
		DIAMOND_JACK_H  = 31,
        HEART_JACK_L    = 32,
        HEART_JACK_H    = 33,
        SPADE_JACK_L    = 34,
        SPADE_JACK_H    = 35,
        CLUB_JACK_L     = 36,
        CLUB_JACK_H     = 37,
		DIAMOND_QUEEN_L = 38,
		DIAMOND_QUEEN_H = 39,
        HEART_QUEEN_L   = 40,
        HEART_QUEEN_H   = 41,
        SPADE_QUEEN_L   = 42,
        SPADE_QUEEN_H   = 43,
        CLUB_QUEEN_L    = 44,
        CLUB_QUEEN_H    = 45,
		HEART_TEN_L     = 46,
		HEART_TEN_H     = 47,
        INVALID         = 48
};
typedef uint8_t CardId;
#define CARDSHIFT(card) (1ul << (card))
#define CARD_VALID(card) ((card) < INVALID)
#define CARD_EQUAL(card1,card2) ((card1)/2 == (card2)/2)

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


/** @brief short 2 byte card name abbreviations as null terminated strings
 *         (indexed by CardId) */
extern const char * const card_names[25];

/** @brief long card names as null terminated strings (indexed by CardId) */
extern const char * const card_names_long[25];


/** @brief get legal cards for next player
 *
 *  @param game_info pointer to the GameInfo object to get the legal cards from
 *  @param legal_cards pointer to array where the result is stored
 *  @return number of legal cards found and stored in legal_cards
 */
uint8_t GetLegalCards( const GameInfo* game_info, CardId legal_cards[12] );


/** @brief performs a card play
 *
 *  @param game_info pointer to GameInfo struct to operate on. Will not be
 *    preserved!
 *  @param card Id of card to play. Will not check for consistency. If the next
 *    player does not hold this card, the behaviour is undefined.
 */
void PlayCard( GameInfo* game_info, CardId card );


/** @brief Simulates a random game
 *
 *  First play the card denoted by next_card onto the game in game_infe. After
 *  that, random but legal cards are played until the game is over. The score
 *  that was achieved by player0's team is returned.
 *
 *  @note The first card is given in next_random instead of being random. This
 *    allows the core functions to simulate a random game with a specific card
 *    and study how well that card behaves.
 *
 *  @param game_info pointer to GameInfo struct to simulate
 *  @param next_card the first card, that is to be played
 *  @param random_state pointer to 32bit random state used by Random and
 *    RandomInt
 *  @return Score value for player 0
 */
Score Simulate( const GameInfo* game_info, CardId next_card, uint32_t* random_state );

/**@}*/

#endif

