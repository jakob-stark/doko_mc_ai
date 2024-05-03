#include "interface.hpp"

#include <boost/describe.hpp>
#include <boost/json.hpp>

#include <iostream>

namespace doko::protocol {

namespace json = boost::json;

namespace {

std::optional<std::string> read_line() {
    std::string line;
    std::getline(std::cin, line);
    if (std::cin.eof()) {
        return {};
    }
    return line;
}

struct rpc_exception {
    int code;
    std::string message;
    std::string data;

    void write_error(json::value const& id = nullptr) const {
        std::cerr << "error: " << message << ": " << data << '\n';
        std::cout << boost::json::value{
            {"jsonrpc", "2.0"},
            {"id", id},
            {"error", code},
            {"message", message},
            {"data", data},
        } << '\n';
    }
};

void write_result(json::value&& result, json::value const& id) {
    std::cout << json::value{
        {"jsonrpc", "2.0"},
        {"id", id},
        {"result", result},
    } << '\n';
}

json::object parse(std::string const& line) {
    boost::system::error_code ec{};
    auto input = boost::json::parse(line, ec);
    if (ec) {
        throw rpc_exception{-32700, "parse error", ec.message()};
    }

    if (not input.is_object()) {
        throw rpc_exception{-32600, "invalid request", "not a json object"};
    }

    return std::move(input).get_object();
}

void check_jsonrpc(json::object const& input) {
    auto const* jsonrpc = input.if_contains("jsonrpc");
    if (jsonrpc == nullptr) {
        throw rpc_exception{-32600, "invalid request", "jsonrpc field missing"};
    }
    if (*jsonrpc != "2.0") {
        throw rpc_exception{-32600, "invalid request",
                            "jsonrpc must be \"2.0\""};
    }
}

json::value check_id(json::object const& input) {
    auto const* id = input.if_contains("id");
    if (id == nullptr) {
        std::cerr << "warning: ignoring notification" << '\n';
    }
    if (id->is_object() or id->is_array()) {
        throw rpc_exception{-32600, "invalid request",
                            "id must be string, number, or null"};
    }
    return *id;
}

std::string get_method(json::object const& input) {
    auto const* method = input.if_contains("method");
    if (method == nullptr) {
        throw rpc_exception{-32600, "invalid request", "method field missing"};
    }
    if (not method->is_string()) {
        throw rpc_exception{-32600, "invalid request", "method must be string"};
    }
    return json::value_to<std::string>(*method);
}

template <typename T>
T get_param(json::object const& input, std::string_view key) {
    auto const* params = input.if_contains("params");
    if (params == nullptr) {
        throw rpc_exception{-32602, "invalid params", "params field missing"};
    }
    if (not params->is_object()) {
        throw rpc_exception{-32602, "invalid params", "params must be object"};
    }
    auto const* param = params->get_object().if_contains(key);
    if (param == nullptr) {
        throw rpc_exception{-32602, "invalid params", "parameter missing"};
    }
    try {
        return json::value_to<T>(*param);
    } catch (...) {
        throw rpc_exception{-32602, "invalid params", "parameter wrong type"};
    }
}

} // namespace

BOOST_DESCRIBE_ENUM(player_t, player1, player2, player3, player4)
BOOST_DESCRIBE_ENUM(suit_t, diamond, heart, spade, club)
BOOST_DESCRIBE_ENUM(value_t, nine, jack, queen, king, ten, ace)
BOOST_DESCRIBE_ENUM(call_t, none, re, contra, not90, not60, not30)
BOOST_DESCRIBE_ENUM(game_t, normal, dismiss, wedding, poverty, grand_solo,
                    king_solo, queen_solo, jack_solo, diamond_solo, heart_solo,
                    spade_solo, club_solo)

BOOST_DESCRIBE_STRUCT(card_t, (), (suit, value))
BOOST_DESCRIBE_STRUCT(move_t, (), (card, calls))

calls_t tag_invoke(json::value_to_tag<calls_t> /*tag*/, json::value const& v) {
    unsigned calls{};
    for (auto const& call : v.as_array()) {
        calls |= json::value_to<call_t>(call);
    }
    return {calls};
}

void tag_invoke(json::value_from_tag /*tag*/, json::value& v, calls_t calls) {
    json::array r{};
    boost::mp11::mp_for_each<boost::describe::describe_enumerators<call_t>>(
        [&](auto d) {
            if (d.value & calls.flags) {
                r.push_back(d.name);
            }
        });
    v = std::move(r);
}

int run(Client& client) {
    for (;;) {
        json::value jsonrpc_id = nullptr;
        try {
            auto line = read_line();
            if (not line) {
                std::cerr << "eof: shutdown\n";
                break;
            }

            auto input = parse(*line);
            check_jsonrpc(input);
            jsonrpc_id = check_id(input);

            auto method = get_method(input);

            json::value result = nullptr;
            if (method == "initialize") {
                client.initialize(
                    get_param<player_t>(input, "computer_player"),
                    get_param<player_t>(input, "starting_player"),
                    get_param<std::array<card_t, 10>>(input, "cards"));
            } else if (method == "get_announcement") {
                result = json::value_from(client.get_announcement());
            } else if (method == "start") {
                client.start(get_param<game_t>(input, "game"),
                             get_param<player_t>(input, "starting_player"));
            } else if (method == "do_move") {
                client.do_move(get_param<player_t>(input, "player"),
                               get_param<move_t>(input, "move"));
            } else if (method == "get_move") {
                result = json::value_from(client.get_move());
            } else {
                throw rpc_exception{-32601, "method not found", method};
            }
            write_result(std::move(result), jsonrpc_id);
        } catch (rpc_exception const& ex) {
            ex.write_error(jsonrpc_id);
        }
    }
    return 0;
}

} // namespace doko::protocol
