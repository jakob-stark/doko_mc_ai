#include "error.hpp"
#include "interface.hpp"

#include <boost/describe.hpp>

namespace {

template <typename E> struct described_category_t : std::error_category {
    char const* name() const noexcept override { return "described"; }

    std::string message(int ev) const override {
        std::string result = "(invalid error code)";
        boost::mp11::mp_for_each<boost::describe::describe_enumerators<E>>(
            [&](auto d) {
                if (ev == static_cast<int>(d.value)) {
                    result = d.name;
                }
            });
        return std::move(result);
    }
};

} // namespace

namespace doko::protocol {

BOOST_DESCRIBE_ENUM(rpc_code_t, parse_error, invalid_request, method_not_found,
                    invalid_params)

std::error_code make_error_code(rpc_code_t ev) {
    static described_category_t<rpc_code_t> const category{};
    return {static_cast<int>(ev), category};
}

BOOST_DESCRIBE_ENUM(app_code_t, not_implemented, gameplay_error, conversion_failed)

std::error_code make_error_code(app_code_t ev) {
    static described_category_t<app_code_t> const category{};
    return {static_cast<int>(ev), category};
}

} // namespace doko::protocol
