// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_INDICATORS_P2FB_H
#define QUANT1X_INDICATORS_P2FB_H 1

/// p2fb —— 趋势反包, 语义以 quant1x/indicators/p2fb.py 为准
///
/// 与 Python 逐行对应: 加减乘除、比较、逻辑与或直接使用运算符 (xtensor 容器)。
/// 跨语言差异: Python 侧由证券代码推导涨停比例; 本实现把比例作为入参。

#include <quant1x/data/schema/bar.h>
#include <quant1x/formula/formula.h>

#include <vector>

#include "support.h"

namespace quant1x::indicators {

/// 趋势反包输出列
struct P2fbResult {
    formula::Series hgzx;  ///< 回归转向
};

/// 趋势反包
inline P2fbResult p2fb(const std::vector<schema::Bar> &bars, double limit_up_ratio) {
    using namespace quant1x::formula;
    const Series close = detail::column(bars, &schema::Bar::close);

    constexpr std::int64_t slope_period = 10;

    const Series a = slope(close, slope_period) / close * 100.0;
    const Series b = forcast(close, slope_period) / close;
    const Series d = -a;
    const Series f = (a * close + b * close) / close;
    const Bools turn = xt::eval(cross(a, d) || cross(f, d));
    const Series hgzx = filter(turn, 5) * 4.0;

    (void)limit_up_ratio;
    return {hgzx};
}

}  // namespace quant1x::indicators

#endif  // QUANT1X_INDICATORS_P2FB_H
