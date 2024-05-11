#ifndef ISMCTS_H_INCLUDED
#define ISMCTS_H_INCLUDED

#include "random.h"
#include "simulate.h"

/** @defgroup ismcts ismcts
 *
 * Information Set Monte Carlo Tree Search
 *
 * The ismcts algorithm is a [monte carlo tree search algorithm (mcts)][1],
 * adapted to imperfect information games. Basically, it applys the standard
 * mcts search on multiple possible sampled realisations of the game state. More
 * detail can be found e.g. on [this website][2] or in the [original paper][3].
 *
 * This module provide a ismcts implementation specialized for the data types
 * and algorithms used in this project for calculating dopplekopf moves.
 *
 * [1]: https://en.wikipedia.org/wiki/Monte_Carlo_tree_search
 * [2]: https://www.aifactory.co.uk/newsletter/2013_01_reduce_burden.htm
 * [3]: https://eprints.whiterose.ac.uk/75048/1/CowlingPowleyWhitehouse2012.pdf
 *
 * @{
 */

struct doko_tree_node_t;
typedef struct doko_tree_node_t doko_tree_node_t;

/**
 * @brief allocate a new, empty ismcts tree
 *
 * The allocated tree must be free'd with @ref doko_ismcts_tree_free.
 */
doko_tree_node_t* doko_ismcts_tree_new();

/**
 * @brief free an ismcts tree
 */
void doko_ismcts_tree_free(doko_tree_node_t* node);

int doko_ismcts_tree_check(doko_tree_node_t const* root);

/**
 * @brief run a single ismcts simulation
 *
 * The game state passed in game_info should be a specific realisation of all
 * possible game states.
 *
 * The algorithm performs the following steps
 *
 *  1. _select_ a path in the tree based on a metric weighting good moves
 * against discovery with the ucb1 formula.
 *  2. _expand_ (add) a new node to the tree corresponding to a yet untried move
 *  3. _simulate_ a random game starting from that move
 *  4. _backpropagate_ the result of that random game to all the ancestor nodes
 *
 * @param root[in,out] one new node is added to the tree and all its parents are
 * updated according to the mcts rules
 * @param game_info[in] the sampled game state realisation to use for the run.
 * Will be modified during playthrough.
 * @param random_state[in] the random generator state to use
 */
void doko_ismcts_run(doko_tree_node_t* root, doko_game_info_t* game_info,
                     doko_random_state_t* random_state);

doko_card_t doko_ismcts_get_best_card(doko_tree_node_t* root, float* card_score);

/** @} */

#endif
