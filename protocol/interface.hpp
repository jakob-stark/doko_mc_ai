#ifndef INTEFACE_HPP_INCLUDED
#define INTEFACE_HPP_INCLUDED

#include <array>
#include <string>
#include <system_error>
#include <utility>

namespace doko::protocol {

enum player_t {
    player1,
    player2,
    player3,
    player4,
};

enum suit_t {
    diamond,
    heart,
    spade,
    club,
};

enum value_t {
    nine,
    jack,
    queen,
    king,
    ten,
    ace,
};

enum call_t {
    none = 0,
    re = 1,
    contra = 2,
    not90 = 8,
    not60 = 16,
    not30 = 32,
};

struct calls_t {
    unsigned flags;
};

enum game_t {
    normal = 0,
    dismiss,
    wedding,
    poverty,
    grand_solo,
    king_solo,
    queen_solo,
    jack_solo,
    diamond_solo,
    heart_solo,
    spade_solo,
    club_solo,
};

struct card_t {
    suit_t suit;
    value_t value;
};

struct move_t {
    card_t card;
    calls_t calls;
};

} // namespace doko::protocol

namespace doko::protocol {

struct agent_if {
    virtual void initialize(player_t computer_player, player_t starting_player,
                            std::array<card_t, 12> cards) = 0;

    virtual game_t get_announcement() = 0;
    virtual void start(game_t game, player_t starting_player) = 0;

    virtual player_t do_move(player_t player, move_t move) = 0;
    virtual move_t get_move() = 0;

  protected:
    ~agent_if() = default;
};

struct rpc_exception {
    std::error_code ec;
    std::string data;

    rpc_exception(std::error_code ec, std::string data)
        : ec{ec}, data{std::move(data)} {}
};

enum class app_code_t {
    not_implemented = -20000,
    gameplay_error = -20100,
};

std::error_code make_error_code(app_code_t ev);

int run(agent_if& client);

} // namespace doko::protocol

namespace std {
template <>
struct is_error_code_enum<doko::protocol::app_code_t> : true_type {};

} // namespace std

#endif
