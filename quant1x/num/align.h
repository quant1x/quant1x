// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_ALIGN_H
#define QUANT1X_NUM_ALIGN_H 1

/// 序列对齐与重复 —— 按目标长度裁剪或补缺。
///
/// 跨语言参考算法: Python spec 自身不调用 align / pad_to / repeat, 应用层请用
/// quant1x/pandas 的容器对齐; 本文件只为与 Python 的对齐口径逐位对照而存在。

#include <quant1x/num/dtype.h>

#include <cstddef>

namespace quant1x::num {

/// 对齐到指定长度: 超长截断, 不足补 fill_value
inline Series align(const Series &values, std::size_t length, double fill_value = nan_value()) {
    Series out(length, fill_value);
    const std::size_t count = values.size() < length ? values.size() : length;
    for (std::size_t index = 0; index < count; ++index) {
        out[index] = values[index];
    }
    return out;
}

/// 补齐到指定长度 (与 align 同义)
inline Series pad_to(const Series &values, std::size_t length, double fill_value = nan_value()) {
    return align(values, length, fill_value);
}

/// 平铺到指定长度: 序列循环重复, 空序列补 NaN
inline Series repeat(const Series &values, std::size_t length) {
    Series out(length, nan_value());
    if (values.empty()) {
        return out;
    }
    for (std::size_t index = 0; index < length; ++index) {
        out[index] = values[index % values.size()];
    }
    return out;
}

}  // namespace quant1x::num

#endif  // QUANT1X_NUM_ALIGN_H
