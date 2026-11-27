"""逐元素运算 —— **与 NumPy 同序的加减乘除、比较取极值**

所有函数按 IEEE-754 语义传播 NaN：任一操作数为 NaN，结果即 NaN；除零得 `±inf`，
`0/0`、负数开方、非正数取对数得 NaN，不抛异常。调用方按结果里的 NaN 判缺失。

二元算子按**第一个操作数**的长度对齐，第二个短则补 NaN、长则截断（与 Go 侧 `Add` / `Sub`
等一致）；对齐口径的参考算法见 `quant1x.num.align`（该函数只为跨语言对照存在，应用层不要调用）。
"""

from __future__ import annotations

import numpy as np

from .dtype import NAN, as_series


def _pair(left, right) -> tuple[np.ndarray, np.ndarray]:
    """**对齐二元操作数**: 以左操作数长度为准，右操作数补 NaN 或截断

    :param left: 左操作数
    :param right: 右操作数
    :return: 等长的两个 float64 数组
    """
    a = as_series(left)
    b = as_series(right)
    if a.ndim == 0 or b.ndim == 0:
        return np.broadcast_arrays(a, b)
    if len(b) < len(a):
        padded = np.full(len(a), NAN, dtype=np.float64)
        padded[: len(b)] = b
        b = padded
    elif len(b) > len(a):
        b = b[: len(a)]
    return a, b


def abs(x) -> np.ndarray:
    """**绝对值**: 逐元素求 `|x|`

    :param x: 输入序列
    :return: 绝对值序列
    """
    return np.abs(as_series(x))


def sqrt(x) -> np.ndarray:
    """**平方根**: 逐元素求 `sqrt(x)`，负数得 NaN

    :param x: 输入序列
    :return: 平方根序列
    """
    with np.errstate(invalid="ignore"):
        return np.sqrt(as_series(x))


def log(x) -> np.ndarray:
    """**自然对数**: 逐元素求 `ln(x)`，非正数得 NaN

    :param x: 输入序列
    :return: 对数序列
    """
    with np.errstate(divide="ignore", invalid="ignore"):
        return np.log(as_series(x))


def pow(x, n) -> np.ndarray:
    """**整数次幂**: 逐元素求 `x ** n`

    :param x: 底数序列
    :param n: 指数，常量或等长序列
    :return: 幂序列
    """
    with np.errstate(invalid="ignore"):
        return np.power(as_series(x), as_series(n))


def add(x, y) -> np.ndarray:
    """**逐元素加**: `x + y`

    :param x: 左操作数
    :param y: 右操作数
    :return: 和序列
    """
    a, b = _pair(x, y)
    return a + b


def sub(x, y) -> np.ndarray:
    """**逐元素减**: `x - y`

    :param x: 左操作数
    :param y: 右操作数
    :return: 差序列
    """
    a, b = _pair(x, y)
    return a - b


def mul(x, y) -> np.ndarray:
    """**逐元素乘**: `x * y`

    :param x: 左操作数
    :param y: 右操作数
    :return: 积序列
    """
    a, b = _pair(x, y)
    return a * b


def div(x, y) -> np.ndarray:
    """**逐元素除**: `x / y`，除零得 `±inf` 或 NaN

    :param x: 被除数
    :param y: 除数
    :return: 商序列
    """
    a, b = _pair(x, y)
    with np.errstate(divide="ignore", invalid="ignore"):
        return a / b


def maximum(x, y) -> np.ndarray:
    """**逐元素取大**: `max(x, y)`，任一侧 NaN 则结果 NaN

    :param x: 左操作数
    :param y: 右操作数
    :return: 逐元素较大值
    """
    a, b = _pair(x, y)
    return np.maximum(a, b)


def minimum(x, y) -> np.ndarray:
    """**逐元素取小**: `min(x, y)`，任一侧 NaN 则结果 NaN

    :param x: 左操作数
    :param y: 右操作数
    :return: 逐元素较小值
    """
    a, b = _pair(x, y)
    return np.minimum(a, b)


def add_scalar(x, value: float) -> np.ndarray:
    """**序列与标量加**: `x + value`

    :param x: 输入序列
    :param value: 标量
    :return: 和序列
    """
    return as_series(x) + float(value)


def sub_scalar(x, value: float) -> np.ndarray:
    """**序列与标量减**: `x - value`

    :param x: 输入序列
    :param value: 标量
    :return: 差序列
    """
    return as_series(x) - float(value)


def mul_scalar(x, value: float) -> np.ndarray:
    """**序列与标量乘**: `x * value`

    :param x: 输入序列
    :param value: 标量
    :return: 积序列
    """
    return as_series(x) * float(value)


def div_scalar(x, value: float) -> np.ndarray:
    """**序列与标量除**: `x / value`，除零得 `±inf` 或 NaN

    :param x: 输入序列
    :param value: 标量
    :return: 商序列
    """
    with np.errstate(divide="ignore", invalid="ignore"):
        return as_series(x) / float(value)


def maximum_scalar(x, value: float) -> np.ndarray:
    """**序列与标量取大**: 任一侧 NaN 则结果 NaN

    :param x: 输入序列
    :param value: 标量
    :return: 逐元素较大值
    """
    values = as_series(x)
    return np.maximum(values, float(value))


def minimum_scalar(x, value: float) -> np.ndarray:
    """**序列与标量取小**: 任一侧 NaN 则结果 NaN

    :param x: 输入序列
    :param value: 标量
    :return: 逐元素较小值
    """
    values = as_series(x)
    return np.minimum(values, float(value))


def where(condition, true_value, false_value) -> np.ndarray:
    """**条件选择**: `condition` 非零取 `true_value`，否则取 `false_value`

    三个操作数先按最长对齐，缺位补 NaN。

    :param condition: 条件序列，非零为真
    :param true_value: 条件成立时的取值
    :param false_value: 条件不成立时的取值
    :return: 选择结果
    """
    cond = np.asarray(condition)
    if cond.dtype != np.bool_:
        cond = cond != 0
    a = as_series(true_value)
    b = as_series(false_value)
    length = max(cond.size if cond.ndim else 1, a.size if a.ndim else 1, b.size if b.ndim else 1)
    cond = _extend_bool(cond, length)
    a = _extend_float(a, length)
    b = _extend_float(b, length)
    return np.where(cond, a, b)


def _extend_float(values: np.ndarray, length: int) -> np.ndarray:
    """**补齐浮点序列**: 在尾部补 NaN 到指定长度

    :param values: 输入序列
    :param length: 目标长度
    :return: 长度不小于 `length` 的序列
    """
    if values.ndim == 0:
        return np.full(length, float(values), dtype=np.float64)
    if len(values) >= length:
        return values[:length]
    out = np.full(length, NAN, dtype=np.float64)
    out[: len(values)] = values
    return out


def _extend_bool(values: np.ndarray, length: int) -> np.ndarray:
    """**补齐布尔序列**: 在尾部补 False 到指定长度

    :param values: 输入序列
    :param length: 目标长度
    :return: 长度不小于 `length` 的布尔序列
    """
    if values.ndim == 0:
        return np.full(length, bool(values), dtype=np.bool_)
    if len(values) >= length:
        return values[:length]
    out = np.zeros(length, dtype=np.bool_)
    out[: len(values)] = values
    return out
