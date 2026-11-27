// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_FORMULA_FORMULA_H
#define QUANT1X_FORMULA_FORMULA_H 1

/// formula —— 通达信 / 同花顺公式函数, 语义以 quant1x/formula/formula.py 为准
///
/// 容器用 xtensor 与 NumPy 对标: `Series` 是 `xt::xarray<double>`, `Bools` 是
/// `xt::xarray<bool>`。加减乘除、比较、逻辑与或非都直接用 C++ 运算符, 写法与 Python 一致:
///
///     Series rsv = (close - llv(low, n)) / (hhv(high, n) - llv(low, n)) * 100.0;
///     Bools sell = cross(j2, j1) & (j2 > 85.0);
///
/// 口径(与 Python spec 逐条对应, 详见 formula.md):
/// - 滚动窗口必须完整且不含 NaN, 否则该位置为 NaN;
/// - EMA / SMA 为 y_t = (1-alpha)*y_{t-1} + alpha*x_t, 首值作种子, NaN 跳过;
/// - SLOPE / FORCAST 用均值中心化最小二乘闭式解, 不做列缩放;
/// - RD 为半程远离零, 不是银行家舍入。
///
/// 超越函数(exp / log / sqrt)保持标量 `std::` 调用, 不走 xsimd: 公式与指标要与 Python
/// 逐位一致, 向量化实现可能在末位与 libm 不同。四则运算与比较是 IEEE 精确的, 用 xtensor
/// 表达式没有这个问题。

#include <xtensor/containers/xarray.hpp>
#include <xtensor/core/xeval.hpp>
#include <xtensor/core/xoperation.hpp>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <tuple>
#include <vector>

namespace quant1x::formula {

/// 数值序列, 缺失值用 NaN
using Series = xt::xarray<double>;

/// 布尔序列
using Bools = xt::xarray<bool>;

/// 逐位置窗口
using Window = std::vector<std::int64_t>;

/// 缺失值
inline double nan_value() { return std::numeric_limits<double>::quiet_NaN(); }

/// 判断缺失
inline bool is_nan(double value) { return std::isnan(value); }

/// 判断真值: 非零且非 NaN
inline bool is_true(double value) { return !std::isnan(value) && value != 0.0; }

/// 构造全 NaN 序列
inline Series make_series(std::size_t size) {
    Series out = xt::xarray<double>::from_shape({size});
    out.fill(nan_value());
    return out;
}

/// 构造指定值序列
inline Series filled_series(std::size_t size, double value) {
    Series out = xt::xarray<double>::from_shape({size});
    out.fill(value);
    return out;
}

/// 构造布尔序列
inline Bools make_bools(std::size_t size, bool value = false) {
    Bools out = xt::xarray<bool>::from_shape({size});
    out.fill(value);
    return out;
}

/// 布尔转 0/1 数值
inline Series as_series(const Bools &values) {
    Series out = make_series(values.size());
    for (std::size_t index = 0; index < values.size(); ++index) {
        out[index] = values[index] ? 1.0 : 0.0;
    }
    return out;
}

/// 数值转布尔: 非零且非 NaN 为真
inline Bools as_bools(const Series &values) {
    Bools out = make_bools(values.size());
    for (std::size_t index = 0; index < values.size(); ++index) {
        out[index] = is_true(values[index]);
    }
    return out;
}

namespace detail {

/// 顺序求和, 归约顺序是语义的一部分
inline double sequential_sum(const Series &x, std::size_t begin, std::size_t end) {
    double total = 0.0;
    for (std::size_t index = begin; index < end; ++index) {
        total += x[index];
    }
    return total;
}

/// 窗口内是否含 NaN
inline bool window_has_nan(const Series &x, std::size_t begin, std::size_t end) {
    for (std::size_t index = begin; index < end; ++index) {
        if (std::isnan(x[index])) {
            return true;
        }
    }
    return false;
}

/// 逐窗口归约: 完整窗口且无 NaN 才计算
template <typename Apply>
Series rolling_apply(const Series &x, const Window &window, Apply apply) {
    Series out = make_series(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        const std::int64_t size = window[index];
        if (size <= 0 || static_cast<std::size_t>(size) > index + 1) {
            continue;
        }
        const std::size_t begin = index + 1 - static_cast<std::size_t>(size);
        const std::size_t end = index + 1;
        if (window_has_nan(x, begin, end)) {
            continue;
        }
        out[index] = apply(x, begin, end);
    }
    return out;
}

}  // namespace detail

/// 常量窗口展开
inline Window constant_window(std::int64_t value, std::size_t length) { return Window(length, value); }

/// 把整数序列转成窗口
inline Window make_window(const std::vector<std::int64_t> &values) { return values; }

/// 把数值序列转成窗口: NaN 记为 -1
inline Window series_to_window(const Series &values) {
    Window out(values.size(), -1);
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (!std::isnan(values[index])) {
            out[index] = static_cast<std::int64_t>(std::llround(values[index]));
        }
    }
    return out;
}

// ---------------------------------------------------------------------------
// 逐元素运算
// ---------------------------------------------------------------------------

/// 绝对值
inline Series abs(const Series &x) {
    Series out = make_series(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = std::abs(x[index]);
    }
    return out;
}

/// 平方根
inline Series sqrt(const Series &x) {
    Series out = make_series(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = std::sqrt(x[index]);
    }
    return out;
}

/// 自然对数
inline Series log(const Series &x) {
    Series out = make_series(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = std::log(x[index]);
    }
    return out;
}

/// 整数次幂
inline Series pow(const Series &x, double exponent) {
    Series out = make_series(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = std::pow(x[index], exponent);
    }
    return out;
}

/// 逐元素加
inline Series add(const Series &left, const Series &right) { return left + right; }

/// 逐元素减
inline Series sub(const Series &left, const Series &right) { return left - right; }

/// 逐元素乘
inline Series mul(const Series &left, const Series &right) { return left * right; }

/// 逐元素除
inline Series div(const Series &left, const Series &right) { return left / right; }

/// 序列与标量运算
inline Series add_scalar(const Series &x, double value) { return x + value; }
inline Series sub_scalar(const Series &x, double value) { return x - value; }
inline Series mul_scalar(const Series &x, double value) { return x * value; }
inline Series div_scalar(const Series &x, double value) { return x / value; }

/// 取大: 任一侧 NaN 则结果 NaN
inline Series maximum(const Series &left, const Series &right) {
    Series out = make_series(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        const double a = left[index];
        const double b = right[index];
        out[index] = (std::isnan(a) || std::isnan(b)) ? nan_value() : std::max(a, b);
    }
    return out;
}

/// 取小: 任一侧 NaN 则结果 NaN
inline Series minimum(const Series &left, const Series &right) {
    Series out = make_series(left.size());
    for (std::size_t index = 0; index < left.size(); ++index) {
        const double a = left[index];
        const double b = right[index];
        out[index] = (std::isnan(a) || std::isnan(b)) ? nan_value() : std::min(a, b);
    }
    return out;
}

/// 序列与标量取大
inline Series maximum(const Series &x, double value) {
    return quant1x::formula::maximum(x, filled_series(x.size(), value));
}

/// 标量与序列取大
inline Series maximum(double value, const Series &x) {
    return quant1x::formula::maximum(filled_series(x.size(), value), x);
}

/// 序列与标量取小
inline Series minimum(const Series &x, double value) {
    return quant1x::formula::minimum(x, filled_series(x.size(), value));
}

/// 标量与序列取小
inline Series minimum(double value, const Series &x) {
    return quant1x::formula::minimum(filled_series(x.size(), value), x);
}

/// 大于
inline Bools greater(const Series &left, const Series &right) { return xt::eval(left > right); }

/// 大于等于
inline Bools greater_equal(const Series &left, const Series &right) { return xt::eval(left >= right); }

/// 小于
inline Bools less(const Series &left, const Series &right) { return xt::eval(left < right); }

/// 小于等于
inline Bools less_equal(const Series &left, const Series &right) { return xt::eval(left <= right); }

/// 相等
inline Bools equal(const Series &left, const Series &right) { return xt::eval(xt::equal(left, right)); }

/// 不等
inline Bools not_equal(const Series &left, const Series &right) { return xt::eval(xt::not_equal(left, right)); }

/// 序列与标量比较
inline Bools greater_scalar(const Series &x, double value) {
    return greater(x, filled_series(x.size(), value));
}
inline Bools greater_equal_scalar(const Series &x, double value) {
    return greater_equal(x, filled_series(x.size(), value));
}
inline Bools less_scalar(const Series &x, double value) { return less(x, filled_series(x.size(), value)); }
inline Bools less_equal_scalar(const Series &x, double value) {
    return less_equal(x, filled_series(x.size(), value));
}
inline Bools equal_scalar(const Series &x, double value) { return equal(x, filled_series(x.size(), value)); }

/// 逐元素逻辑与
inline Bools logic_and(const Bools &left, const Bools &right) { return xt::eval(left && right); }

/// 逐元素逻辑或
inline Bools logic_or(const Bools &left, const Bools &right) { return xt::eval(left || right); }

/// 逐元素逻辑非
inline Bools logic_not(const Bools &x) { return xt::eval(!x); }

/// 条件选择: 条件为真取真值
inline Series if_select(const Bools &condition, const Series &true_value, const Series &false_value) {
    Series out = make_series(condition.size());
    for (std::size_t index = 0; index < condition.size(); ++index) {
        out[index] = condition[index] ? true_value[index] : false_value[index];
    }
    return out;
}

/// 条件选择的标量版本
inline Series if_select(const Bools &condition, double true_value, double false_value) {
    Series out = make_series(condition.size());
    for (std::size_t index = 0; index < condition.size(); ++index) {
        out[index] = condition[index] ? true_value : false_value;
    }
    return out;
}

/// 条件选择: 真值为序列, 假值为标量
inline Series if_select(const Bools &condition, const Series &true_value, double false_value) {
    return if_select(condition, true_value, filled_series(true_value.size(), false_value));
}

/// 条件选择: 真值为标量, 假值为序列
inline Series if_select(const Bools &condition, double true_value, const Series &false_value) {
    return if_select(condition, filled_series(false_value.size(), true_value), false_value);
}

/// 条件选择: 两侧都是布尔序列
inline Bools if_select(const Bools &condition, const Bools &true_value, const Bools &false_value) {
    Bools out = make_bools(condition.size());
    for (std::size_t index = 0; index < condition.size(); ++index) {
        out[index] = condition[index] ? true_value[index] : false_value[index];
    }
    return out;
}

// ---------------------------------------------------------------------------
// 移位 / 差分 / 累计
// ---------------------------------------------------------------------------

/// 取 N 周期前的值
inline Series ref(const Series &x, std::int64_t periods) {
    Series out = make_series(x.size());
    if (periods == 0) {
        return x;
    }
    if (periods > 0) {
        const std::size_t offset = static_cast<std::size_t>(periods);
        for (std::size_t index = offset; index < x.size(); ++index) {
            out[index] = x[index - offset];
        }
        return out;
    }
    const std::size_t offset = static_cast<std::size_t>(-periods);
    for (std::size_t index = 0; index + offset < x.size(); ++index) {
        out[index] = x[index + offset];
    }
    return out;
}

/// 取逐位置窗口前的值
inline Series ref(const Series &x, const Window &window) {
    Series out = make_series(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        const std::int64_t periods = window[index];
        if (periods >= 0) {
            if (static_cast<std::size_t>(periods) > index) {
                continue;
            }
            out[index] = x[index - static_cast<std::size_t>(periods)];
        } else {
            const std::size_t offset = static_cast<std::size_t>(-periods);
            if (index + offset >= x.size()) {
                continue;
            }
            out[index] = x[index + offset];
        }
    }
    return out;
}

/// N 周期差分
inline Series diff(const Series &x, std::int64_t periods) { return x - ref(x, periods); }

/// 累计和
inline Series cumsum(const Series &x) {
    Series out = make_series(x.size());
    double total = 0.0;
    for (std::size_t index = 0; index < x.size(); ++index) {
        total += x[index];
        out[index] = total;
    }
    return out;
}

/// 求 N 周期累计和, 0 表示从首个值开始累计
inline Series sum(const Series &x, std::int64_t window) {
    if (window > 0) {
        return detail::rolling_apply(x, constant_window(window, x.size()),
                                     [](const Series &v, std::size_t begin, std::size_t end) {
                                         return detail::sequential_sum(v, begin, end);
                                     });
    }
    return cumsum(x);
}

/// 求逐位置窗口的累计和
inline Series sum(const Series &x, const Window &window) {
    Series out = make_series(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        const std::int64_t size = window[index];
        if (size > 0) {
            if (static_cast<std::size_t>(size) > index + 1) {
                continue;
            }
            const std::size_t begin = index + 1 - static_cast<std::size_t>(size);
            if (detail::window_has_nan(x, begin, index + 1)) {
                continue;
            }
            out[index] = detail::sequential_sum(x, begin, index + 1);
        } else {
            if (detail::window_has_nan(x, 0, index + 1)) {
                continue;
            }
            out[index] = detail::sequential_sum(x, 0, index + 1);
        }
    }
    return out;
}

/// 常量序列: 用最后一个值铺满
inline Series const_series(const Series &x) {
    if (x.size() == 0) {
        return Series{};
    }
    return filled_series(x.size(), x[x.size() - 1]);
}

// ---------------------------------------------------------------------------
// 极值与滚动窗口
// ---------------------------------------------------------------------------

namespace detail {

/// 窗口最大值
inline double window_max(const Series &v, std::size_t begin, std::size_t end) {
    double result = v[begin];
    for (std::size_t index = begin + 1; index < end; ++index) {
        if (v[index] > result) {
            result = v[index];
        }
    }
    return result;
}

/// 窗口最小值
inline double window_min(const Series &v, std::size_t begin, std::size_t end) {
    double result = v[begin];
    for (std::size_t index = begin + 1; index < end; ++index) {
        if (v[index] < result) {
            result = v[index];
        }
    }
    return result;
}

/// 最高点到窗口末端的周期数, 并列取最近
inline double window_hhvbars(const Series &v, std::size_t begin, std::size_t end) {
    std::size_t position = begin;
    for (std::size_t index = begin + 1; index < end; ++index) {
        if (v[index] >= v[position]) {
            position = index;
        }
    }
    return static_cast<double>(end - 1 - position);
}

/// 最低点到窗口末端的周期数, 并列取最近
inline double window_llvbars(const Series &v, std::size_t begin, std::size_t end) {
    std::size_t position = begin;
    for (std::size_t index = begin + 1; index < end; ++index) {
        if (v[index] <= v[position]) {
            position = index;
        }
    }
    return static_cast<double>(end - 1 - position);
}

}  // namespace detail

/// N 周期最高值
inline Series hhv(const Series &x, std::int64_t window) {
    return detail::rolling_apply(x, constant_window(window, x.size()), detail::window_max);
}

/// 逐位置窗口的最高值
inline Series hhv(const Series &x, const Window &window) {
    return detail::rolling_apply(x, window, detail::window_max);
}

/// N 周期最低值
inline Series llv(const Series &x, std::int64_t window) {
    return detail::rolling_apply(x, constant_window(window, x.size()), detail::window_min);
}

/// 逐位置窗口的最低值
inline Series llv(const Series &x, const Window &window) {
    return detail::rolling_apply(x, window, detail::window_min);
}

/// N 周期最高点位置
inline Series hhvbars(const Series &x, std::int64_t window) {
    return detail::rolling_apply(x, constant_window(window, x.size()), detail::window_hhvbars);
}

/// N 周期最低点位置
inline Series llvbars(const Series &x, std::int64_t window) {
    return detail::rolling_apply(x, constant_window(window, x.size()), detail::window_llvbars);
}

// ---------------------------------------------------------------------------
// 均线族
// ---------------------------------------------------------------------------

/// 指数加权均值: y_t = (1-alpha)*y_{t-1} + alpha*x_t, 首值作种子, NaN 跳过
inline Series ewm_mean(const Series &x, double alpha) {
    Series out = make_series(x.size());
    bool seeded = false;
    double previous = nan_value();
    const double decay = 1.0 - alpha;
    for (std::size_t index = 0; index < x.size(); ++index) {
        const double current = x[index];
        if (std::isnan(current)) {
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

namespace detail {

/// 窗口均值
inline double window_mean(const Series &v, std::size_t begin, std::size_t end) {
    return sequential_sum(v, begin, end) / static_cast<double>(end - begin);
}

/// 窗口总体标准差
inline double window_std(const Series &v, std::size_t begin, std::size_t end) {
    const double count = static_cast<double>(end - begin);
    const double average = sequential_sum(v, begin, end) / count;
    double total = 0.0;
    for (std::size_t index = begin; index < end; ++index) {
        const double delta = v[index] - average;
        total += delta * delta;
    }
    return std::sqrt(total / count);
}

/// 窗口平均绝对偏差
inline double window_avedev(const Series &v, std::size_t begin, std::size_t end) {
    const double count = static_cast<double>(end - begin);
    const double average = sequential_sum(v, begin, end) / count;
    double total = 0.0;
    for (std::size_t index = begin; index < end; ++index) {
        total += std::abs(v[index] - average);
    }
    return total / count;
}

/// 窗口线性回归斜率与截距
inline void window_regression(const Series &v, std::size_t begin, std::size_t end, double &slope, double &intercept) {
    const std::size_t count = end - begin;
    if (count < 2 || window_has_nan(v, begin, end)) {
        slope = nan_value();
        intercept = nan_value();
        return;
    }
    const double mean_t = static_cast<double>(count - 1) / 2.0;
    const double mean_y = sequential_sum(v, begin, end) / static_cast<double>(count);
    double numerator = 0.0;
    double denominator = 0.0;
    for (std::size_t index = begin; index < end; ++index) {
        const double offset = static_cast<double>(index - begin) - mean_t;
        numerator += offset * (v[index] - mean_y);
        denominator += offset * offset;
    }
    slope = numerator / denominator;
    intercept = mean_y - slope * mean_t;
}

}  // namespace detail

/// N 日简单移动平均
inline Series ma(const Series &x, std::int64_t window) {
    return detail::rolling_apply(x, constant_window(window, x.size()), detail::window_mean);
}

/// 逐位置窗口的简单移动平均
inline Series ma(const Series &x, const Window &window) {
    return detail::rolling_apply(x, window, detail::window_mean);
}

/// 指数移动平均: alpha = 2/(N+1)
inline Series ema(const Series &x, std::int64_t span) {
    const double period = span <= 0 ? 1.0 : static_cast<double>(span);
    return ewm_mean(x, 2.0 / (period + 1.0));
}

/// 中国式 SMA: alpha = M/N
inline Series sma(const Series &x, std::int64_t span, std::int64_t weight = 1) {
    const double period = span == 0 ? 1.0 : static_cast<double>(span);
    const double mass = weight == 0 ? 1.0 : static_cast<double>(weight);
    return ewm_mean(x, mass / period);
}

/// 加权移动平均
inline Series wma(const Series &x, std::int64_t window) {
    return detail::rolling_apply(x, constant_window(window, x.size()),
                                 [](const Series &v, std::size_t begin, std::size_t end) {
                                     double weighted = 0.0;
                                     double total = 0.0;
                                     for (std::size_t index = begin; index < end; ++index) {
                                         const double mass = static_cast<double>(index - begin + 1);
                                         weighted += mass * v[index];
                                         total += mass;
                                     }
                                     return weighted / total;
                                 });
}

/// 动态移动平均: 常量平滑因子
inline Series dma(const Series &x, double alpha) { return ewm_mean(x, alpha); }

/// 动态移动平均: 逐位置平滑因子, NaN 按 1 处理
inline Series dma(const Series &x, const Series &weights) {
    Series out = make_series(x.size());
    if (x.size() == 0) {
        return out;
    }
    out[0] = x[0];
    for (std::size_t index = 1; index < x.size(); ++index) {
        double alpha = index < weights.size() ? weights[index] : 1.0;
        if (std::isnan(alpha)) {
            alpha = 1.0;
        }
        out[index] = alpha * x[index] + (1.0 - alpha) * out[index - 1];
    }
    return out;
}

/// N 日总体标准差
inline Series stddev(const Series &x, std::int64_t window) {
    return detail::rolling_apply(x, constant_window(window, x.size()), detail::window_std);
}

/// 平均绝对偏差
inline Series avedev(const Series &x, std::int64_t window) {
    return detail::rolling_apply(x, constant_window(window, x.size()), detail::window_avedev);
}

/// 逐位置窗口的平均绝对偏差
inline Series avedev(const Series &x, const Window &window) {
    return detail::rolling_apply(x, window, detail::window_avedev);
}

/// 线性回归斜率
inline Series slope(const Series &x, std::int64_t window) {
    return detail::rolling_apply(x, constant_window(window, x.size()),
                                 [](const Series &v, std::size_t begin, std::size_t end) {
                                     double value = nan_value();
                                     double intercept = nan_value();
                                     detail::window_regression(v, begin, end, value, intercept);
                                     return value;
                                 });
}

/// 线性回归预测值
inline Series forcast(const Series &x, std::int64_t window) {
    return detail::rolling_apply(x, constant_window(window, x.size()),
                                 [](const Series &v, std::size_t begin, std::size_t end) {
                                     double value = nan_value();
                                     double intercept = nan_value();
                                     detail::window_regression(v, begin, end, value, intercept);
                                     if (std::isnan(value)) {
                                         return nan_value();
                                     }
                                     return intercept + value * static_cast<double>(end - begin - 1);
                                 });
}

/// 逐位置窗口的线性回归预测值
inline Series forcast(const Series &x, const Window &window) {
    return detail::rolling_apply(x, window, [](const Series &v, std::size_t begin, std::size_t end) {
        double value = nan_value();
        double intercept = nan_value();
        detail::window_regression(v, begin, end, value, intercept);
        if (std::isnan(value)) {
            return nan_value();
        }
        return intercept + value * static_cast<double>(end - begin - 1);
    });
}

// ---------------------------------------------------------------------------
// 1 级函数
// ---------------------------------------------------------------------------

/// 从前 A 日到前 B 日是否一直满足条件
inline Bools last(const Bools &condition, std::int64_t far, std::int64_t near) {
    Bools out = make_bools(condition.size());
    const std::size_t window = static_cast<std::size_t>(far + 1);
    for (std::size_t index = 0; index < condition.size(); ++index) {
        if (index + 1 < window) {
            continue;
        }
        bool good = true;
        for (std::size_t offset = static_cast<std::size_t>(near); offset < window; ++offset) {
            if (!condition[index - offset]) {
                good = false;
                break;
            }
        }
        out[index] = good;
    }
    return out;
}

/// 最近 N 天满足条件的天数
inline Series count(const Bools &condition, std::int64_t window) {
    return quant1x::formula::sum(as_series(condition), window);
}

/// 逐位置窗口的计数
inline Series count(const Bools &condition, const Window &window) {
    return quant1x::formula::sum(as_series(condition), window);
}

/// 最近 N 天是否全为真
inline Bools every(const Bools &condition, std::int64_t window) {
    Series total = count(condition, window);
    Bools out = make_bools(total.size());
    for (std::size_t index = 0; index < total.size(); ++index) {
        out[index] = total[index] == static_cast<double>(window);
    }
    return out;
}

/// 最近 N 天是否存在真值
inline Bools exist(const Bools &condition, std::int64_t window) {
    Series total = count(condition, window);
    Bools out = make_bools(total.size());
    for (std::size_t index = 0; index < total.size(); ++index) {
        out[index] = total[index] > 0.0;
    }
    return out;
}

/// 信号过滤: 条件成立后 N 个周期置 0, 返回 0/1 序列
inline Series filter(const Bools &condition, std::int64_t window) {
    Series out = as_series(condition);
    for (std::size_t index = 0; index < out.size(); ++index) {
        if (!is_true(out[index])) {
            continue;
        }
        const std::size_t end = std::min(index + 1 + static_cast<std::size_t>(window), out.size());
        for (std::size_t cursor = index + 1; cursor < end; ++cursor) {
            out[cursor] = 0.0;
        }
    }
    return out;
}

/// 上一次条件成立到当前的周期数
inline Series barslast(const Bools &condition) {
    Series out = make_series(condition.size());
    std::int64_t previous = 0;
    for (std::size_t index = 0; index < condition.size(); ++index) {
        previous = condition[index] ? 0 : previous + 1;
        out[index] = static_cast<double>(previous);
    }
    return out;
}

/// 连续满足条件的周期数
inline Series barslastcount(const Bools &condition) {
    Series out = make_series(condition.size());
    std::int64_t counter = 0;
    for (std::size_t index = 0; index < condition.size(); ++index) {
        counter = condition[index] ? counter + 1 : 0;
        out[index] = static_cast<double>(counter);
    }
    return out;
}

/// N 周期内第一次条件成立到现在的周期数
inline Series barssincen(const Bools &condition, std::int64_t window) {
    Series out = make_series(condition.size());
    out.fill(0.0);
    if (window <= 0) {
        return out;
    }
    const std::size_t size = static_cast<std::size_t>(window);
    for (std::size_t index = 0; index < condition.size(); ++index) {
        if (index + 1 < size) {
            continue;
        }
        const std::size_t begin = index + 1 - size;
        std::size_t position = 0;
        while (position < size && !condition[begin + position]) {
            ++position;
        }
        if (position == size) {
            position = 0;
        }
        if (position != 0 || condition[begin]) {
            out[index] = static_cast<double>(size - 1 - position);
        }
    }
    return out;
}

/// 向上金叉
inline Bools cross(const Series &fast, const Series &slow) {
    Bools previous = less(ref(fast, 1), ref(slow, 1));
    Bools current = greater_equal(fast, slow);
    return logic_and(previous, current);
}

/// 维持 N 周期后上穿
inline Bools longcross(const Series &fast, const Series &slow, std::int64_t window) {
    return logic_and(last(less(fast, slow), window, 1), greater(fast, slow));
}

/// 条件成立时取值, 否则沿用上一次
inline Series valuewhen(const Bools &condition, const Series &values) {
    Series out = make_series(condition.size());
    double previous = nan_value();
    bool seeded = false;
    for (std::size_t index = 0; index < condition.size(); ++index) {
        if (condition[index] && index < values.size() && !std::isnan(values[index])) {
            previous = values[index];
            seeded = true;
        }
        out[index] = seeded ? previous : nan_value();
    }
    return out;
}

/// 区间判断
inline Bools between(const Series &values, double low, double high) {
    Bools left = logic_and(greater_scalar(values, low), less_scalar(values, high));
    Bools right = logic_and(less_scalar(values, low), greater_scalar(values, high));
    return logic_or(left, right);
}

/// 当前值距区间最高点的周期数
inline Series toprange(const Series &x) {
    Series out = make_series(x.size());
    out.fill(0.0);
    for (std::size_t index = 1; index < x.size(); ++index) {
        std::size_t cursor = 0;
        while (cursor < index && x[index - 1 - cursor] < x[index]) {
            ++cursor;
        }
        out[index] = cursor == index ? 0.0 : static_cast<double>(cursor);
    }
    return out;
}

/// 当前值距区间最低点的周期数
inline Series lowrange(const Series &x) {
    Series out = make_series(x.size());
    out.fill(0.0);
    for (std::size_t index = 1; index < x.size(); ++index) {
        std::size_t cursor = 0;
        while (cursor < index && x[index - 1 - cursor] > x[index]) {
            ++cursor;
        }
        out[index] = cursor == index ? 0.0 : static_cast<double>(cursor);
    }
    return out;
}

// ---------------------------------------------------------------------------
// 舍入与涨停价
// ---------------------------------------------------------------------------

/// 半程远离零的舍入
inline double decimal_one(double value, int digits) {
    const int places = std::max(digits, 0);
    if (std::isnan(value)) {
        value = 0.0;
    }
    const double half = std::signbit(value) ? -5.0 : 5.0;
    const double scale = std::pow(10.0, static_cast<double>(places));
    const double wider = std::pow(10.0, static_cast<double>(places + 1));
    return std::trunc((value * wider + half) / 10.0) / scale;
}

/// 对序列保留指定位数
inline Series rd(const Series &x, int digits = 3) {
    Series out = make_series(x.size());
    for (std::size_t index = 0; index < x.size(); ++index) {
        out[index] = decimal_one(x[index], digits);
    }
    return out;
}

namespace detail {

/// 十进制字面量: 数值 = digits * 10^exponent
struct decimal_parts {
    std::string digits;
    int exponent = 0;
    bool negative = false;
};

/// 把 double 的最短往返表示拆成十进制字面量
inline decimal_parts parse_shortest(double value) {
    char buffer[64];
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
    std::string text(buffer, result.ptr);

    decimal_parts parts;
    std::size_t cursor = 0;
    if (!text.empty() && text.front() == '-') {
        parts.negative = true;
        cursor = 1;
    }

    int exponent = 0;
    const auto epos = text.find_first_of("eE", cursor);
    if (epos != std::string::npos) {
        exponent = std::stoi(text.substr(epos + 1));
        text.erase(epos);
    }

    const auto dot = text.find('.', cursor);
    int fraction = 0;
    if (dot != std::string::npos) {
        fraction = static_cast<int>(text.size() - dot - 1);
        text.erase(dot, 1);
    }

    const auto first = text.find_first_not_of('0');
    parts.digits = first == std::string::npos ? "0" : text.substr(first);
    parts.exponent = exponent - fraction;
    return parts;
}

/// 十进制数字串相乘
inline std::string multiply_digits(const std::string &left, const std::string &right) {
    std::vector<int> acc(left.size() + right.size(), 0);
    for (std::size_t i = left.size(); i-- > 0;) {
        for (std::size_t j = right.size(); j-- > 0;) {
            acc[i + j + 1] += (left[i] - '0') * (right[j] - '0');
        }
    }
    for (std::size_t k = acc.size(); k-- > 1;) {
        acc[k - 1] += acc[k] / 10;
        acc[k] %= 10;
    }
    std::string out;
    out.reserve(acc.size());
    for (int digit : acc) {
        out.push_back(static_cast<char>('0' + digit));
    }
    const auto first = out.find_first_not_of('0');
    return first == std::string::npos ? "0" : out.substr(first);
}

/// 按分四舍五入(半程远离零)
inline double quantize_cents(const std::string &digits, int exponent, bool negative) {
    const int target = exponent + 2;
    std::string scaled;
    if (target >= 0) {
        scaled = digits + std::string(static_cast<std::size_t>(target), '0');
    } else {
        const std::size_t drop = static_cast<std::size_t>(-target);
        const char dropped = drop <= digits.size() ? digits[digits.size() - drop] : '0';
        if (drop > digits.size()) {
            scaled = "0";
        } else {
            scaled = drop == digits.size() ? std::string() : digits.substr(0, digits.size() - drop);
            if (dropped >= '5') {
                int cursor = static_cast<int>(scaled.size()) - 1;
                while (cursor >= 0) {
                    if (scaled[static_cast<std::size_t>(cursor)] == '9') {
                        scaled[static_cast<std::size_t>(cursor)] = '0';
                        --cursor;
                    } else {
                        ++scaled[static_cast<std::size_t>(cursor)];
                        break;
                    }
                }
                if (cursor < 0) {
                    scaled.insert(scaled.begin(), '1');
                }
            }
            if (scaled.empty()) {
                scaled = "0";
            }
        }
    }

    std::string text;
    if (scaled.size() <= 2) {
        text = "0." + std::string(2 - scaled.size(), '0') + scaled;
    } else {
        text = scaled.substr(0, scaled.size() - 2) + "." + scaled.substr(scaled.size() - 2);
    }
    if (negative) {
        text.insert(text.begin(), '-');
    }
    return std::strtod(text.c_str(), nullptr);
}

}  // namespace detail

/// 涨停价: 前收盘价乘(1+比例), 按分四舍五入
inline double ztprice_one(double price, double ratio) {
    if (ratio <= 0.0 || std::isnan(price)) {
        return price;
    }
    const detail::decimal_parts base = detail::parse_shortest(price);
    const detail::decimal_parts factor = detail::parse_shortest(1.0 + ratio);
    const std::string digits = detail::multiply_digits(base.digits, factor.digits);
    return detail::quantize_cents(digits, base.exponent + factor.exponent, base.negative != factor.negative);
}

/// 涨停价: 序列版本
inline Series ztprice(const Series &price, double ratio) {
    Series out = price;
    if (ratio <= 0.0) {
        return out;
    }
    for (std::size_t index = 0; index < out.size(); ++index) {
        out[index] = ztprice_one(price[index], ratio);
    }
    return out;
}

// ---------------------------------------------------------------------------
// 2 级指标
// ---------------------------------------------------------------------------

/// MACD: 返回 (DIF, DEA, MACD)
inline std::tuple<Series, Series, Series> macd(const Series &close, std::int64_t fast = 12,
                                               std::int64_t slow = 26, std::int64_t signal = 9) {
    Series dif = ema(close, fast) - ema(close, slow);
    Series dea = ema(dif, signal);
    Series histogram = (dif - dea) * 2.0;
    return {rd(dif), rd(dea), rd(histogram)};
}

/// KDJ: 返回 (K, D, J)
inline std::tuple<Series, Series, Series> kdj(const Series &close, const Series &high, const Series &low,
                                               std::int64_t period = 9, std::int64_t m1 = 3, std::int64_t m2 = 3) {
    Series rsv = (close - llv(low, period)) / (hhv(high, period) - llv(low, period)) * 100.0;
    Series k = ema(rsv, m1 * 2 - 1);
    Series d = ema(k, m2 * 2 - 1);
    Series j = k * 3.0 - d * 2.0;
    return {k, d, j};
}

/// RSI
inline Series rsi(const Series &close, std::int64_t period = 24) {
    Series delta = close - ref(close, 1);
    Series up = sma(quant1x::formula::maximum(delta, 0.0), period, 1);
    Series down = sma(quant1x::formula::abs(delta), period, 1);
    return rd(up / down * 100.0);
}

/// W&R: 返回 (WR, WR1)
inline std::tuple<Series, Series> wr(const Series &close, const Series &high, const Series &low,
                                     std::int64_t period = 10, std::int64_t short_period = 6) {
    Series first = (hhv(high, period) - close) / (hhv(high, period) - llv(low, period)) * 100.0;
    Series second = (hhv(high, short_period) - close) / (hhv(high, short_period) - llv(low, short_period)) * 100.0;
    return {rd(first), rd(second)};
}

/// BIAS: 返回 (BIAS1, BIAS2, BIAS3)
inline std::tuple<Series, Series, Series> bias(const Series &close, std::int64_t l1 = 6, std::int64_t l2 = 12,
                                               std::int64_t l3 = 24) {
    const auto one = [&close](std::int64_t period) {
        Series average = ma(close, period);
        return (close - average) / average * 100.0;
    };
    return {rd(one(l1)), rd(one(l2)), rd(one(l3))};
}

/// BOLL: 返回 (UPPER, MID, LOWER)
inline std::tuple<Series, Series, Series> boll(const Series &close, std::int64_t period = 20, double width = 2.0) {
    Series mid = ma(close, period);
    Series band = quant1x::formula::stddev(close, period) * width;
    return {rd(mid + band), rd(mid), rd(mid - band)};
}

/// PSY: 返回 (PSY, PSYMA)
inline std::tuple<Series, Series> psy(const Series &close, std::int64_t period = 12, std::int64_t average_period = 6) {
    Bools rising = greater(close, ref(close, 1));
    Series value = count(rising, period) / static_cast<double>(period) * 100.0;
    return {rd(value), rd(ma(value, average_period))};
}

/// CCI
inline Series cci(const Series &close, const Series &high, const Series &low, std::int64_t period = 14) {
    Series typical = (high + low + close) / 3.0;
    return (typical - ma(typical, period)) / (avedev(typical, period) * 0.015);
}

/// ATR
inline Series atr(const Series &close, const Series &high, const Series &low, std::int64_t period = 20) {
    Series true_range = quant1x::formula::maximum(quant1x::formula::maximum(high - low, quant1x::formula::abs(ref(close, 1) - high)), quant1x::formula::abs(ref(close, 1) - low));
    return ma(true_range, period);
}

/// BBI
inline Series bbi(const Series &close, std::int64_t m1 = 3, std::int64_t m2 = 6, std::int64_t m3 = 12,
                  std::int64_t m4 = 20) {
    return (ma(close, m1) + ma(close, m2) + ma(close, m3) + ma(close, m4)) / 4.0;
}

/// DMI: 返回 (PDI, MDI, ADX, ADXR)
inline std::tuple<Series, Series, Series, Series> dmi(const Series &close, const Series &high, const Series &low,
                                                      std::int64_t m1 = 14, std::int64_t m2 = 6) {
    Series true_range = quant1x::formula::sum(quant1x::formula::maximum(quant1x::formula::maximum(high - low, quant1x::formula::abs(high - ref(close, 1))), quant1x::formula::abs(low - ref(close, 1))), m1);
    Series high_delta = high - ref(high, 1);
    Series low_delta = ref(low, 1) - low;
    Bools up = (high_delta > 0.0) && (high_delta > low_delta);
    Bools down = (low_delta > 0.0) && (low_delta > high_delta);
    Series dmp = quant1x::formula::sum(if_select(up, high_delta, 0.0), m1);
    Series dmm = quant1x::formula::sum(if_select(down, low_delta, 0.0), m1);
    Series pdi = dmp * 100.0 / true_range;
    Series mdi = dmm * 100.0 / true_range;
    Series adx = ma(quant1x::formula::abs(mdi - pdi) / (pdi + mdi) * 100.0, m2);
    Series adxr = (adx + ref(adx, m2)) / 2.0;
    return {pdi, mdi, adx, adxr};
}

/// TAQ: 返回 (UP, MID, DOWN)
inline std::tuple<Series, Series, Series> taq(const Series &high, const Series &low, std::int64_t period) {
    Series up = hhv(high, period);
    Series down = llv(low, period);
    Series mid = (up + down) / 2.0;
    return {up, mid, down};
}

/// KTN: 返回 (UPPER, MID, LOWER)
inline std::tuple<Series, Series, Series> ktn(const Series &close, const Series &high, const Series &low,
                                              std::int64_t period = 20, std::int64_t atr_period = 10) {
    Series mid = ema((high + low + close) / 3.0, period);
    Series band = atr(close, high, low, atr_period) * 2.0;
    return {mid + band, mid, mid - band};
}

/// TRIX: 返回 (TRIX, TRMA)
inline std::tuple<Series, Series> trix(const Series &close, std::int64_t m1 = 12, std::int64_t m2 = 20) {
    Series smoothed = ema(ema(ema(close, m1), m1), m1);
    Series value = (smoothed - ref(smoothed, 1)) / ref(smoothed, 1) * 100.0;
    return {value, ma(value, m2)};
}

/// VR
inline Series vr(const Series &close, const Series &volume, std::int64_t period = 26) {
    Bools rising = greater(close, ref(close, 1));
    Bools falling = less_equal(close, ref(close, 1));
    return quant1x::formula::sum(if_select(rising, volume, 0.0), period) / quant1x::formula::sum(if_select(falling, volume, 0.0), period) * 100.0;
}

/// EMV: 返回 (EMV, MAEMV)
inline std::tuple<Series, Series> emv(const Series &high, const Series &low, const Series &volume,
                                      std::int64_t period = 14, std::int64_t average_period = 9) {
    Series volume_ratio = ma(volume, period) / volume;
    Series mid = (high + low - ref(high + low, 1)) / (high + low) * 100.0;
    Series value = ma(mid * volume_ratio * (high - low) / ma(high - low, period), period);
    return {value, ma(value, average_period)};
}

/// DPO: 返回 (DPO, MADPO)
inline std::tuple<Series, Series> dpo(const Series &close, std::int64_t m1 = 20, std::int64_t m2 = 10,
                                      std::int64_t m3 = 6) {
    Series value = close - ref(ma(close, m1), m2);
    return {value, ma(value, m3)};
}

/// BRAR: 返回 (AR, BR)
inline std::tuple<Series, Series> brar(const Series &open, const Series &close, const Series &high, const Series &low,
                                       std::int64_t period = 26) {
    Series ar = quant1x::formula::sum(high - open, period) / quant1x::formula::sum(open - low, period) * 100.0;
    Series br = quant1x::formula::sum(quant1x::formula::maximum(0.0, high - ref(close, 1)), period) /
                quant1x::formula::sum(quant1x::formula::maximum(0.0, ref(close, 1) - low), period) * 100.0;
    return {ar, br};
}

/// DFMA: 返回 (DIF, DIFMA)
inline std::tuple<Series, Series> dfma(const Series &close, std::int64_t n1 = 10, std::int64_t n2 = 50,
                                       std::int64_t m = 10) {
    Series value = ma(close, n1) - ma(close, n2);
    return {value, ma(value, m)};
}

/// MTM: 返回 (MTM, MTMMA)
inline std::tuple<Series, Series> mtm(const Series &close, std::int64_t period = 12, std::int64_t average_period = 6) {
    Series value = close - ref(close, period);
    return {value, ma(value, average_period)};
}

/// MASS: 返回 (MASS, MA_MASS)
inline std::tuple<Series, Series> mass(const Series &high, const Series &low, std::int64_t n1 = 9, std::int64_t n2 = 25,
                                       std::int64_t m = 6) {
    Series range_series = high - low;
    Series value = quant1x::formula::sum(ma(range_series, n1) / ma(ma(range_series, n1), n1), n2);
    return {value, ma(value, m)};
}

/// ROC: 返回 (ROC, MAROC)
inline std::tuple<Series, Series> roc(const Series &close, std::int64_t period = 12, std::int64_t average_period = 6) {
    Series value = 100.0 * (close - ref(close, period)) / ref(close, period);
    return {value, ma(value, average_period)};
}

/// EXPMA: 返回 (EMA(N1), EMA(N2))
inline std::tuple<Series, Series> expma(const Series &close, std::int64_t n1 = 12, std::int64_t n2 = 50) {
    return {ema(close, n1), ema(close, n2)};
}

/// OBV
inline Series obv(const Series &close, const Series &volume) {
    Bools rising = greater(close, ref(close, 1));
    Bools falling = less(close, ref(close, 1));
    Series signed_volume = if_select(rising, volume, if_select(falling, volume * -1.0, 0.0));
    return cumsum(signed_volume) / 10000.0;
}

/// MFI
inline Series mfi(const Series &close, const Series &high, const Series &low, const Series &volume,
                  std::int64_t period = 14) {
    Series typical = (high + low + close) / 3.0;
    Bools up = greater(typical, ref(typical, 1));
    Bools down = less(typical, ref(typical, 1));
    Series ratio = quant1x::formula::sum(if_select(up, typical * volume, 0.0), period) /
                   quant1x::formula::sum(if_select(down, typical * volume, 0.0), period);
    return 100.0 - 100.0 / (1.0 + ratio);
}

/// ASI: 返回 (ASI, ASIT)
inline std::tuple<Series, Series> asi(const Series &open, const Series &close, const Series &high, const Series &low,
                                      std::int64_t m1 = 26, std::int64_t m2 = 10) {
    Series previous_close = ref(close, 1);
    Series aa = quant1x::formula::abs(high - previous_close);
    Series bb = quant1x::formula::abs(low - previous_close);
    Series cc = quant1x::formula::abs(high - ref(low, 1));
    Series dd = quant1x::formula::abs(previous_close - ref(open, 1));
    Bools first_branch = (aa > bb) && (aa > cc);
    Bools second_branch = (bb > cc) && (bb > aa);
    Series branch_one = aa + bb / 2.0 + dd / 4.0;
    Series branch_two = bb + aa / 2.0 + dd / 4.0;
    Series branch_three = cc + dd / 4.0;
    Series r = if_select(first_branch, branch_one, if_select(second_branch, branch_two, branch_three));
    Series x = close - previous_close + (close - open) / 2.0 + previous_close - ref(open, 1);
    Series si = 16.0 * x / r * quant1x::formula::maximum(aa, bb);
    Series value = quant1x::formula::sum(si, m1);
    return {value, ma(value, m2)};
}

/// XSII: 返回 (TD1, TD2, TD3, TD4)
inline std::tuple<Series, Series, Series, Series> xsii(const Series &close, const Series &high, const Series &low,
                                                       std::int64_t n = 102, std::int64_t m = 7) {
    Series typical = (2.0 * close + high + low) / 4.0;
    Series aa = ma(typical, 5);
    Series td1 = aa * static_cast<double>(n) / 100.0;
    Series td2 = aa * static_cast<double>(200 - n) / 100.0;
    Series cc = quant1x::formula::abs(typical - ma(close, 20)) / ma(close, 20);
    Series dd = dma(close, cc);
    Series td3 = (1.0 + static_cast<double>(m) / 100.0) * dd;
    Series td4 = (1.0 - static_cast<double>(m) / 100.0) * dd;
    return {td1, td2, td3, td4};
}

}  // namespace quant1x::formula

#endif  // QUANT1X_FORMULA_FORMULA_H
