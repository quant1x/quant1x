// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_REDUCE_H
#define QUANT1X_NUM_REDUCE_H 1

/// 归约 —— 顺序累加、总体方差、NaN 传播。
/// 归约顺序是语义的一部分: 一律从左到右顺序累加。

#include <quant1x/num/dtype.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace quant1x::num {

/// 求和; 空输入为 0
inline double sum_value(const Series &values) {
    double total = 0.0;
    for (const double value : values) {
        total += value;
    }
    return total;
}

/// 均值; 空输入为 NaN
inline double mean(const Series &values) {
    if (values.empty()) {
        return nan_value();
    }
    return sum_value(values) / static_cast<double>(values.size());
}

/// 中位数; 空输入或含 NaN 为 NaN
inline double median(const Series &values) {
    if (values.empty()) {
        return nan_value();
    }
    for (const double value : values) {
        if (is_nan(value)) {
            return nan_value();
        }
    }
    std::vector<double> ordered(values.begin(), values.end());
    std::sort(ordered.begin(), ordered.end());
    const std::size_t middle = ordered.size() / 2;
    if (ordered.size() % 2 == 0) {
        return (ordered[middle - 1] + ordered[middle]) / 2.0;
    }
    return ordered[middle];
}

namespace detail {

/// 空输入或含 NaN 时不可归约
inline bool not_reducible(const Series &values) {
    if (values.empty()) {
        return true;
    }
    for (const double value : values) {
        if (is_nan(value)) {
            return true;
        }
    }
    return false;
}

}  // namespace detail

/// 最小值; 空输入或含 NaN 为 NaN
inline double min_value(const Series &values) {
    if (detail::not_reducible(values)) {
        return nan_value();
    }
    double result = std::numeric_limits<double>::infinity();
    for (const double value : values) {
        if (value < result) {
            result = value;
        }
    }
    return result;
}

/// 最大值; 空输入或含 NaN 为 NaN
inline double max_value(const Series &values) {
    if (detail::not_reducible(values)) {
        return nan_value();
    }
    double result = -std::numeric_limits<double>::infinity();
    for (const double value : values) {
        if (value > result) {
            result = value;
        }
    }
    return result;
}

/// 最小值下标; 空输入或含 NaN 为 -1, 并列取第一个
inline std::int64_t argmin(const Series &values) {
    if (detail::not_reducible(values)) {
        return -1;
    }
    std::size_t best = 0;
    for (std::size_t index = 1; index < values.size(); ++index) {
        if (values[index] < values[best]) {
            best = index;
        }
    }
    return static_cast<std::int64_t>(best);
}

/// 最大值下标; 空输入或含 NaN 为 -1, 并列取第一个
inline std::int64_t argmax(const Series &values) {
    if (detail::not_reducible(values)) {
        return -1;
    }
    std::size_t best = 0;
    for (std::size_t index = 1; index < values.size(); ++index) {
        if (values[index] > values[best]) {
            best = index;
        }
    }
    return static_cast<std::int64_t>(best);
}

/// 总体方差; 空输入为 0
inline double variance(const Series &values) {
    if (values.empty()) {
        return 0.0;
    }
    const double average = mean(values);
    double total = 0.0;
    for (const double value : values) {
        const double delta = value - average;
        total += delta * delta;
    }
    return total / static_cast<double>(values.size());
}

/// 总体标准差; 空输入为 0
///
/// C++ 侧命名为 stddev: `std` 会遮蔽 `std::` 命名空间, 与 Python 的 `reduce.std`
/// 语义相同。
inline double stddev(const Series &values) { return std::sqrt(variance(values)); }

/// 总体协方差; 长度不一致或空输入为 0
inline double covariance(const Series &left, const Series &right) {
    if (left.size() != right.size() || left.empty()) {
        return 0.0;
    }
    const double mean_left = mean(left);
    const double mean_right = mean(right);
    double total = 0.0;
    for (std::size_t index = 0; index < left.size(); ++index) {
        total += (left[index] - mean_left) * (right[index] - mean_right);
    }
    return total / static_cast<double>(left.size());
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_REDUCE_H
