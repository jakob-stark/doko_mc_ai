#include "ab.h"
#include "simulate.h"
#include <stdio.h>
#include <string.h>

typedef uint16_t hash_t;

typedef struct {
    GameInfo game_info;
    Score score;
    Score alpha;
    Score beta;
} HashEntry;

HashEntry Table[0x10000] = {0};
int hits = 0;
int miss = 0;

hash_t Hash(GameInfo const* game_info) {
    hash_t result = 0;
    for (uint8_t i = 0; i < sizeof(GameInfo) / 2; i++) {
        result ^= ((uint16_t const*)game_info)[i];
    };
    return result;
}

void print_table_usage(void) {
    int result = 0;
    for (int i = 0; i < 0x10000; i++) {
        if (Table[i].score != 0) {
            result++;
        }
    }
    printf("table usage %.2f%%\n", result * 100. / 0x10000);
    printf("hits %.2f%% %d\n", hits * 100. / (hits + miss), hits + miss);
}

Score GetFromTable(GameInfo const* game_info, Score alpha, Score beta) {
    hash_t hash = Hash(game_info);
    if (Table[hash].score != 0) {
        if (Table[hash].alpha <= alpha && beta <= Table[hash].beta) {
            if (memcmp(&(Table[hash].game_info), game_info, sizeof(GameInfo)) ==
                0) {
                hits++;
                return Table[hash].score;
            }
        }
    }
    miss++;
    return 0xff;
}

void StoreToTable(GameInfo const* game_info, Score score, Score alpha,
                  Score beta) {
    hash_t hash = Hash(game_info);
    if (Table[hash].score != 0) {
        if (Table[hash].game_info.cards_left > game_info->cards_left) {
            return;
        }
    }
    Table[hash].score = score;
    Table[hash].game_info = *game_info;
    Table[hash].alpha = alpha;
    Table[hash].beta = beta;
}

Score AlphaBetaEvaluate(GameInfo const* game_info_in, CardId next_card,
                        Score alpha, Score beta) {
    Score result;
    GameInfo game_info;
    CardId legal_cards[12];
    uint8_t legal_cards_len;

    /* get a local copy of the game_info and play the first card on it */
    game_info = *game_info_in;
    PlayCard(&game_info, next_card);

    /* if the game ended return the score */
    if (game_info.cards_left <= 36) {
        result = 0;
        for (PlayerId p = 0; p < 4; p++) {
            if (game_info.player_isre[p] == game_info.player_isre[0]) {
                result += game_info.player_scores[p];
            }
        }
        return result;
    }

    result = GetFromTable(&game_info, alpha, beta);
    if (result < 0xff) {
        return result;
    }

    /* get the legal cards */
    legal_cards_len = GetLegalCards(&game_info, legal_cards);

    if (game_info.player_isre[game_info.next] == game_info.player_isre[0]) {
        /* maximizing player */
        result = 0;
        for (uint8_t c = 0; c < legal_cards_len; c++) {
            Score s =
                AlphaBetaEvaluate(&game_info, legal_cards[c], alpha, beta);
            if (s > result) {
                result = s;
            }
            if (result >= beta) {
                /* beta cutoff */
                result = beta;
                break;
            }
            if (result > alpha) {
                alpha = result;
            }
        }
    } else {
        /* minimizing player */
        result = 0xff;
        for (uint8_t c = 0; c < legal_cards_len; c++) {
            Score s =
                AlphaBetaEvaluate(&game_info, legal_cards[c], alpha, beta);
            if (s < result) {
                result = s;
            }
            if (result <= alpha) {
                /* alpha cutoff */
                result = alpha;
                break;
            }
            if (result < beta) {
                beta = result;
            }
        }
    }

    StoreToTable(&game_info, result, alpha, beta);

    return result;
}
