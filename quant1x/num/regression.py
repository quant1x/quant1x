"""一元线性回归 —— **均值中心化最小二乘，闭式解**

`SLOPE` / `FORCAST` 一律走这里的闭式解，不用 `numpy.polyfit`：

- `polyfit` 内部做列缩放与 LAPACK SVD，末位随 BLAS 实现变化，其它语言无法逐位复现；
- 均值中心化后 `slope = Σ(t-t̄)(y-ȳ) / Σ(t-t̄)²`，`intercept = ȳ - slope·t̄`，
  只依赖加减乘除，四门语言照抄同一循环即可逐位相同。

窗口长度小于 2、或任一分量为 NaN 时返回 NaN（不足两点无法定斜率）。
"""

from __future__ import annotations

import math

import numpy as np

from .dtype import NAN, as_series


def linear_regression(values) -> tuple[float, float]:
    """**一元回归**: 返回窗口内 `y = slope * t + intercept` 的斜率与截距

    `t` 取 `0, 1, ..., n-1`。

    :param values: 输入序列
    :return: `(slope, intercept)`，不足两点或含 NaN 时为 `(NaN, NaN)`
    """
    data = as_series(values)
    count = len(data)
    if count < 2 or np.isnan(data).any():
        return NAN, NAN

    mean_t = (count - 1) / 2.0
    total = 0.0
    for value in data:
        total += float(value)
    mean_y = total / count

    numerator = 0.0
    denominator = 0.0
    for index, value in enumerate(data):
        offset = index - mean_t
        numerator += offset * (float(value) - mean_y)
        denominator += offset * offset

    slope = numerator / denominator
    intercept = mean_y - slope * mean_t
    return slope, intercept


def slope(values) -> float:
    """**回归斜率**: 序列对 `0..n-1` 的线性回归斜率

    :param values: 输入序列
    :return: 斜率，不足两点或含 NaN 时为 NaN
    """
    result, _ = linear_regression(values)
    return result


def forcast(values) -> float:
    """**回归预测值**: 回归线在最后一个点 `t = n-1` 上的取值

    :param values: 输入序列
    :return: 预测值，不足两点或含 NaN 时为 NaN
    """
    result, intercept = linear_regression(values)
    if math.isnan(result):
        return NAN
    return intercept + result * (len(as_series(values)) - 1)
