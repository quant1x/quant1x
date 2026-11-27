// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_DECIMALS_H
#define QUANT1X_NUM_DECIMALS_H 1

/// 小数位与变化率 —— 半程远离零, NaN 按 0 参与舍入。

#include <quant1x/num/dtype.h>

#include <cmath>
#include <cstddef>

namespace quant1x::num {

/// 单值保留小数位 (半程远离零, NaN 按 0 参与舍入)
inline double decimal(double value, int digits = 2) {
    const int places = digits > 0 ? digits : 0;
    const double input = is_nan(value) ? 0.0 : value;
    const double half = std::signbit(input) ? -5.0 : 5.0;
    const double scale = std::pow(10.0, places);
    const double wider = std::pow(10.0, places + 1);
    return std::trunc((input * wider + half) / 10.0) / scale;
}

/// 对序列逐元素保留小数位
inline Series decimal_series(const Series &values, int digits = 2) {
    Series out(values.size());
    for (std::size_t index = 0; index < values.size(); ++index) {
        out[index] = decimal(values[index], digits);
    }
    return out;
}

/// 变化率 current / base
inline double change_rate(double base, double current) { return current / base; }

/// 净变化率百分比 (current / base - 1) * 100
inline double net_change_rate(double base, double current) {
    return (change_rate(base, current) - 1.0) * 100.0;
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_DECIMALS_H
