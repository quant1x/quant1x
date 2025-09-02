#pragma once
#ifndef QUANT1X_BACKTEST_TRADE_H
#define QUANT1X_BACKTEST_TRADE_H 1

#include <quant1x/engine/strategy.h>

namespace backtest {
    // 成交记录数据结构
    struct Trade {
        std::string    trade_id;    // 成交ID
        std::string    order_id;    // 关联订单ID
        std::string    symbol;      // 交易标的
        TradeDirection direction;   // 交易方向
        double         price;       // 成交价格
        double         quantity;    // 成交数量
        double         fee;         // 手续费
        std::string    trade_time;  // 成交时间
    };

} // namespace backtest

#endif //QUANT1X_BACKTEST_TRADE_H
