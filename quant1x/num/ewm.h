// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_EWM_H
#define QUANT1X_NUM_EWM_H 1

/// 指数加权移动平均 —— NaN 跳过, 首个有效值作种子。
/// 递推式固定为 y_t = (1 - alpha) * y_{t-1} + alpha * x_t, y_0 = x_0。
/// 三个乘加步骤刻意拆开, 避免编译器收缩成 FMA 造成跨语言末位差异。

#include <quant1x/num/dtype.h>

#include <cstddef>

namespace quant1x::num {

/// 指数加权均值
inline Series ewm_mean(const Series &values, double alpha) {
    Series out = default_like(values);
    const double decay = 1.0 - alpha;
    double previous = nan_value();
    bool seeded = false;
    for (std::size_t index = 0; index < values.size(); ++index) {
        const double current = values[index];
        if (is_nan(current)) {
            if (seeded) {
                out[index] = previous;
            }
            continue;
        }
        if (!seeded) {
            previous = current;
            seeded = true;
            out[index] = previous;
            continue;
        }
        const double weighted_previous = decay * previous;
        const double weighted_current = alpha * current;
        previous = weighted_previous + weighted_current;
        out[index] = previous;
    }
    return out;
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_EWM_H
