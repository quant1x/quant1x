// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_SHIFT_H
#define QUANT1X_NUM_SHIFT_H 1

/// 移位、差分与累计 —— 越界补 NaN, 累计顺序累加。

#include <quant1x/num/dtype.h>

#include <cstddef>
#include <cstdint>

namespace quant1x::num {

/// 向后移位: 取 N 周期前的值, 越界为 NaN
inline Series shift(const Series &values, std::int64_t periods) {
    Series out = default_like(values);
    if (periods == 0) {
        out = values;
    } else if (periods > 0) {
        const auto offset = static_cast<std::size_t>(periods);
        if (offset < values.size()) {
            for (std::size_t index = offset; index < values.size(); ++index) {
                out[index] = values[index - offset];
            }
        }
    } else {
        const auto offset = static_cast<std::size_t>(-periods);
        if (offset < values.size()) {
            for (std::size_t index = 0; index < values.size() - offset; ++index) {
                out[index] = values[index + offset];
            }
        }
    }
    return out;
}

/// 引用 N 周期前的值 (shift 的同义名)
inline Series ref(const Series &values, std::int64_t periods) { return shift(values, periods); }

/// 引用 N 周期前的值 (Rust 关键字规避名 ref_value 的对齐别名)
inline Series ref_value(const Series &values, std::int64_t periods) {
    return shift(values, periods);
}

/// N 周期差分
inline Series diff(const Series &values, std::int64_t periods) {
    const Series lagged = shift(values, periods);
    Series out(values.size());
    for (std::size_t index = 0; index < values.size(); ++index) {
        out[index] = values[index] - lagged[index];
    }
    return out;
}

/// 累计和: 从左到右顺序累加
inline Series cumsum(const Series &values) {
    Series out(values.size());
    double total = 0.0;
    for (std::size_t index = 0; index < values.size(); ++index) {
        total += values[index];
        out[index] = total;
    }
    return out;
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_SHIFT_H
