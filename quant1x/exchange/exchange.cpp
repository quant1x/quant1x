#include "exchange.h"
#include <fmt/format.h>

namespace exchange {

std::string ExchangeInfo::ToString() const {
    return fmt::format("{}({})", name, code);
}

void ExchangeInfo::Validate() const {
    if (code.empty()) throw std::invalid_argument("exchange code cannot be empty");
    if (name.empty()) throw std::invalid_argument("exchange name cannot be empty");
}

ExchangeInfo ExchangeInfo::NewExchange(const std::string& code,
                                       const std::string& name,
                                       const std::string& desc,
                                       ExchangeId id) {
    ExchangeInfo e;
    e.code = code;
    e.name = name;
    e.description = desc;
    e.id = id;
    e.is_active = true;
    return e;
}

std::string SecurityCode::ToString() const {
    return fmt::format("{}{}", to_code(market), symbol);
}

void SecurityCode::Validate() const {
    if (symbol.empty()) throw std::invalid_argument("security code symbol cannot be empty");
}

} // namespace exchange
