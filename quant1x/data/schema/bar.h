#pragma once
#ifndef QUANT1X_DATA_SCHEMA_BAR_H
#define QUANT1X_DATA_SCHEMA_BAR_H 1

#include "adjustment.h"
#include <string>
#include <vector>
#include <algorithm>

namespace meta::schema {

/// K线数据结构体
struct Bar {
    std::string date;                   ///< 日期 YYYY-MM-DD
    double      open = 0.0;             ///< 开盘价
    double      close = 0.0;            ///< 收盘价
    double      high = 0.0;             ///< 最高价
    double      low = 0.0;              ///< 最低价
    double      volume = 0.0;           ///< 成交量
    double      amount = 0.0;           ///< 成交额
    int         up = 0;                 ///< 上涨家数 (仅指数)
    int         down = 0;               ///< 下跌家数 (仅指数)
    std::string timestamp;              ///< 时间戳 YYYY-MM-DD HH:MM:SS
    int         adjustment_count = 0;   ///< 复权次数

    /// 涨跌额
    double change() const { return close - open; }

    /// 涨跌幅 (百分比)
    double change_pct() const {
        if (open == 0.0) return 0.0;
        return (close - open) / open * 100.0;
    }

    /// 是否阳线
    bool is_positive() const { return close > open; }

    /// 是否阴线
    bool is_negative() const { return close < open; }

    /// K线实体大小
    double body_size() const { return std::abs(close - open); }

    /// 上影线长度
    double upper_shadow() const { return high - std::max(open, close); }

    /// 下影线长度
    double lower_shadow() const { return std::min(open, close) - low; }

    /// 均价
    double avg_price() const {
        if (volume == 0.0) return 0.0;
        return amount / volume;
    }

    /// 价格区间
    double price_range() const { return high - low; }

    /// 复权
    void adjust(const CumulativeAdjustment& adj) {
        open   = open  * adj.m + adj.a;
        close  = close * adj.m + adj.a;
        high   = high  * adj.m + adj.a;
        low    = low   * adj.m + adj.a;

        if (volume != 0.0) {
            double ap = amount / volume;
            double ap_adjusted = ap * adj.m + adj.a;
            volume *= (1.0 + adj.share_adjustment_ratio);
            amount = volume * ap_adjusted;
        }

        adjustment_count = adj.no;
    }

    static std::vector<std::string> headers() {
        return {"date", "open", "close", "high", "low", "volume", "amount",
                "up", "down", "timestamp", "adjustment_count"};
    }
};

} // namespace meta::schema

#endif // QUANT1X_DATA_SCHEMA_BAR_H
