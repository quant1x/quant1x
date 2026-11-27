// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_MATRIX_H
#define QUANT1X_NUM_MATRIX_H 1

/// 形状与构造 —— 一维拼接、二维转置、等差序列。

#include <quant1x/num/dtype.h>

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <vector>

namespace quant1x::num {

/// 形状 (一维序列固定为 {length})
inline std::vector<std::size_t> shape(const Series &values) { return {values.size()}; }

/// 一维拼接
inline Series concat1d(std::initializer_list<Series> parts) {
    std::size_t total = 0;
    for (const auto &part : parts) {
        total += part.size();
    }
    Series out;
    out.reserve(total);
    for (const auto &part : parts) {
        out.insert(out.end(), part.begin(), part.end());
    }
    return out;
}

/// 二维转置; 非矩形输入按最短行截断
inline std::vector<Series> transpose2d(const std::vector<Series> &values) {
    std::vector<Series> out;
    if (values.empty()) {
        return out;
    }
    std::size_t columns = values.front().size();
    for (const auto &row : values) {
        columns = std::min(columns, row.size());
    }
    out.reserve(columns);
    for (std::size_t column = 0; column < columns; ++column) {
        Series row(values.size());
        for (std::size_t index = 0; index < values.size(); ++index) {
            row[index] = values[index][column];
        }
        out.push_back(std::move(row));
    }
    return out;
}

/// 全零序列
inline Series zeros(std::size_t length) { return Series(length, 0.0); }

/// 全一序列
inline Series ones(std::size_t length) { return Series(length, 1.0); }

/// 等差序列 [start, stop)
inline Series arange(double start, double stop, double step) {
    Series out;
    if (step == 0.0) {
        return out;
    }
    double value = start;
    if (step > 0.0) {
        while (value < stop) {
            out.push_back(value);
            value += step;
        }
    } else {
        while (value > stop) {
            out.push_back(value);
            value += step;
        }
    }
    return out;
}

/// 等分序列, 含两端
inline Series linspace(double start, double stop, std::size_t count) {
    Series out;
    if (count == 0) {
        return out;
    }
    if (count == 1) {
        out.push_back(start);
        return out;
    }
    const double step = (stop - start) / static_cast<double>(count - 1);
    out.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        out.push_back(start + step * static_cast<double>(index));
    }
    return out;
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_MATRIX_H
