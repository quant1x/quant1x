"""序列对齐与重复 —— **按目标长度裁剪或补缺**

`align` 把序列收敛到指定长度：超长裁剪，不足补 NaN；`pad_to` 是它的显式命名版本，
用于二元运算前把两侧拉到同一长度。`repeat` 把常量或序列平铺到目标长度。
"""

from __future__ import annotations

import numpy as np

from .dtype import NAN, as_series


def align(values, length: int, fill_value: float = NAN) -> np.ndarray:
    """**对齐到指定长度**: 超长截断，不足补 `fill_value`

    :param values: 输入序列
    :param length: 目标长度
    :param fill_value: 补齐值
    :return: 长度为 `length` 的序列
    """
    data = as_series(values)
    if data.ndim == 0:
        return np.full(length, float(data), dtype=np.float64)
    if len(data) == length:
        return data.copy()
    if len(data) > length:
        return data[:length].copy()
    out = np.full(length, fill_value, dtype=np.float64)
    out[: len(data)] = data
    return out


def pad_to(values, length: int, fill_value: float = NAN) -> np.ndarray:
    """**补齐到指定长度**: `align` 的同义名

    :param values: 输入序列
    :param length: 目标长度
    :param fill_value: 补齐值
    :return: 长度为 `length` 的序列
    """
    return align(values, length, fill_value)


def repeat(values, length: int) -> np.ndarray:
    """**平铺到指定长度**: 常量重复，序列循环重复

    :param values: 常量或序列
    :param length: 目标长度
    :return: 长度为 `length` 的序列
    """
    data = as_series(values)
    if data.ndim == 0:
        return np.full(length, float(data), dtype=np.float64)
    if len(data) == 0:
        return np.full(length, NAN, dtype=np.float64)
    out = np.empty(length, dtype=np.float64)
    for index in range(length):
        out[index] = float(data[index % len(data)])
    return out
