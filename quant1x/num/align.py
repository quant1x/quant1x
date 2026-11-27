"""序列对齐与重复 —— **跨语言对齐的参考算法**

`align` 把序列收敛到指定长度：超长裁剪，不足补 `fill_value`（缺省 NaN）；`pad_to` 是它的
显式命名版本；`repeat` 把序列循环平铺到目标长度。

**定位：参考算法，不是 Python 的运行依赖。**

- 这些能力 NumPy 已经提供（`repeat` 等价 `np.resize`，`align` 等价切片 + `np.pad`），本模块
  再实现一遍，是为了给 C++ / Rust / Go / Zig 一个**逐位可复刻的对齐口径**——Python 作为 spec，
  其它语言照抄这里的循环即可，不必去猜 NumPy 的内部行为。
- **本项目 Python 代码本身不调用** `align` / `pad_to` / `repeat`（formula / indicators /
  pandas 都不用）；`elementwise` 的二元算子内部有自己的对齐，不需要它。
- **应用层请勿调用**：容器级的列对齐由 `quant1x.pandas` 的 `Series.align` 负责，直接数值处理用
  NumPy 的 `np.resize` / `np.pad`。本模块只为跨语言对照存在。
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
