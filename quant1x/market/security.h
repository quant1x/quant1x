#pragma once
#ifndef QUANT1X_INSTRUMENTS_SECURITY_H
#define QUANT1X_INSTRUMENTS_SECURITY_H 1

//============================================================
// instruments 证券信息相关                                   //
//============================================================
#include <quant1x/std/api.h>

namespace instruments {

    /// 证券信息
    struct SecurityInfo {
        std::string code; ///< 证券代码
        std::string name; ///< 证券名称
        uint16_t lotSize = 100; ///< 每手股数
        uint8_t pricePrecision = 2; ///< 股价保持小数点后几位
        friend std::ostream& operator<<(std::ostream& os, const SecurityInfo& p);
    };

    std::optional<SecurityInfo> get_instrument_info(const std::string &code);

    double get_up_limit_rate(const std::string& security_code);
    double calc_limit_up_price(const std::string& security_code, double prev_close);

}

#endif // QUANT1X_INSTRUMENTS_SECURITY_H
