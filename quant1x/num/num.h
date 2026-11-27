// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_NUM_NUM_H
#define QUANT1X_NUM_NUM_H 1

/// num —— 数值规格层: 与 quant1x/num/*.py 同语义的 C++ 对齐实现。
///
/// 口径见 quant1x/num/num.md: 一维 float64 序列、NaN 缺失、顺序累加、完整窗口、
/// 总体方差、均值中心化回归、半程远离零。容器用 std::vector, 全程显式循环,
/// 不借助 xsimd / 广播表达式, 保证与 Python spec 逐位一致。
///
/// 与 Python 的命名差异: `reduce.std` → `stddev` (`std` 遮蔽命名空间);
/// 同时提供 Rust 关键字规避名 `where_select` / `ref_value` 的对齐别名。

#include <quant1x/num/align.h>
#include <quant1x/num/decimals.h>
#include <quant1x/num/dtype.h>
#include <quant1x/num/elementwise.h>
#include <quant1x/num/ewm.h>
#include <quant1x/num/fillna.h>
#include <quant1x/num/logic.h>
#include <quant1x/num/matrix.h>
#include <quant1x/num/reduce.h>
#include <quant1x/num/regression.h>
#include <quant1x/num/rolling.h>
#include <quant1x/num/shift.h>
#include <quant1x/num/window.h>

#endif  // QUANT1X_NUM_NUM_H
