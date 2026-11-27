// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_INDICATORS_MA1X_H
#define QUANT1X_INDICATORS_MA1X_H 1

/// ma1x —— 89K 趋势策略线, 语义以 quant1x/indicators/ma1x.py 为准
///
/// 与 Python 逐行对应: 加减乘除、比较、逻辑与或直接使用运算符 (xtensor 容器)。

#include <quant1x/data/schema/bar.h>
#include <quant1x/formula/formula.h>

#include <vector>

#include "support.h"

namespace quant1x::indicators {

/// 89K 趋势策略线输出列
struct Ma1xResult {
    formula::Series dn;  ///< 放量上攻起点距今周期数
    formula::Series zs;  ///< 止盈止损线
    formula::Series yl;  ///< 压力线
    formula::Bools hc;   ///< 回踩
    formula::Bools tp;   ///< 突破
};

/// 89K 趋势策略线
inline Ma1xResult ma1x(const std::vector<schema::Bar> &bars) {
    using namespace quant1x::formula;
    const Series close = detail::column(bars, &schema::Bar::close);
    const Series high = detail::column(bars, &schema::Bar::high);
    const Series low = detail::column(bars, &schema::Bar::low);
    const Series volume = detail::column(bars, &schema::Bar::volume);

    constexpr double c_s = 0.191;

    const Bools male = xt::eval(close > ref(close, 1));
    const Bools volume_up = xt::eval(volume > ref(volume, 1));
    const Series volume_average = ma(volume, 5);
    const Bools index_volume = xt::eval(volume >= volume_average * (1.0 + c_s / 10.0));
    const Bools general_volume = xt::eval(volume >= volume_average * (1.0 + c_s));
    const Bools branch = if_select(xt::eval(close >= 100.0), index_volume, general_volume);
    const Bools breakout = xt::eval(branch && male && volume_up);

    const Series dn = barslast(breakout);
    const Series stop_loss = ref(low, series_to_window(dn));

    const Series h0 = hhv(high, 5);
    const Bools resistance_pattern = xt::eval(male && equal(high, h0) && volume_up);
    const Series yl_n = barslast(resistance_pattern);
    const Series resistance_high = ref(high, series_to_window(yl_n));

    const Bools dn_zero = equal_scalar(dn, 0.0);
    const Series support_period = if_select(dn_zero, ref(stop_loss, 1) + 1.0, dn);
    const Series support_line = if_select(dn_zero, ref(stop_loss, 1), stop_loss);

    const Series previous_high = ref(high, 1);
    const Series wash = (previous_high - low) / previous_high * 100.0;
    const Bools pullback = xt::eval((support_period >= 2.0) && (wash >= 2.33) && (low < support_line) &&
                                    (close >= support_line));

    const Bools yl_zero = equal_scalar(yl_n, 0.0);
    const Series resistance_period = if_select(yl_zero, ref(yl_n, 1) + 1.0, yl_n);
    const Series resistance_line = if_select(yl_zero, ref(resistance_high, 1), resistance_high);

    const Bools touched = xt::eval((ref(close, 1) < resistance_line) || (ref(low, 1) < resistance_line) ||
                                   (low < resistance_line));
    const Bools recovered = xt::eval(close >= resistance_line);
    const Bools penetrated = xt::eval(touched && recovered && (resistance_period >= 2.0));

    return {dn, stop_loss, resistance_high, pullback, penetrated};
}

}  // namespace quant1x::indicators

#endif  // QUANT1X_INDICATORS_MA1X_H
