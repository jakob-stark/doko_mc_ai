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

