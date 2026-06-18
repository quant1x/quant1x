#pragma once
#ifndef QUANT1X_DATA_KLINE_H
#define QUANT1X_DATA_KLINE_H 1

#include <string>

namespace quant1x::data {

/// K线数据 (对齐 Python contrib/data/tdx/kline.py 和 Rust q1x/src/data/tdx.rs)
struct KLine {
    std::string date;    ///< 日期 "YYYY-MM-DD"
    std::string code;    ///< 证券代码
    double      open  = 0.0;  ///< 开盘价
    double      close = 0.0;  ///< 收盘价
    double      high  = 0.0;  ///< 最高价
    double      low   = 0.0;  ///< 最低价
    double      volume = 0.0; ///< 成交量
    double      amount = 0.0; ///< 成交金额
};

} // namespace quant1x::data

#endif // QUANT1X_DATA_KLINE_H
