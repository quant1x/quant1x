#pragma once

#include <string>
#include <cstdint>
#include <stdexcept>

namespace exchange {

static inline const char* EXCHANGE_SSE = "sh";
static inline const char* EXCHANGE_SZSE = "sz";
static inline const char* EXCHANGE_BJSE = "bj";
static inline const char* EXCHANGE_HK = "hk";
static inline const char* EXCHANGE_US = "us";

enum class ExchangeId : std::uint8_t {
    ShenZhen = 0,
    ShangHai = 1,
    BeiJing = 2,
    HongKong = 21,
    USA = 22,
};

inline const char* to_code(ExchangeId id) {
    switch (id) {
    case ExchangeId::ShenZhen: return EXCHANGE_SZSE;
    case ExchangeId::ShangHai: return EXCHANGE_SSE;
    case ExchangeId::BeiJing: return EXCHANGE_BJSE;
    case ExchangeId::HongKong: return EXCHANGE_HK;
    case ExchangeId::USA: return EXCHANGE_US;
    default: throw std::runtime_error("unknown market id");
    }
}

struct ExchangeInfo {
    ExchangeId id;
    std::string code;
    std::string name;
    std::string description;
    bool is_active = true;

    std::string ToString() const;
    void Validate() const;
    static ExchangeInfo NewExchange(const std::string& code,
                                    const std::string& name,
                                    const std::string& desc,
                                    ExchangeId id);
};

struct SecurityCode {
    ExchangeId market;
    std::string symbol;

    std::string ToString() const;
    void Validate() const;
};

} // namespace exchange
