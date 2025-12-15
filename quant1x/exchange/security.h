#pragma once
#ifndef QUANT1X_EXCHANGE_SECURITY_H
#define QUANT1X_EXCHANGE_SECURITY_H 1

#include <quant1x/instruments/security.h>

namespace exchange {

    using SecurityInfo = instruments::SecurityInfo;

    inline std::optional<SecurityInfo> get_security_info(const std::string &code) {
        return instruments::get_security_info(code);
    }

    inline double get_up_limit_rate(const std::string& security_code) {
        return instruments::get_up_limit_rate(security_code);
    }

    inline double calc_limit_up_price(const std::string& security_code, double prev_close) {
        return instruments::calc_limit_up_price(security_code, prev_close);
    }

}

#endif // QUANT1X_EXCHANGE_SECURITY_H
