// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_INDICATORS_CDTD_H
#define QUANT1X_INDICATORS_CDTD_H 1

/// cdtd —— 抄底逃顶, 语义以 quant1x/indicators/cdtd.py 为准
///
/// 与 Python 逐行对应: 加减乘除、比较、逻辑与或直接使用运算符 (xtensor 容器)。

#include <quant1x/data/schema/bar.h>
#include <quant1x/formula/formula.h>

#include <vector>

#include "support.h"

namespace quant1x::indicators {

/// 抄底逃顶输出列
struct CdtdResult {
    formula::Series wen;  ///< 趋势线
    formula::Bools s1;    ///< 卖出信号
    formula::Bools b;     ///< 买入信号
    formula::Bools s2;    ///< 趋势线 <= 5
    formula::Bools s3;    ///< 连续两周期满足买入条件
};

/// 抄底逃顶
inline CdtdResult cdtd(const std::vector<schema::Bar> &bars) {
    using namespace quant1x::formula;
    const Series close = detail::column(bars, &schema::Bar::close);
    const Series high = detail::column(bars, &schema::Bar::high);
    const Series low = detail::column(bars, &schema::Bar::low);

    constexpr std::int64_t n1 = 3;
    constexpr std::int64_t n2 = 9;
    constexpr std::int64_t n3 = 27;
    constexpr std::int64_t n4 = 5;

    const Series high_low_span = hhv(high, n3) - llv(low, n3);
    const Series close_low_span = close - llv(low, n3);
    const Series rsv1 = (close - llv(low, n2)) / (hhv(high, n2) - llv(low, n2)) * 100.0;
    const Series rsv2 = close_low_span / high_low_span * 100.0;

    const Series smooth = sma(rsv2, n4, 1);
    const Series wen = smooth * static_cast<double>(n1) - 2.0 * sma(smooth, n1, 1);

    const Series j1 = sma(rsv1, n1, 1);
    const Series j2 = sma(j1, n1, 1);

    const Bools s1 = cross(j2, j1) && (j2 > 85.0);
    const Bools s2 = wen <= 5.0;
    const Bools buy = (wen < ref(wen, 1)) && s2;
    const Bools s3 = equal_scalar(count(buy, 2), 2.0);

    return {wen, s1, buy, s2, s3};
}

}  // namespace quant1x::indicators

#endif  // QUANT1X_INDICATORS_CDTD_H
