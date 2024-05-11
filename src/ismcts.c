#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "ismcts.h"
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
    doko_player_t player;
};

#define DOKO_TREE_FOR_EACH_CHILD(x, node)                                      \
    for (doko_tree_node_t* x = (node)->first_child; x != nullptr;              \
         x = x->next_sibling)

static doko_tree_node_t* tree_allocate_node() {
    return malloc(sizeof(doko_tree_node_t));
}

static void tree_free_node(doko_tree_node_t* node) { free(node); }

static float ucb1(float playouts, float available, float score,
                  doko_random_state_t* random_state) {
    float const scale = 0.7f * 240.0f;
    return score / playouts + scale * sqrtf(logf(available) / playouts) +
           doko_random_float(random_state);
}

static void tree_free_children(doko_tree_node_t* node) {
    for (doko_tree_node_t* child = node->first_child; child != nullptr;
         child = node->first_child) {
        node->first_child = child->next_sibling;
        tree_free_children(child);
        tree_free_node(child);
    }
}

doko_tree_node_t* doko_ismcts_tree_new() {
    doko_tree_node_t* node = tree_allocate_node();
    node->parent = nullptr;
    node->first_child = nullptr;
    node->next_sibling = nullptr;
    node->child_cardset = 0ul;
    node->playouts = 0;
    node->available = 0;
    node->score = 0;
    node->card = INVALID;
    node->player = 0;
    return node;
}

void doko_ismcts_tree_free(doko_tree_node_t* root) {
    tree_free_children(root);
    tree_free_node(root);
}

int doko_ismcts_tree_check(doko_tree_node_t const* root) {
    int child_count = 1;
    doko_cardset_t child_cardset = 0ul;
    DOKO_TREE_FOR_EACH_CHILD(child, root) {
        child_cardset |= DOKO_CARDSHIFT(child->card);
        child_count += doko_ismcts_tree_check(child);
    }
    if (child_cardset != root->child_cardset) {
        fprintf(stderr, "%lx != %lx\n", child_cardset, root->child_cardset);
    }
    assert(child_cardset == root->child_cardset);
    return child_count;
}

void doko_ismcts_run(doko_tree_node_t* root, doko_game_info_t* game_info,
                     doko_random_state_t* random_state) {
    doko_tree_node_t* node = root;
    doko_cardset_t untried_cardset = 0ul;
    for (;;) {
        /* increase the playout counter */
        node->playouts++;

        /* find legal moves and check if any of them are still untried, that is
         * no child node exists for them */
        doko_cardset_t legal_cardset = doko_get_legal_cardset(game_info);
        untried_cardset =
            DOKO_CARDSET_DIFFERENCE(legal_cardset, node->child_cardset);
        if (untried_cardset != 0) {
            break;
        }

        if (legal_cardset == 0 ) {
            /* mcts backpropagation */

            /* calculate the scores */
            doko_score_t scores[2] = {0, 0};
            DOKO_FOR_EACH_PLAYER(p) {
                scores[game_info->player_isre[p]] += game_info->player_scores[p];
            }

            /* backpropagate */
            for (; node->parent != nullptr; node = node->parent) {
                node->score += scores[game_info->player_isre[node->player]];
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
                                        child->score, random_state);
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

    /* select a random untried cardset */
    doko_card_t cards[12];
    doko_count_t cards_len = doko_cardset_to_array(untried_cardset, cards);
    doko_card_t chosen_card = cards[doko_random_uint8(random_state, cards_len)];

    /* create new node */
    doko_tree_node_t* new_node = tree_allocate_node();
    assert(new_node != nullptr);

    /* initialize the new node */
    new_node->parent = node;
    new_node->next_sibling = node->first_child;
    new_node->first_child = nullptr;
    new_node->child_cardset = 0ul;
    new_node->playouts = 1;
    new_node->available = 1;
    new_node->score = 0;
    new_node->card = chosen_card;
    new_node->player = game_info->next;

    /* insert the new node as the first child of the current node */
    node->first_child = new_node;
    node->child_cardset |= DOKO_CARDSHIFT(chosen_card);

    /* play out the card */
    doko_play_card(game_info, chosen_card);

    /* mcts simulation */
    doko_simulate_v2(game_info, random_state);

    /* mcts backpropagation */

    /* calculate the scores */
    doko_score_t scores[2] = {0, 0};
    DOKO_FOR_EACH_PLAYER(p) {
        scores[game_info->player_isre[p]] += game_info->player_scores[p];
    }

    /* backpropagate */
    for (node = new_node; node->parent != nullptr; node = node->parent) {
        node->score += scores[game_info->player_isre[node->player]];
    }
}

doko_card_t doko_ismcts_get_best_card(doko_tree_node_t* root,
                                      float* card_score) {
    float max_score = -1.0f;
    unsigned max_playouts = 0;
    doko_card_t max_card = INVALID;
    DOKO_TREE_FOR_EACH_CHILD(child, root) {
        if (child->playouts >= max_playouts) {
            max_playouts = child->playouts;
            max_score = (float)child->score / (float)child->playouts;
            max_card = child->card;
        }
    }
    if (card_score != nullptr) {
        *card_score = max_score;
    }
    return max_card;
}
