// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_ROLLING_H
#define QUANT1X_NUM_ROLLING_H 1

/// 滚动窗口 —— 完整窗口才计算, 窗口内含 NaN 即 NaN。
/// 归约函数按窗口内从左到右顺序执行, 不做增量维护。

#include <quant1x/num/dtype.h>
#include <quant1x/num/window.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace quant1x::num {

namespace detail {

/// 顺序求和
inline double sequential_sum(const Series &values) {
    double total = 0.0;
    for (const double value : values) {
        total += value;
    }
    return total;
}

/// 有效窗口的位置范围; 无效窗口为 false
struct Block {
    bool valid;
    std::size_t begin;
    std::size_t end;
};

/// 枚举窗口: 返回每个位置的有效切片范围
inline std::vector<Block> blocks(const Series &x, const Window &window) {
    const auto sizes = as_window(window, x.size());
    std::vector<Block> out(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        const std::int64_t size = sizes[index];
        if (size <= 0) {
            out[index] = Block{false, 0, 0};
            continue;
        }
        const std::size_t available = (index + 1) < static_cast<std::size_t>(size)
                                          ? index + 1
                                          : static_cast<std::size_t>(size);
        // 完整窗口才计算: 可用观测数不足窗口长度时该位置为 NaN
        if (available < static_cast<std::size_t>(size)) {
            out[index] = Block{false, 0, 0};
            continue;
        }
        out[index] = Block{true, index + 1 - available, index + 1};
    }
    return out;
}

/// 两遍标准差 (总体口径)
inline double two_pass_std(const Series &block) {
    const double count = static_cast<double>(block.size());
    const double average = sequential_sum(block) / count;
    double total = 0.0;
    for (const double value : block) {
        const double delta = value - average;
        total += delta * delta;
    }
    return std::sqrt(total / count);
}

/// 平均绝对偏差
inline double mean_absolute_deviation(const Series &block) {
    const double count = static_cast<double>(block.size());
    const double average = sequential_sum(block) / count;
    double total = 0.0;
    for (const double value : block) {
        total += std::abs(value - average);
    }
    return total / count;
}

/// 最高值到窗口末端的周期数, 并列取最近
inline double hhvbars(const Series &block) {
    std::size_t position = 0;
    for (std::size_t index = 1; index < block.size(); ++index) {
        if (block[index] >= block[position]) {
            position = index;
        }
    }
    return static_cast<double>(block.size() - 1 - position);
}

/// 最低值到窗口末端的周期数, 并列取最近
inline double llvbars(const Series &block) {
    std::size_t position = 0;
    for (std::size_t index = 1; index < block.size(); ++index) {
        if (block[index] <= block[position]) {
            position = index;
        }
    }
    return static_cast<double>(block.size() - 1 - position);
}

}  // namespace detail

/// 返回每个位置的窗口; 无效位置为空序列
inline std::vector<Series> rolling(const Series &x, const Window &window) {
    const auto window_blocks = detail::blocks(x, window);
    std::vector<Series> out(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        const auto &block = window_blocks[index];
        if (!block.valid) {
            continue;
        }
        out[index] = Series(x.begin() + static_cast<std::ptrdiff_t>(block.begin),
                            x.begin() + static_cast<std::ptrdiff_t>(block.end));
    }
    return out;
}

/// 逐窗口归约; 无效窗口或含 NaN 的窗口为 NaN
template <typename Apply>
inline Series rolling_apply(const Series &x, const Window &window, Apply apply) {
    Series out = default_like(x);
    const auto window_blocks = detail::blocks(x, window);
    for (std::size_t index = 0; index < x.size(); ++index) {
        const auto &block = window_blocks[index];
        if (!block.valid) {
            continue;
        }
        const Series chunk(x.begin() + static_cast<std::ptrdiff_t>(block.begin),
                           x.begin() + static_cast<std::ptrdiff_t>(block.end));
        bool has_nan = false;
        for (const double value : chunk) {
            if (is_nan(value)) {
                has_nan = true;
                break;
            }
        }
        if (has_nan) {
            continue;
        }
        out[index] = apply(chunk);
    }
    return out;
}

/// 滚动求和
inline Series rolling_sum(const Series &x, const Window &window) {
    return rolling_apply(x, window, detail::sequential_sum);
}

/// 滚动均值
inline Series rolling_mean(const Series &x, const Window &window) {
    return rolling_apply(x, window, [](const Series &block) {
        return detail::sequential_sum(block) / static_cast<double>(block.size());
    });
}

/// 滚动标准差 (总体口径)
inline Series rolling_std(const Series &x, const Window &window) {
    return rolling_apply(x, window, detail::two_pass_std);
}

/// 滚动最小值
inline Series rolling_min(const Series &x, const Window &window) {
    return rolling_apply(x, window, [](const Series &block) {
        double result = std::numeric_limits<double>::infinity();
        for (const double value : block) {
            if (value < result) {
                result = value;
            }
        }
        return result;
    });
}

/// 滚动最大值
inline Series rolling_max(const Series &x, const Window &window) {
    return rolling_apply(x, window, [](const Series &block) {
        double result = -std::numeric_limits<double>::infinity();
        for (const double value : block) {
            if (value > result) {
                result = value;
            }
        }
        return result;
    });
}

/// 滚动平均绝对偏差
inline Series rolling_avedev(const Series &x, const Window &window) {
    return rolling_apply(x, window, detail::mean_absolute_deviation);
}

/// 滚动最高点周期数
inline Series rolling_hhvbars(const Series &x, const Window &window) {
    return rolling_apply(x, window, detail::hhvbars);
}

/// 滚动最低点周期数
inline Series rolling_llvbars(const Series &x, const Window &window) {
    return rolling_apply(x, window, detail::llvbars);
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_ROLLING_H
