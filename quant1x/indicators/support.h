// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_INDICATORS_SUPPORT_H
#define QUANT1X_INDICATORS_SUPPORT_H 1

/// indicators 公共前置: 从 Bar 序列取列, 统一成 formula::Series

#include <quant1x/data/schema/bar.h>
#include <quant1x/formula/formula.h>

#include <cstddef>
#include <vector>

namespace quant1x::indicators {

/// 行情结构别名
namespace schema = quant1x::data::schema;

namespace detail {

/// 取某一列
inline formula::Series column(const std::vector<schema::Bar> &bars, double schema::Bar::*member) {
    formula::Series out = formula::make_series(bars.size());
    for (std::size_t index = 0; index < bars.size(); ++index) {
        out[index] = bars[index].*member;
    }
    return out;
}

}  // namespace detail

}  // namespace quant1x::indicators

#endif  // QUANT1X_INDICATORS_SUPPORT_H
