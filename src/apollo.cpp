#include <protocol/interface.hpp>

#include "common.hpp"

extern "C" {
#include "ismcts.h"
#include "mc.h"
#include "simulate.h"
}

#include <chrono>
#include <iostream>

using namespace doko::protocol;
using namespace doko;

struct Apollo : doko::protocol::BasicAgent {
    std::string get_name() final { return "apollo"; }

    move_t get_move() final {
        std::array<doko_card_t, 12> legal_cards{};
        auto length = doko_get_legal_cards(&game_info, legal_cards.data());
        if (length <= 0 or length > 12) {
            throw rpc_exception{app_code_t::gameplay_error, "get_legal_cards"};
        }

        if (doko_sort_and_check(&card_info) != 0) {
            throw rpc_exception{app_code_t::gameplay_error, "sort_and_check"};
        }

        using clock = std::chrono::steady_clock;
        using std::chrono_literals::operator""ms;

        unsigned mc_calls{};
        doko_random_state_t random_state{1};
        doko_tree_node_t* tree = doko_ismcts_tree_new();

        for (auto start = clock::now(); clock::now() - start < 100ms;) {
            doko_game_info_t game_info_copy = game_info;

            mc_calls++;
            doko_mc_sample(&game_info_copy, &card_info, &random_state);
            doko_ismcts_run(tree, &game_info_copy, &random_state);
        }

        float max_score = 0.0;
        doko_card_t max_card = INVALID;

        auto nnodes = doko_ismcts_tree_check(tree);
        max_card = doko_ismcts_get_best_card(tree, &max_score);

        doko_ismcts_tree_free(tree);

        if (not DOKO_CARD_VALID(max_card)) {
            throw rpc_exception{app_code_t::conversion_failed,
                                "ismct_get_best_card"};
        }

        std::cerr << "chosen '" << doko_card_names_long[max_card / 2]
                  << "' with an expected score of " << max_score << " after "
                  << mc_calls << " samplings and creating " << nnodes
                  << " nodes\n";

        return {convert::card_r(max_card), {}};
    };
};

int main() {
    Apollo agent;
    doko::protocol::run(agent);
}
