"""移位、差分与累计 —— **越界补 NaN，累计顺序累加**

`shift` 与通达信 `REF` 同义：`shift(x, 1)[i] = x[i-1]`，`i < n` 的位置为 NaN。
`diff` 是「本值减 N 周期前的值」，不是相邻差分的特例函数。`cumsum` 从左到右累加，
一旦遇到 NaN，后续结果全部为 NaN（IEEE 传播）。
"""

from __future__ import annotations

import numpy as np

from .dtype import NAN, as_series


def shift(values, periods: int = 1) -> np.ndarray:
    """**向后移位**: 取 N 周期前的值

    :param values: 输入序列
    :param periods: 周期数，正数向过去取，负数向未来取
    :return: 移位后的序列，越界位置为 NaN
    """
    data = as_series(values)
    out = np.full(len(data), NAN, dtype=np.float64)
    if periods == 0:
        out[:] = data
    elif periods > 0:
        if periods < len(data):
            out[periods:] = data[: len(data) - periods]
    else:
        offset = -periods
        if offset < len(data):
            out[: len(data) - offset] = data[offset:]
    return out


def ref(values, periods: int = 1) -> np.ndarray:
    """**引用 N 周期前的值**: `shift` 的同义名

    :param values: 输入序列
    :param periods: 周期数
    :return: 引用序列
    """
    return shift(values, periods)


def diff(values, periods: int = 1) -> np.ndarray:
    """**N 周期差分**: `x[i] - x[i-periods]`

    :param values: 输入序列
    :param periods: 周期数
    :return: 差分序列，前 `periods` 个位置为 NaN
    """
    data = as_series(values)
    lagged = shift(data, periods)
    return data - lagged


def cumsum(values) -> np.ndarray:
    """**累计和**: 从左到右顺序累加

    :param values: 输入序列
    :return: 累计和序列
    """
    data = as_series(values)
    out = np.empty(len(data), dtype=np.float64)
    total = 0.0
    for index, value in enumerate(data):
        total += float(value)
        out[index] = total
    return out
