// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_ELEMENTWISE_H
#define QUANT1X_NUM_ELEMENTWISE_H 1

/// 逐元素运算 —— 与 NumPy 同序的加减乘除、取极值与条件选择。
/// NaN 按 IEEE-754 传播; 二元算子按第一个操作数的长度对齐。

#include <quant1x/num/dtype.h>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace quant1x::num {

/// 二元算子右操作数取值: 短则补 NaN、长则截断。
inline double right_at(const Series &values, std::size_t index) {
    return index < values.size() ? values[index] : nan_value();
}

/// 绝对值
inline Series abs(const Series &x) {
    Series out(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = std::abs(x[index]);
    }
    return out;
}

/// 平方根, 负数得 NaN
inline Series sqrt(const Series &x) {
    Series out(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = std::sqrt(x[index]);
    }
    return out;
}

/// 自然对数, 非正数得 NaN
inline Series log(const Series &x) {
    Series out(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = std::log(x[index]);
    }
    return out;
}

/// 幂
inline Series pow(const Series &x, double exponent) {
    Series out(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = std::pow(x[index], exponent);
    }
    return out;
}

/// 逐元素加
inline Series add(const Series &left, const Series &right) {
    Series out(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        out[index] = left[index] + right_at(right, index);
    }
    return out;
}

/// 逐元素减
inline Series sub(const Series &left, const Series &right) {
    Series out(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        out[index] = left[index] - right_at(right, index);
    }
    return out;
}

/// 逐元素乘
inline Series mul(const Series &left, const Series &right) {
    Series out(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        out[index] = left[index] * right_at(right, index);
    }
    return out;
}

/// 逐元素除, 除零得 ±inf 或 NaN
inline Series div(const Series &left, const Series &right) {
    Series out(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        out[index] = left[index] / right_at(right, index);
    }
    return out;
}

/// 序列与标量加
inline Series add_scalar(const Series &x, double value) {
    Series out(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = x[index] + value;
    }
    return out;
}

/// 序列与标量减
inline Series sub_scalar(const Series &x, double value) {
    Series out(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = x[index] - value;
    }
    return out;
}

/// 序列与标量乘
inline Series mul_scalar(const Series &x, double value) {
    Series out(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = x[index] * value;
    }
    return out;
}

/// 序列与标量除
inline Series div_scalar(const Series &x, double value) {
    Series out(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = x[index] / value;
    }
    return out;
}

/// 逐元素取大, 任一侧 NaN 则结果 NaN
inline Series maximum(const Series &left, const Series &right) {
    Series out(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        const double other = right_at(right, index);
        out[index] = (is_nan(left[index]) || is_nan(other)) ? nan_value()
                                                             : std::max(left[index], other);
    }
    return out;
}

/// 逐元素取小, 任一侧 NaN 则结果 NaN
inline Series minimum(const Series &left, const Series &right) {
    Series out(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        const double other = right_at(right, index);
        out[index] = (is_nan(left[index]) || is_nan(other)) ? nan_value()
                                                             : std::min(left[index], other);
    }
    return out;
}

/// 序列与标量取大
inline Series maximum_scalar(const Series &x, double value) {
    Series out(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = is_nan(x[index]) ? nan_value() : std::max(x[index], value);
    }
    return out;
}

/// 序列与标量取小
inline Series minimum_scalar(const Series &x, double value) {
    Series out(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = is_nan(x[index]) ? nan_value() : std::min(x[index], value);
    }
    return out;
}

/// 条件选择: 按最长对齐, 条件缺位视为假、取值缺位补 NaN。
inline Series where(const Bools &condition, const Series &true_value, const Series &false_value) {
    std::size_t length = condition.size();
    if (true_value.size() > length) {
        length = true_value.size();
    }
    if (false_value.size() > length) {
        length = false_value.size();
    }
    Series out(length);
    for (std::size_t index = 0; index < length; ++index) {
        const bool flag = index < condition.size() && condition[index];
        out[index] = flag ? right_at(true_value, index) : right_at(false_value, index);
    }
    return out;
}

/// 条件选择 (Rust 关键字规避名 where_select 的对齐别名)
inline Series where_select(const Bools &condition, const Series &true_value,
                           const Series &false_value) {
    return where(condition, true_value, false_value);
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_ELEMENTWISE_H
