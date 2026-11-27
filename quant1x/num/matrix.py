"""形状与构造 —— **一维拼接、二维转置、等差序列**

只收一维 / 二维的容器级操作；逐元素数学在 `elementwise`、归约在 `reduce`。
`shape` 返回元组，`concat1d` 按参数顺序拼接，`transpose2d` 只接受矩形二维序列。
"""

from __future__ import annotations

import numpy as np

from .dtype import as_series


def shape(values) -> tuple[int, ...]:
    """**形状**: 返回序列的形状元组

    :param values: 输入序列
    :return: 形状
    """
    return tuple(np.shape(values))


def concat1d(*parts) -> np.ndarray:
    """**一维拼接**: 按参数顺序首尾相接

    :param parts: 一个或多个一维序列
    :return: 拼接后的序列
    """
    if not parts:
        return np.empty(0, dtype=np.float64)
    return np.concatenate([as_series(part) for part in parts])


def transpose2d(values) -> np.ndarray:
    """**二维转置**: 行列互换

    :param values: 二维序列
    :return: 转置结果
    """
    return np.asarray(values, dtype=np.float64).T


def zeros(length: int) -> np.ndarray:
    """**全零序列**

    :param length: 长度
    :return: 全零数组
    """
    return np.zeros(length, dtype=np.float64)


def ones(length: int) -> np.ndarray:
    """**全一序列**

    :param length: 长度
    :return: 全一数组
    """
    return np.ones(length, dtype=np.float64)


def arange(start: float, stop: float | None = None, step: float = 1.0) -> np.ndarray:
    """**等差序列**: 与 `numpy.arange` 同义

    :param start: 起点，或唯一参数时的终点
    :param stop: 终点（不含）
    :param step: 步长
    :return: 等差数组
    """
    if stop is None:
        return np.arange(0.0, float(start), float(step), dtype=np.float64)
    return np.arange(float(start), float(stop), float(step), dtype=np.float64)


def linspace(start: float, stop: float, count: int) -> np.ndarray:
    """**等分序列**: `count` 个点等分 `[start, stop]`（含两端）

    :param start: 起点
    :param stop: 终点
    :param count: 点数
    :return: 等分数组
    """
    return np.linspace(float(start), float(stop), int(count), dtype=np.float64)
