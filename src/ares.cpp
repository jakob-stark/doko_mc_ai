#include <protocol/interface.hpp>

#include "common.hpp"

extern "C" {
#include "simulate.h"
}

#include <random>

using namespace doko::protocol;
using namespace doko;

struct Ares : doko::protocol::BasicAgent {
    std::string get_name() final { return "ares"; }

    move_t get_move() final {
        std::array<doko_card_t, 12> legal_cards{};
        auto length = doko_get_legal_cards(&game_info, legal_cards.data());
        if (length <= 0 or length > 12) {
            throw rpc_exception{app_code_t::gameplay_error, "get_legal_cards"};
        }
        auto index = std::uniform_int_distribution<>{0, length - 1}(rnd);
        return {convert::card_r(legal_cards.at(index)), {}};
    };
};

int main() {
    Ares agent;
    doko::protocol::run(agent);
}
