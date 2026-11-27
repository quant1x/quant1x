"""类型与缺失值 —— **float64 是唯一数值类型**，NaN 是唯一缺失标记

所有数值序列在进入本包时收敛为 `float64` 的 `numpy.ndarray`；整型、bool、`pandas.Series`
都在边界处转换一次，包内不再区分 dtype。

缺失值只用 IEEE-754 的 NaN 表示，不用 `None` / `0` / 哨兵值；判断一律走 `is_nan`，
不能写 `x == nan`（恒为 False）。
"""

from __future__ import annotations

import numpy as np

#: 包内唯一的数值类型
DType = float

#: 缺失值常量
NAN = float("nan")

#: 默认值：浮点的默认值是 NaN，与 Go 侧 `TypeDefault[float64]` 一致
DEFAULT = NAN


def as_series(values) -> np.ndarray:
    """**转为 float64 序列**: 把 list / ndarray / Series 收敛成连续的一维 float64 数组

    不复制已是 float64 的 ndarray，调用方需要独立副本时自行 `.copy()`。

    :param values: 任意一维序列
    :return: float64 的一维数组
    """
    if isinstance(values, np.ndarray):
        if values.dtype == np.float64:
            return values
        return values.astype(np.float64)
    if hasattr(values, "to_numpy"):
        return np.asarray(values.to_numpy(dtype=np.float64), dtype=np.float64)
    return np.asarray(values, dtype=np.float64)


def is_nan(value) -> bool | np.ndarray:
    """**判断缺失**: 标量返回 bool，序列返回同形 bool 数组

    :param value: 标量或序列
    :return: 是否为 NaN
    """
    if isinstance(value, np.ndarray):
        return np.isnan(value)
    return bool(np.isnan(value))


def default_like(values) -> np.ndarray:
    """**生成默认值序列**: 返回与 `values` 等长的全 NaN 数组

    :param values: 决定长度的序列
    :return: 全 NaN 数组
    """
    return np.full(len(values), NAN, dtype=np.float64)


def to_scalar(value) -> float:
    """**收敛为标量**: 把 0 维数组 / 单元素序列取成 float

    :param value: 标量或单元素序列
    :return: float 标量
    """
    array = np.asarray(value, dtype=np.float64)
    if array.ndim == 0:
        return float(array)
    return float(array[-1])
