#include <stdio.h>
#include "simulate.h"
#include "ab.h"

Score AlphaBetaEvaluate(const GameInfo* game_info_in, CardId next_card, Score alpha, Score beta) {
    Score result;
    GameInfo game_info;
    CardId legal_cards[12];
    uint8_t legal_cards_len;

    /* get a local copy of the game_info and play the first card on it */
    game_info = *game_info_in;
    PlayCard(&game_info, next_card);

    if ( game_info.cards_left >= 28 ) {
        for ( uint8_t i = 0; i < 48-game_info.cards_left; i++ ) {
            printf(" ");
        }
        printf("%hhu\n", game_info.cards_left);
    }

    /* if the game ended return the score */
    if ( game_info.cards_left == 0 ) {
        result = 0;
        for ( PlayerId p = 0; p < 4; p++ ) {
            if ( game_info.player_isre[p] == game_info.player_isre[0] ) {
                result += game_info.player_scores[p];
            }
        }
        return result;
    }

    /* get the legal cards */
    legal_cards_len = GetLegalCards(&game_info, legal_cards);

    if ( game_info.player_isre[game_info.next] == game_info.player_isre[0] ) {
        /* maximizing player */
        result = 0;
        for ( uint8_t c = 0; c < legal_cards_len; c++ ) {
            Score s = AlphaBetaEvaluate(&game_info, legal_cards[c], alpha, beta);
            if ( s > result ) {
                result = s;
            }
            if ( result >= beta ) {
                /* beta cutoff */
                return beta;
            }
            if ( result > alpha ) {
                alpha = result;
            }
        }
        return result;

    } else {
        /* minimizing player */
        result = 0xff;
        for ( uint8_t c = 0; c < legal_cards_len; c++ ) {
            Score s = AlphaBetaEvaluate(&game_info, legal_cards[c], alpha, beta);
            if ( s < result ) {
                result = s;
            }
            if ( result <= alpha ) {
                /* alpha cutoff */
                return alpha;
            }
            if ( result < beta ) {
                beta = result;
            }
        }
        return result;
    }
}

