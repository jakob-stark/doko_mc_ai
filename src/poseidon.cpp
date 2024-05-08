#include <protocol/interface.hpp>

#include "common.hpp"

extern "C" {
#include "mc.h"
#include "simulate.h"
}

#include <chrono>
#include <iostream>

using namespace doko::protocol;
using namespace doko;

struct Poseidon : doko::protocol::BasicAgent {
    std::string get_name() final { return "poseidon"; }

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

        std::array<unsigned, 12> results{};
        unsigned mc_calls{};

        doko_random_state_t random_state{1};
        for (auto start = clock::now(); clock::now() - start < 1000ms;) {
            doko_game_info_t game_info_copy = game_info;

            mc_calls++;
            doko_mc_sample(&game_info_copy, &card_info, &random_state);

            for (uint8_t c = 0; c < length; c++) {
                results[c] += doko_simulate(&game_info_copy, legal_cards[c],
                                            &random_state);
            }
        }

        int64_t max_score = -1;
        doko_card_t max_card = 0;

        for (uint8_t c = 0; c < length; c++) {
            if (results[c] > max_score) {
                max_score = results[c];
                max_card = legal_cards[c];
            }
        }

        std::cerr << "chosen '" << doko_card_names_long[max_card / 2]
                  << "' with an expected score of "
                  << double(max_score) / double(mc_calls) << " after "
                  << mc_calls << " samplings\n";

        return {convert::card_r(max_card), {}};
    };
};

int main() {
    Poseidon agent;
    doko::protocol::run(agent);
}
