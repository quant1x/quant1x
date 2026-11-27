// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_DTYPE_H
#define QUANT1X_NUM_DTYPE_H 1

/// 类型与缺失值 —— float64 是唯一数值类型, NaN 是唯一缺失标记。
/// 口径见 quant1x/num/num.md; 与 Python spec 及 Rust 端口同语义。

#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace quant1x::num {

/// 一维 float64 序列
using Series = std::vector<double>;

/// 一维布尔序列
using Bools = std::vector<bool>;

/// 缺失值
inline double nan_value() { return std::numeric_limits<double>::quiet_NaN(); }

/// 判断缺失
inline bool is_nan(double value) { return std::isnan(value); }

/// 判断真值: 非零且非 NaN
inline bool is_true(double value) { return !std::isnan(value) && value != 0.0; }

/// 生成与输入等长的全 NaN 序列
inline Series default_like(const Series &values) {
    return Series(values.size(), nan_value());
}

/// 取最后一个值; 空序列返回 NaN
inline double to_scalar(const Series &values) {
    return values.empty() ? nan_value() : values.back();
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_DTYPE_H
