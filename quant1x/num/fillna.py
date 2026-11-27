"""缺失值填充 —— **默认复制入参，不原地修改**

`fill(x, value)` 返回新序列：所有 NaN 替换为 `value`。需要原地写入时显式传 `inplace=True`，
调用方要自行确认入参可写 —— 这与 Go 侧 `Fill` 的第二个布尔参数同义。
"""

from __future__ import annotations

import numpy as np

from .dtype import NAN, as_series


def fill(values, value: float = 0.0, inplace: bool = False) -> np.ndarray:
    """**填充缺失值**: 把 NaN 替换为指定值

    :param values: 输入序列
    :param value: 替换值
    :param inplace: 是否原地写入
    :return: 填充后的序列
    """
    data = as_series(values) if inplace else np.array(as_series(values), dtype=np.float64, copy=True)
    data[np.isnan(data)] = value
    return data


def fill_na(values, value: float = 0.0, inplace: bool = False) -> np.ndarray:
    """**填充缺失值**: `fill` 的同义名

    :param values: 输入序列
    :param value: 替换值
    :param inplace: 是否原地写入
    :return: 填充后的序列
    """
    return fill(values, value, inplace)


def is_na(values) -> np.ndarray:
    """**缺失标记**: 逐元素判断是否为 NaN

    :param values: 输入序列
    :return: 布尔数组
    """
    return np.isnan(as_series(values))


__all__ = ["fill", "fill_na", "is_na", "NAN"]
