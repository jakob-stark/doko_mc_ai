#ifndef ERROR_HPP_INCLUDED
#define ERROR_HPP_INCLUDED

#include <system_error>

namespace doko::protocol {

enum class rpc_code_t {
    parse_error = -32600,
    invalid_request = -32600,
    method_not_found = -32601,
    invalid_params = -32602,
};

std::error_code make_error_code(rpc_code_t ev);

} // namespace doko::protocol

namespace std {
template <>
struct is_error_code_enum<doko::protocol::rpc_code_t> : true_type {};
} // namespace std

#endif
