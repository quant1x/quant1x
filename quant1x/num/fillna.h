// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_FILLNA_H
#define QUANT1X_NUM_FILLNA_H 1

/// 缺失值填充 —— 返回新序列, 不原地修改。

#include <quant1x/num/dtype.h>

#include <cstddef>

namespace quant1x::num {

/// 把 NaN 替换为指定值
inline Series fill(const Series &values, double value = 0.0) {
    Series out(values.size());
    for (std::size_t index = 0; index < values.size(); ++index) {
        out[index] = is_nan(values[index]) ? value : values[index];
    }
    return out;
}

/// 填充缺失值 (与 fill 同义)
inline Series fill_na(const Series &values, double value = 0.0) { return fill(values, value); }

/// 逐元素判断是否为 NaN
inline Bools is_na(const Series &values) {
    Bools out(values.size());
    for (std::size_t index = 0; index < values.size(); ++index) {
        out[index] = is_nan(values[index]);
    }
    return out;
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_FILLNA_H
