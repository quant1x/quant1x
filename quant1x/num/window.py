"""窗口参数 —— **固定窗口与逐位置窗口统一成 int64 数组**

通达信函数的第二个参数既可以是常量（`MA(C, 5)`），也可以是序列（`HHV(VOL, PN + 1)`）。
本模块把两者统一成「长度等于序列长度」的 `int64` 数组，让下游只处理一种形态。

无效窗口（`<= 0`、NaN）保留原值或记为 `-1`，由滚动算子统一返回 NaN，不再各自解释。
"""

from __future__ import annotations

import numpy as np

#: 无效窗口标记
INVALID_WINDOW = -1


def as_window(window, length: int) -> np.ndarray:
    """**归一化窗口**: 常量展开成定长数组，序列按位取整

    浮点序列中的 NaN 记为 `INVALID_WINDOW`；负数与 0 原样保留，表示该位置不产生窗口。

    :param window: 常量窗口或逐位置窗口
    :param length: 目标序列长度
    :return: int64 的窗口数组
    """
    if isinstance(window, (int, np.integer)):
        return np.full(length, int(window), dtype=np.int64)
    if isinstance(window, (float, np.floating)):
        value = float(window)
        if np.isnan(value):
            return np.full(length, INVALID_WINDOW, dtype=np.int64)
        return np.full(length, int(value), dtype=np.int64)

    values = np.asarray(window, dtype=np.float64)
    if values.ndim == 0:
        value = float(values)
        if np.isnan(value):
            return np.full(length, INVALID_WINDOW, dtype=np.int64)
        return np.full(length, int(value), dtype=np.int64)
    if len(values) != length:
        raise ValueError(f"窗口长度与序列不一致: {len(values)} != {length}")
    out = np.full(length, INVALID_WINDOW, dtype=np.int64)
    valid = ~np.isnan(values)
    out[valid] = values[valid].astype(np.int64)
    return out


def window_at(window, index: int) -> int:
    """**取某位置的窗口**: 常量直接返回，序列按位取

    :param window: 常量窗口或逐位置窗口
    :param index: 位置
    :return: 该位置的窗口长度
    """
    if isinstance(window, (int, np.integer)):
        return int(window)
    value = window[index]
    return int(value)
