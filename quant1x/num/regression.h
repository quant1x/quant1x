// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_REGRESSION_H
#define QUANT1X_NUM_REGRESSION_H 1

/// 一元线性回归 —— 均值中心化最小二乘闭式解。

#include <quant1x/num/dtype.h>

#include <cstddef>
#include <utility>

namespace quant1x::num {

/// 返回窗口内 y = slope * t + intercept 的斜率与截距, t 取 0..n-1。
/// 窗口小于 2 或含 NaN 时返回 (NaN, NaN)。
inline std::pair<double, double> linear_regression(const Series &values) {
    const std::size_t count = values.size();
    if (count < 2) {
        return {nan_value(), nan_value()};
    }
    for (const double value : values) {
        if (is_nan(value)) {
            return {nan_value(), nan_value()};
        }
    }

    const double mean_t = static_cast<double>(count - 1) / 2.0;
    double total = 0.0;
    for (const double value : values) {
        total += value;
    }
    const double mean_y = total / static_cast<double>(count);

    double numerator = 0.0;
    double denominator = 0.0;
    for (std::size_t index = 0; index < count; ++index) {
        const double offset = static_cast<double>(index) - mean_t;
        numerator += offset * (values[index] - mean_y);
        denominator += offset * offset;
    }
    const double slope = numerator / denominator;
    const double intercept = mean_y - slope * mean_t;
    return {slope, intercept};
}

/// 回归斜率
inline double slope(const Series &values) { return linear_regression(values).first; }

/// 回归线在最后一个点上的预测值
inline double forcast(const Series &values) {
    const auto result = linear_regression(values);
    if (is_nan(result.first)) {
        return nan_value();
    }
    return result.second + result.first * (static_cast<double>(values.size()) - 1.0);
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_REGRESSION_H
