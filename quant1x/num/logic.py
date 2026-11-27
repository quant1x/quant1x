"""逻辑与比较 —— **非零即真，NaN 比较恒为假**

通达信公式里的 `AND` / `OR` / `NOT` 是逐元素逻辑运算，`>` `<` `=` 是逐元素比较。
本模块统一用 `numpy.bool_` 表达结果，输入允许是数值序列（非零为真）。

NaN 参与比较时按 IEEE-754 规则：`NaN > x`、`NaN == x` 均为假，`NaN != x` 为真。
"""

from __future__ import annotations

import numpy as np

from .dtype import as_series
from .elementwise import _pair


def _truth(values) -> np.ndarray:
    """**转为布尔序列**: 非零即真

    :param values: 数值或布尔序列
    :return: 布尔数组
    """
    array = np.asarray(values)
    if array.dtype == np.bool_:
        return array
    return array != 0


def gt(left, right) -> np.ndarray:
    """**大于**: `left > right`

    :param left: 左操作数
    :param right: 右操作数
    :return: 布尔序列
    """
    a, b = _pair(left, right)
    return a > b


def gte(left, right) -> np.ndarray:
    """**大于等于**: `left >= right`

    :param left: 左操作数
    :param right: 右操作数
    :return: 布尔序列
    """
    a, b = _pair(left, right)
    return a >= b


def lt(left, right) -> np.ndarray:
    """**小于**: `left < right`

    :param left: 左操作数
    :param right: 右操作数
    :return: 布尔序列
    """
    a, b = _pair(left, right)
    return a < b


def lte(left, right) -> np.ndarray:
    """**小于等于**: `left <= right`

    :param left: 左操作数
    :param right: 右操作数
    :return: 布尔序列
    """
    a, b = _pair(left, right)
    return a <= b


def equal(left, right) -> np.ndarray:
    """**相等**: `left == right`

    :param left: 左操作数
    :param right: 右操作数
    :return: 布尔序列
    """
    a, b = _pair(left, right)
    return a == b


def not_equal(left, right) -> np.ndarray:
    """**不等**: `left != right`

    :param left: 左操作数
    :param right: 右操作数
    :return: 布尔序列
    """
    a, b = _pair(left, right)
    return a != b


def _extend(values: np.ndarray, length: int) -> np.ndarray:
    """**补齐布尔序列**: 不足部分补 False，超出部分截断

    :param values: 布尔序列
    :param length: 目标长度
    :return: 长度为 `length` 的布尔数组
    """
    if len(values) >= length:
        return values[:length]
    out = np.zeros(length, dtype=np.bool_)
    out[: len(values)] = values
    return out


def logic_and(left, right) -> np.ndarray:
    """**逐元素逻辑与**: 两侧都真才为真

    :param left: 左操作数
    :param right: 右操作数
    :return: 布尔序列
    """
    a = _truth(left)
    b = _truth(right)
    length = max(len(a), len(b))
    return _extend(a, length) & _extend(b, length)


def logic_or(left, right) -> np.ndarray:
    """**逐元素逻辑或**: 任一侧为真即为真

    :param left: 左操作数
    :param right: 右操作数
    :return: 布尔序列
    """
    a = _truth(left)
    b = _truth(right)
    length = max(len(a), len(b))
    return _extend(a, length) | _extend(b, length)


def logic_not(value) -> np.ndarray:
    """**逐元素逻辑非**: 真变假、假变真

    :param value: 操作数
    :return: 布尔序列
    """
    return ~_truth(value)


def all_of(value) -> bool:
    """**全部为真**: 序列是否全部真

    :param value: 操作数
    :return: 是否全真
    """
    return bool(np.all(_truth(value)))


def any_of(value) -> bool:
    """**任一为真**: 序列是否存在真值

    :param value: 操作数
    :return: 是否存在真值
    """
    return bool(np.any(_truth(value)))


def count_true(value) -> int:
    """**统计真值个数**: 非零元素的个数

    :param value: 操作数
    :return: 真值个数
    """
    return int(np.count_nonzero(_truth(value)))
