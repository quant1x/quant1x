// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_INDICATORS_HJK_H
#define QUANT1X_INDICATORS_HJK_H 1

/// hjk —— 黄金坑, 语义以 quant1x/indicators/hjk.py 为准
///
/// 与 Python 逐行对应: 加减乘除、比较、逻辑与或直接使用运算符 (xtensor 容器)。
/// 跨语言差异: Python 侧由证券代码推导涨停比例; 本实现把比例作为入参。

#include <quant1x/data/schema/bar.h>
#include <quant1x/formula/formula.h>

#include <vector>

#include "support.h"

namespace quant1x::indicators {

/// 黄金坑输出列
struct HjkResult {
    formula::Bools b;  ///< 买点
};

/// 黄金坑
inline HjkResult hjk(const std::vector<schema::Bar> &bars, double limit_up_ratio) {
    using namespace quant1x::formula;
    const Series close = detail::column(bars, &schema::Bar::close);
    const Series high = detail::column(bars, &schema::Bar::high);
    const Series volume = detail::column(bars, &schema::Bar::volume);

    const Series previous_close = ref(close, 1);
    const Series limit_price = ztprice(previous_close, limit_up_ratio);
    const Series volume_ratio = volume / ref(volume, 1);

    const Bools doubled = xt::eval(volume_ratio >= 2.00);
    const Bools broken = xt::eval(close <= limit_price);
    const Bools near_limit = xt::eval(high >= limit_price * 0.98);
    const Bools pattern = xt::eval(doubled && broken && near_limit);

    const Series pn = barslast(pattern);
    const Window pn_window = series_to_window(pn);
    const Series previous_high = ref(close, pn_window);
    const Series count_above = count(xt::eval(previous_high >= close), pn_window);
    const Series volume_high = hhv(volume, series_to_window(xt::eval(pn + 1.0)));
    const Series shrink = volume / volume_high;

    const Bools c21 = xt::eval(shrink <= 0.50);
    const Bools c22 = xt::eval((count_above / pn >= 0.50) || equal(count_above, pn));
    const Bools c23 = xt::eval((pn >= 4.0) && (pn < 7.0));
    const Bools buy = xt::eval(c21 && c22 && c23);

    return {buy};
}

}  // namespace quant1x::indicators

#endif  // QUANT1X_INDICATORS_HJK_H
