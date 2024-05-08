#include <assert.h>
#include <math.h>

#include "simulate.h"

struct doko_tree_node_t {
    struct doko_tree_node_t* parent;
    struct doko_tree_node_t* first_child;
    struct doko_tree_node_t* next_sibling;

    doko_cardset_t child_cardset;

    /** count how often the node was choosen for a playout */
    unsigned playouts;

    /** count how often the node was available to be chosen */
    unsigned available;

    /** count how many points the *current player* at this node has made through
     * all playouts*/
    unsigned score;

    /** the card that was played to get here */
    doko_card_t card;

    /** the player that play's the next card */
    doko_player_t next_player;
};
typedef struct doko_tree_node_t doko_tree_node_t;

#define DOKO_TREE_FOR_EACH_CHILD(x, node)                                      \
    for (doko_tree_node_t* x = (node)->first_child;                            \
         x->next_sibling != nullptr; x = x->next_sibling)

static doko_tree_node_t* tree_allocate_node();
static void tree_free_node(doko_tree_node_t* node);

static float ucb1(float playouts, float available, float score,
                  doko_random_state_t* random_state) {
    float const scale = 0.7f * 240.0f;
    return score / playouts + scale * sqrtf(logf(available) / playouts) +
           doko_random_float(random_state);
}

/** @brief select a child node based on ucb1 */
void doko_select_and_expand(doko_tree_node_t* root,
                                         doko_game_info_t* game_info,
                                         doko_random_state_t* random_state) {
    for (doko_tree_node_t* node = root;;) {
        /* increase the playout counter */
        node->playouts++;

        /* find legal moves and check if any of them are still untried, that is
         * no child node exists for them */
        doko_cardset_t legal_cardset = doko_get_legal_cardset(game_info);
        doko_cardset_t untried_cardset =
            DOKO_CARDSET_DIFFERENCE(legal_cardset, node->child_cardset);
        if (untried_cardset != 0) {
            /* mcts expansion */
            doko_card_t cards[12];
            doko_count_t cards_len =
                doko_cardset_to_array(untried_cardset, cards);

            doko_card_t chosen_card =
                cards[doko_random_uint8(random_state, cards_len)];

            /* create new node */
            doko_tree_node_t* new_node = tree_allocate_node();
            assert(new_node != nullptr);

            /* play out the card */
            doko_play_card(game_info, chosen_card);

            /* initialize the new node */
            new_node->parent = node;
            new_node->next_sibling = node->first_child;
            new_node->first_child = nullptr;
            new_node->child_cardset = 0ul;
            new_node->playouts = 1;
            new_node->available = 1;
            new_node->score = 0;
            new_node->card = chosen_card;
            new_node->next_player = game_info->next;

            /* insert the new node as the first child of the current node */
            node->first_child = new_node;
            node->child_cardset |= DOKO_CARDSHIFT(chosen_card);

            /* mcts simulation */
            doko_simulate_v2(game_info, random_state);

            /* mcts backpropagation */

            /* calculate the scores */
            doko_score_t scores[2] = {0, 0};
            DOKO_FOR_EACH_PLAYER(p) {
                scores[game_info->player_isre[p]] +=
                    game_info->player_scores[p];
            }

            /* backpropagate */
            for ( node = new_node; node->parent != nullptr; node = node->parent ) {
                node->score += scores[game_info->player_isre[node->next_player]];
            }

            return;
        }

        /* select the next child based on the maximal ucb1 value */
        float max_value = -1.0f;
        DOKO_TREE_FOR_EACH_CHILD(child, node) {
            if (DOKO_CARDSET_CONTAINS(legal_cardset, child->card)) {
                /* update the available counter right away */
                child->available++;

                /* calculate the metric and keep if the value is higher than any
                 * previous ones */
                float ucb1_value = ucb1(child->playouts, child->available,
                                        child->score, &random_state);
                if (max_value < ucb1_value) {
                    max_value = ucb1_value;
                    node = child;
                }
            }
        }

        assert(max_value > -1.0f);
        assert(node != nullptr);

        /* play out the move */
        doko_play_card(game_info, node->card);
    }
}

void doko_ismcts(doko_tree_node_t* root, doko_game_info_t* game_info,
                 doko_random_state_t* random_state) {
    /* selection */
    doko_tree_node_t* node = doko_ucb1_select(root, game_info, random_state);

    /* expansion */
}
