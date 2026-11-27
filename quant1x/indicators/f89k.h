// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_INDICATORS_F89K_H
#define QUANT1X_INDICATORS_F89K_H 1

/// f89k —— 89K 图形指标, 语义以 quant1x/indicators/f89k.py 为准
///
/// 与 Python 逐行对应: 比较与逻辑直接使用运算符 (xtensor 容器)。

#include <quant1x/data/schema/bar.h>
#include <quant1x/formula/formula.h>

#include <vector>

#include "support.h"

namespace quant1x::indicators {

/// 89K 图形输出列
struct F89kResult {
    formula::Series zs;  ///< 止损线
    formula::Bools b;    ///< 买点
    formula::Series dn;  ///< 放量上攻的起点距今周期数
};

/// 89K 图形指标
inline F89kResult f89k(const std::vector<schema::Bar> &bars) {
    using namespace quant1x::formula;
    const Series close = detail::column(bars, &schema::Bar::close);
    const Series high = detail::column(bars, &schema::Bar::high);
    const Series low = detail::column(bars, &schema::Bar::low);
    const Series volume = detail::column(bars, &schema::Bar::volume);

    constexpr std::int64_t n1 = 89;
    constexpr double c_s = 0.191;

    const Series high_89 = hhv(high, n1);
    const Series n7 = barslast(equal(high_89, high));
    const Series t8 = llv(low, series_to_window(n7));
    const Series n8 = barslast(xt::eval(equal(t8, low) && less_scalar(n7, static_cast<double>(n1))));
    const Series t9 = hhv(high, series_to_window(n8));
    const Series n9x = barslast(xt::eval(equal(t9, high) && greater(n7, n8)));
    const Series n9 = if_select(equal_scalar(n9x, 0.0), n9x + 1.0, n9x);
    const Series k10 = llv(low, series_to_window(n9));
    const Series n10 = barslast(xt::eval(equal(k10, low) && greater(n8, n9)));

    const Bools male = close > ref(close, 1);
    const Bools volume_up = volume > ref(volume, 1);
    const Series volume_average = ma(volume, 5);
    const Bools index_volume = volume >= volume_average * (1.0 + c_s / 10.0);
    const Bools general_volume = volume >= volume_average * (1.0 + c_s);
    const Bools branch = if_select(xt::eval(close >= 100.0), index_volume, general_volume);
    const Bools breakout = branch && male && volume_up;

    const Series dn = barslast(breakout);
    const Series stop_loss = ref(low, series_to_window(dn));

    const Bools above = (close > stop_loss) && (n9 > n10);
    const Bools below = (close < stop_loss) && (n9 > n10);
    const Bools c1_zero = equal_scalar(barslast(above), 0.0);
    const Bools c2_one = equal_scalar(barslast(below), 1.0);
    const Bools n10_positive = xt::eval(n10 > 0.0);
    const Bools buy = c1_zero && c2_one && n10_positive;

    return {stop_loss, buy, dn};
}

}  // namespace quant1x::indicators

#endif  // QUANT1X_INDICATORS_F89K_H
