#pragma once
#ifndef QUANT1X_DATA_SCHEMA_TRADE_H
#define QUANT1X_DATA_SCHEMA_TRADE_H 1

#include <cstdint>
#include <string>
#include <vector>

namespace meta::schema {

/// 交易方向
enum class Direction : uint8_t {
    BUY     = 0, ///< 主动买入
    SELL    = 1, ///< 主动卖出
    NEUTRAL = 2, ///< 中性盘
};

/// 逐笔交易数据结构体
struct Transaction {
    std::string time;           ///< 时间
    double      price = 0.0;    ///< 价格
    int         volume = 0;     ///< 成交量
    int         num = 0;        ///< 成交笔数
    double      amount = 0.0;   ///< 成交额
    int         direction = 2;  ///< 交易方向 Direction

    static std::vector<std::string> headers() {
        return {"time", "price", "volume", "num", "amount", "direction"};
    }
};

} // namespace meta::schema

#endif // QUANT1X_DATA_SCHEMA_TRADE_H
