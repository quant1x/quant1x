"""小数位与变化率 —— **半程远离零，NaN 按 0 参与舍入**

`decimal` 对应 Go 侧 `Decimal`：`trunc((value * 10^(d+1) + half) / 10) / 10^d`，
`half` 的符号跟随数值，因此 `decimal(0.5, 0) = 1`、`decimal(-0.5, 0) = -1`，
不是 Python 内置 `round` 的银行家舍入。
"""

from __future__ import annotations

import math

import numpy as np

from .dtype import as_series


def _decimal_scalar(value: float, digits: int) -> float:
    """**单值保留小数**: 半程远离零

    :param value: 输入值
    :param digits: 小数位，负数按 0 处理
    :return: 舍入后的值
    """
    digits = max(digits, 0)
    if math.isnan(value):
        value = 0.0
    half = -5.0 if math.copysign(1.0, value) < 0 else 5.0
    scale = math.pow(10.0, digits)
    wider = math.pow(10.0, digits + 1)
    return math.trunc((value * wider + half) / 10.0) / scale


def decimal(value, digits: int = 2):
    """**保留小数位**: 标量返回 float，序列返回 ndarray

    :param value: 标量或序列
    :param digits: 小数位，缺省 2
    :return: 舍入后的值
    """
    if np.isscalar(value):
        return _decimal_scalar(float(value), digits)
    data = as_series(value)
    out = np.empty(len(data), dtype=np.float64)
    for index, item in enumerate(data):
        out[index] = _decimal_scalar(float(item), digits)
    return out


def change_rate(base, current):
    """**变化率**: `current / base`

    :param base: 基期值
    :param current: 当期值
    :return: 变化率
    """
    return _divide(current, base)


def net_change_rate(base, current):
    """**净变化率（百分比）**: `(current / base - 1) * 100`

    :param base: 基期值
    :param current: 当期值
    :return: 百分比变化率
    """
    return (_divide(current, base) - 1.0) * 100.0


def _divide(current, base):
    """**安全除**: 除零得 inf 或 NaN，不抛异常

    :param current: 分子
    :param base: 分母
    :return: 商
    """
    with np.errstate(divide="ignore", invalid="ignore"):
        return np.asarray(current, dtype=np.float64) / np.asarray(base, dtype=np.float64)
