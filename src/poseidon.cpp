#include <protocol/interface.hpp>

#include "common.hpp"

extern "C" {
#include "mc.h"
#include "simulate.h"
}

#include <iostream>

using namespace doko::protocol;
using namespace doko;

struct Poseidon : doko::protocol::BasicAgent {
    std::string get_name() final { return "poseidon"; }

    move_t get_move() final {
        std::array<CardId, 12> legal_cards{};
        auto length = GetLegalCards(&game_info, legal_cards.data());
        if (length <= 0 or length > 12) {
            throw rpc_exception{app_code_t::gameplay_error, "get_legal_cards"};
        }

        if ( sort_and_check(&card_info) != 0 ) {
            throw rpc_exception{app_code_t::gameplay_error, "sort_and_check"};
        }

        std::array<unsigned, 12> results{};
        unsigned mc_calls{};

        uint32_t random_state{1};
        for ( int n = 0; n < 10000; n++ ) {
            GameInfo game_info_copy = game_info;

            mc_calls++;
            mc_sample(&game_info_copy, &card_info, &random_state);

            for ( uint8_t c = 0; c < length; c++ ) {
                results[c] += Simulate(&game_info_copy, legal_cards[c], &random_state);
            }
        }

        int64_t max_score = -1;
        CardId max_card = 0;

        for ( uint8_t c = 0; c < length; c++ ) {
            if ( results[c] > max_score ) {
                max_score = results[c];
                max_card = legal_cards[c];
            }
        }

        std::cerr << double(max_score)/double(mc_calls) << ' ' << card_names_long[max_card / 2] << '\n';

        return {convert::card_r(max_card), {}};
    };
};

int main() {
    Poseidon agent;
    doko::protocol::run(agent);
}
