#ifndef CORE_H
#define CORE_H

#include <stdbool.h>
#include <stdint.h>

#include "random.h"

/** @defgroup core core
 *
 *   The core module
 *
 *   Detailed core module description
 *   @todo add more documentation
 *
 *  @{
 */

enum { CLUB = 0, SPADE = 1, HEART = 2, DIAMOND = 3, TRUMP = 4, NOSUIT = 5 };
typedef uint8_t doko_suit_t;

enum {
    CLUB_NINE_L = 0,
    CLUB_NINE_H = 1,
    CLUB_KING_L = 2,
    CLUB_KING_H = 3,
    CLUB_TEN_L = 4,
    CLUB_TEN_H = 5,
    CLUB_ACE_L = 6,
    CLUB_ACE_H = 7,
    SPADE_NINE_L = 8,
    SPADE_NINE_H = 9,
    SPADE_KING_L = 10,
    SPADE_KING_H = 11,
    SPADE_TEN_L = 12,
    SPADE_TEN_H = 13,
    SPADE_ACE_L = 14,
    SPADE_ACE_H = 15,
    HEART_NINE_L = 16,
    HEART_NINE_H = 17,
    HEART_KING_L = 18,
    HEART_KING_H = 19,
    HEART_ACE_L = 20,
    HEART_ACE_H = 21,
    DIAMOND_NINE_L = 22,
    DIAMOND_NINE_H = 23,
    DIAMOND_KING_L = 24,
    DIAMOND_KING_H = 25,
    DIAMOND_TEN_L = 26,
    DIAMOND_TEN_H = 27,
    DIAMOND_ACE_L = 28,
    DIAMOND_ACE_H = 29,
    DIAMOND_JACK_L = 30,
    DIAMOND_JACK_H = 31,
    HEART_JACK_L = 32,
    HEART_JACK_H = 33,
    SPADE_JACK_L = 34,
    SPADE_JACK_H = 35,
    CLUB_JACK_L = 36,
    CLUB_JACK_H = 37,
    DIAMOND_QUEEN_L = 38,
    DIAMOND_QUEEN_H = 39,
    HEART_QUEEN_L = 40,
    HEART_QUEEN_H = 41,
    SPADE_QUEEN_L = 42,
    SPADE_QUEEN_H = 43,
    CLUB_QUEEN_L = 44,
    CLUB_QUEEN_H = 45,
    HEART_TEN_L = 46,
    HEART_TEN_H = 47,
    INVALID = 48
};
typedef uint8_t doko_card_t;
#define DOKO_CARDSHIFT(card) (1ul << (card))
#define DOKO_CARD_VALID(card) ((card) < INVALID)
#define DOKO_CARD_EQUAL(card1, card2) ((card1) / 2 == (card2) / 2)

#define DOKO_CARDSET_CONTAINS(set, card) ((set) & DOKO_CARDSHIFT(card))

#define DOKO_CARDSET_DIFFERENCE(rhs, lhs) ((rhs) & ~(lhs))

typedef uint8_t doko_score_t;
typedef uint8_t doko_count_t;

typedef uint8_t doko_player_t;
#define DOKO_FOR_EACH_PLAYER(x) for (doko_player_t x = 0; x < 4; x++)

typedef uint64_t doko_cardset_t;

typedef struct {
    doko_cardset_t player_cardsets[4];
    doko_score_t player_scores[4];
    bool player_isre[4];
    doko_count_t cards_left;

    doko_player_t next;

    doko_score_t trickscore;
    doko_suit_t tricksuit;
    doko_card_t trickwinnercard;
    doko_player_t trickwinner;
} doko_game_info_t;

/**
 * @brief short 2 byte card name abbreviations as null terminated strings
 *   (indexed by CardId)
 */
extern char const* const doko_card_names[25];

/** @brief long card names as null terminated strings (indexed by CardId) */
extern char const* const doko_card_names_long[25];

/**
 * @brief get legal cards for next player
 *
 * @param[in] game_info pointer to the GameInfo object to get the legal cards
 * from
 * @param[out] legal_cards pointer to array where the result is stored
 * @return number of legal cards found and stored in legal_cards
 */
doko_count_t doko_get_legal_cards(doko_game_info_t const* game_info,
                                  doko_card_t legal_cards[12]);

/**
 * @brief get legal cards for next player
 *
 * this does the same calculations than @ref doko_get_legal_cards but returns
 * the result as cardset.
 *
 * @param[in] game_info game info object to perform action on
 * @return a cardset containing the legal cards
 */
doko_cardset_t doko_get_legal_cardset(doko_game_info_t const* game_info);

/**
 * @brief converts a cardset to an array of card ids
 *
 * @param[in] cardset
 * @param[out] cards the target array - *must* be long enough to hold all cards
 * in the cardset
 */
doko_count_t doko_cardset_to_array(doko_cardset_t cardset, doko_card_t cards[]);

/**
 * @brief performs a card play
 *
 * @param game_info[in,out] game info object to operate - will not be preserved!
 * @param card id of card to play. Will not check for consistency. If the next
 * player does not hold this card, the behaviour is undefined.
 */
void doko_play_card(doko_game_info_t* game_info, doko_card_t card);

/**
 * @brief Simulates a random game
 *
 * First play the card denoted by next_card onto the game in game_infe.
 * After that, random but legal cards are played until the game is over. The
 * score that was achieved by player0's team is returned.
 *
 * @note The first card is given in next_random instead of being random.
 * This allows the core functions to simulate a random game with a specific
 * card and study how well that card behaves.
 *
 * @param game_info pointer to GameInfo struct to simulate
 * @param next_card the first card, that is to be played
 * @param random_state pointer to 32bit random state used by Random and
 *   RandomInt
 * @return Score value for player 0
 */
doko_score_t doko_simulate(doko_game_info_t const* game_info,
                           doko_card_t next_card,
                           doko_random_state_t* random_state);

/**
 * @brief simulates a random game
 *
 * This does the same as \ref doko_simulate with the difference, that it
 *
 *   a) does modify the passed game_info object
 *   b) does not play a deterministic first card
 *   c) does not return a pre calculated score
 *
 * Use this function instead of \ref doko_simulate if you need to calculate
 * the score yourself or you need some additionaly game state information.
 */
void doko_simulate_v2(doko_game_info_t* game_info,
                      doko_random_state_t* random_state);

/**@}*/

#endif
