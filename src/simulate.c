#include <stdbool.h>

#include <pthread.h>
#include <sys/sysinfo.h>

#include "random.h"
#include "simulate.h"

char const* const card_names[25] = {
    "cn", "ck", "ct", "ca", //
    "sn", "sk", "st", "sa", //
    "hn", "hk", "ha",       //
    "dn", "dk", "dt", "da", //
    "dj", "hj", "sj", "cj", //
    "dq", "hq", "sq", "cq", //
    "ht", "ic",             //
};

char const* const card_names_long[25] = {
    "club nine",     "club king",    "club ten",    "club ace",    //
    "spade nine",    "spade king",   "spade ten",   "spade ace",   //
    "heard nine",    "heart king",   "heart ace",                  //
    "diamond nine",  "diamond king", "diamond ten", "diamond ace", //
    "diamond jack",  "heart jack",   "spade jack",  "club jack",   //
    "diamond queen", "heart queen",  "spade queen", "club queen",  //
    "heart ten",     "invalid",                                    //
};

static Score const card_values[25] = {
    0,  4, 10, 11, //
    0,  4, 10, 11, //
    0,  4, 11,     //
    0,  4, 10, 11, //
    2,  2, 2,  2,  //
    3,  3, 3,  3,  //
    10, 0,         //
};

static Suit const card_suits[25] = {
    CLUB,  CLUB,   CLUB,  CLUB,  //
    SPADE, SPADE,  SPADE, SPADE, //
    HEART, HEART,  HEART,        //
    TRUMP, TRUMP,  TRUMP, TRUMP, //
    TRUMP, TRUMP,  TRUMP, TRUMP, //
    TRUMP, TRUMP,  TRUMP, TRUMP, //
    TRUMP, NOSUIT,               //
};

static CardSet const suit_sets[6] = {
    0x00000000000000fful, 0x000000000000ff00ul, 0x00000000003f0000ul,
    0x0000000000000000ul, 0x0000ffffffc00000ul, 0x0000fffffffffffful,
};

Score Simulate(GameInfo const* game_info_in, CardId next_card,
               uint32_t* random_state) {
    GameInfo game_info;
    /* get a copy of the game_info and play the first card on it */
    game_info = *game_info_in;
    PlayCard(&game_info, next_card);

    CardId legal_cards[12];
    while (game_info.cards_left > 0) {
        /* determine legal cards to play */
        uint8_t legal_cards_len = GetLegalCards(&game_info, legal_cards);
        /* determine random legal card and play it */
        next_card = legal_cards[random_uint8(random_state, legal_cards_len)];
        PlayCard(&game_info, next_card);
    }

    /* return score for party of player 0 */
    Score result = 0;
    for (PlayerId p = 0; p < 4; p++) {
        if (game_info.player_isre[p] == game_info.player_isre[0]) {
            result += game_info.player_scores[p];
        }
    }
    return result;
}

uint8_t GetLegalCards(GameInfo const* game_info, CardId legal_cards[12]) {
    /* determine legal cards to play */
    CardSet legal_card_set = game_info->player_cardsets[game_info->next] &
                             suit_sets[game_info->tricksuit];
    if (legal_card_set == 0) {
        /* no card is legal, the suit enforcement is lifted */
        legal_card_set = game_info->player_cardsets[game_info->next];
    }

    uint8_t legal_cards_len = 0;
    uint8_t legal_card_id = 0;
    while (legal_card_set != 0) {
        if (legal_card_set & 1) {
            legal_cards[legal_cards_len++] = legal_card_id;
        }
        legal_card_id++;
        legal_card_set >>= 1;
    }
    return legal_cards_len;
}

void PlayCard(GameInfo* game_info, CardId card) {
    /* remove card from players hand */
    game_info->player_cardsets[game_info->next] &= ~CARDSHIFT(card);
    --(game_info->cards_left);

    /* add player to re if the club queen is played */
    if (card / 2 == CLUB_QUEEN_L / 2) {
        game_info->player_isre[game_info->next] = true;
    }

    /* add value to score */
    game_info->trickscore += card_values[card / 2];

    if (game_info->tricksuit == NOSUIT) {
        /* no card on trick */
        game_info->tricksuit = card_suits[card / 2];
        game_info->trickwinnercard = card;
        game_info->trickwinner = game_info->next;
    } else {
        /* test if new card is winning */
        if (card_suits[card / 2] == game_info->tricksuit ||
            card_suits[card / 2] == TRUMP) {
            if (card / 2 > game_info->trickwinnercard / 2 ||
                (card / 2 == HEART_TEN_L / 2 && game_info->cards_left > 4)) {
                game_info->trickwinnercard = card;
                game_info->trickwinner = game_info->next;
            }
        }
    }

    /* test if trick is full */
    if (game_info->cards_left % 4 == 0) {
        game_info->player_scores[game_info->trickwinner] +=
            game_info->trickscore;
        game_info->trickscore = 0;
        game_info->tricksuit = NOSUIT;
        game_info->next = game_info->trickwinner;
    } else {
        game_info->next = (game_info->next + 1) % 4;
    }
}

void Initialize(GameInfo* game_info) {
    for (PlayerId p = 0; p < 4; p++) {
        game_info->player_cardsets[p] = 0UL;
        game_info->player_scores[p] = 0;
        game_info->player_isre[p] = false;
    }
    game_info->cards_left = 48;
    game_info->next = 0;
    game_info->trickscore = 0;
    game_info->tricksuit = NOSUIT;
}
