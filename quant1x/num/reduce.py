"""归约 —— **顺序累加、总体方差、NaN 传播**

归约顺序是语义的一部分：本模块一律按**从左到右顺序累加**，不做 NumPy 的分块求和，
这样 C++ / Rust / Go / Zig 只要照抄同一循环就能逐位相同。

方差 / 标准差用两遍算法（先均值、再离差平方均值），分母是 `n`（总体口径）。
空输入：`sum` / `variance` / `std` 为 `0.0`，`mean` / `median` / `min` / `max` 为 NaN。
"""

from __future__ import annotations

import math

import numpy as np

from .dtype import NAN, as_series


def sum_value(x) -> float:
    """**求和**: 从左到右顺序累加

    :param x: 输入序列
    :return: 累加和，空输入为 `0.0`
    """
    total = 0.0
    for value in as_series(x):
        total += float(value)
    return total


def mean(x) -> float:
    """**均值**: `sum(x) / n`

    :param x: 输入序列
    :return: 算术平均，空输入为 NaN
    """
    values = as_series(x)
    if len(values) == 0:
        return NAN
    return sum_value(values) / len(values)


def median(x) -> float:
    """**中位数**: 排序后取中点，任一 NaN 传播

    :param x: 输入序列
    :return: 中位数，空输入为 NaN
    """
    values = as_series(x)
    if len(values) == 0:
        return NAN
    if np.isnan(values).any():
        return NAN
    ordered = np.sort(values)
    middle = len(ordered) // 2
    if len(ordered) % 2 == 0:
        return float((ordered[middle - 1] + ordered[middle]) / 2.0)
    return float(ordered[middle])


def min_value(x) -> float:
    """**最小值**: 任一 NaN 传播

    :param x: 输入序列
    :return: 最小值，空输入为 NaN
    """
    values = as_series(x)
    if len(values) == 0 or np.isnan(values).any():
        return NAN
    return float(np.min(values))


def max_value(x) -> float:
    """**最大值**: 任一 NaN 传播

    :param x: 输入序列
    :return: 最大值，空输入为 NaN
    """
    values = as_series(x)
    if len(values) == 0 or np.isnan(values).any():
        return NAN
    return float(np.max(values))


def argmin(x) -> int:
    """**最小值下标**: 并列取第一个

    :param x: 输入序列
    :return: 下标，空输入或含 NaN 时为 `-1`
    """
    values = as_series(x)
    if len(values) == 0 or np.isnan(values).any():
        return -1
    return int(np.argmin(values))


def argmax(x) -> int:
    """**最大值下标**: 并列取第一个

    :param x: 输入序列
    :return: 下标，空输入或含 NaN 时为 `-1`
    """
    values = as_series(x)
    if len(values) == 0 or np.isnan(values).any():
        return -1
    return int(np.argmax(values))


def variance(x) -> float:
    """**总体方差**: 两遍算法，分母为 `n`

    :param x: 输入序列
    :return: 方差，空输入为 `0.0`
    """
    values = as_series(x)
    if len(values) == 0:
        return 0.0
    average = mean(values)
    total = 0.0
    for value in values:
        delta = float(value) - average
        total += delta * delta
    return total / len(values)


def std(x) -> float:
    """**总体标准差**: `sqrt(variance(x))`

    :param x: 输入序列
    :return: 标准差，空输入为 `0.0`
    """
    return math.sqrt(variance(x))


def covariance(x, y) -> float:
    """**总体协方差**: 两遍算法，分母为 `n`

    长度不一致或空输入返回 `0.0`，与 Go 侧 `Covariance` 一致。

    :param x: 左序列
    :param y: 右序列
    :return: 协方差
    """
    left = as_series(x)
    right = as_series(y)
    if len(left) != len(right) or len(left) == 0:
        return 0.0
    mean_x = mean(left)
    mean_y = mean(right)
    total = 0.0
    for a, b in zip(left, right):
        total += (float(a) - mean_x) * (float(b) - mean_y)
    return total / len(left)
