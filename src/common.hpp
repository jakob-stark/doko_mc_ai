#ifndef COMMON_HPP_INCLUDED
#define COMMON_HPP_INCLUDED

#include <protocol/interface.hpp>

extern "C" {
#include "analysis.h"
#include "mc.h"
#include "random.h"
#include "simulate.h"
}

namespace doko::convert {

using namespace protocol;

inline doko_player_t player(player_t const& player) {
    switch (player) {
    case player1:
        return 0;
    case player2:
        return 1;
    case player3:
        return 2;
    case player4:
        return 3;
    }
    throw rpc_exception{app_code_t::conversion_failed, "invalid player_t"};
}

inline player_t player_r(doko_player_t player) {
    switch (player) {
    case 0:
        return player1;
    case 1:
        return player2;
    case 2:
        return player3;
    case 3:
        return player4;
    }
    throw rpc_exception{app_code_t::conversion_failed, "invalid PlayerId"};
}

inline doko_card_t card(card_t const& card) {
    switch (card.suit) {
    case diamond:
        switch (card.value) {
        case nine:
            return DIAMOND_NINE_L;
        case jack:
            return DIAMOND_JACK_L;
        case queen:
            return DIAMOND_QUEEN_L;
        case king:
            return DIAMOND_KING_L;
        case ten:
            return DIAMOND_TEN_L;
        case ace:
            return DIAMOND_ACE_L;
        }
    case heart:
        switch (card.value) {
        case nine:
            return HEART_NINE_L;
        case jack:
            return HEART_JACK_L;
        case queen:
            return HEART_QUEEN_L;
        case king:
            return HEART_KING_L;
        case ten:
            return HEART_TEN_L;
        case ace:
            return HEART_ACE_L;
        }
    case spade:
        switch (card.value) {
        case nine:
            return SPADE_NINE_L;
        case jack:
            return SPADE_JACK_L;
        case queen:
            return SPADE_QUEEN_L;
        case king:
            return SPADE_KING_L;
        case ten:
            return SPADE_TEN_L;
        case ace:
            return SPADE_ACE_L;
        }
    case club:
        switch (card.value) {
        case nine:
            return CLUB_NINE_L;
        case jack:
            return CLUB_JACK_L;
        case queen:
            return CLUB_QUEEN_L;
        case king:
            return CLUB_KING_L;
        case ten:
            return CLUB_TEN_L;
        case ace:
            return CLUB_ACE_L;
        }
    }
    throw rpc_exception{app_code_t::conversion_failed, "invalid card_t"};
}

inline card_t card_r(doko_card_t card) {
    switch (card) {
    case CLUB_NINE_L:
    case CLUB_NINE_H:
        return {club, nine};
    case CLUB_JACK_L:
    case CLUB_JACK_H:
        return {club, jack};
    case CLUB_QUEEN_L:
    case CLUB_QUEEN_H:
        return {club, queen};
    case CLUB_KING_L:
    case CLUB_KING_H:
        return {club, king};
    case CLUB_TEN_L:
    case CLUB_TEN_H:
        return {club, ten};
    case CLUB_ACE_L:
    case CLUB_ACE_H:
        return {club, ace};

    case SPADE_NINE_L:
    case SPADE_NINE_H:
        return {spade, nine};
    case SPADE_JACK_L:
    case SPADE_JACK_H:
        return {spade, jack};
    case SPADE_QUEEN_L:
    case SPADE_QUEEN_H:
        return {spade, queen};
    case SPADE_KING_L:
    case SPADE_KING_H:
        return {spade, king};
    case SPADE_TEN_L:
    case SPADE_TEN_H:
        return {spade, ten};
    case SPADE_ACE_L:
    case SPADE_ACE_H:
        return {spade, ace};

    case HEART_NINE_L:
    case HEART_NINE_H:
        return {heart, nine};
    case HEART_JACK_L:
    case HEART_JACK_H:
        return {heart, jack};
    case HEART_QUEEN_L:
    case HEART_QUEEN_H:
        return {heart, queen};
    case HEART_KING_L:
    case HEART_KING_H:
        return {heart, king};
    case HEART_TEN_L:
    case HEART_TEN_H:
        return {heart, ten};
    case HEART_ACE_L:
    case HEART_ACE_H:
        return {heart, ace};

    case DIAMOND_NINE_L:
    case DIAMOND_NINE_H:
        return {diamond, nine};
    case DIAMOND_JACK_L:
    case DIAMOND_JACK_H:
        return {diamond, jack};
    case DIAMOND_QUEEN_L:
    case DIAMOND_QUEEN_H:
        return {diamond, queen};
    case DIAMOND_KING_L:
    case DIAMOND_KING_H:
        return {diamond, king};
    case DIAMOND_TEN_L:
    case DIAMOND_TEN_H:
        return {diamond, ten};
    case DIAMOND_ACE_L:
    case DIAMOND_ACE_H:
        return {diamond, ace};
    }
    throw rpc_exception{app_code_t::conversion_failed, "invalid CardId"};
}

}; // namespace doko::convert

namespace doko::random {

struct RandomGenerator {
    static_assert(std::is_same_v<doko_random_state_t, uint32_t>);
    static_assert(std::is_same_v<doko_random_bits_t, uint32_t>);

    using result_type = uint32_t;

    static constexpr result_type max() { return UINT32_MAX; }
    static constexpr result_type min() { return 0; }

    result_type operator()() { return doko_random_bits(&state); }

    result_type state{1};
};

} // namespace doko::random

namespace doko::protocol {

struct BasicAgent : doko::protocol::agent_if {
    doko_player_t computer_player_id{};
    doko_player_t starting_player_id{};
    std::array<doko_card_t, 12> computer_player_cards{};

    doko_game_info_t game_info{};
    doko_card_info_t card_info{};

    random::RandomGenerator rnd{};

    void initialize(player_t computer_player, player_t starting_player,
                    std::array<card_t, 12> cards) final {
        computer_player_id = convert::player(computer_player);
        starting_player_id = convert::player(starting_player);

        for (auto i = 0; i < cards.size(); ++i) {
            computer_player_cards[i] = convert::card(cards[i]);
        }
    };

    game_t get_announcement() final { return {}; };

    void start(game_t game, player_t starting_player) final {
        if (game != normal) {
            throw rpc_exception{app_code_t::not_implemented,
                                "ares support only normal games"};
        }

        starting_player_id = convert::player(starting_player);
        if (doko_analysis_start(&game_info, &card_info, computer_player_id,
                                starting_player_id,
                                computer_player_cards.data()) != 0) {
            throw rpc_exception{app_code_t::gameplay_error, "analysis_start"};
        }
    };

    player_t do_move(player_t player, move_t move) final {
        if (doko_analysis_move(&game_info, &card_info, computer_player_id,
                               convert::player(player),
                               convert::card(move.card)) != 0) {
            throw rpc_exception{app_code_t::gameplay_error, "analysis_move"};
        }
        return convert::player_r((game_info.next + computer_player_id) % 4);
    };

    int finish() final {
        int points{};
        if (doko_analysis_finish(&game_info, &points) != 0) {
            throw rpc_exception{app_code_t::gameplay_error, "analysis_finish"};
        }
        return points;
    }
};

}; // namespace doko::protocol

#endif
