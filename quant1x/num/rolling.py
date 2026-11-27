"""滚动窗口 —— **完整窗口才计算，窗口内含 NaN 即 NaN**

滚动框架只有一条判据：

- 第 `i` 个位置的窗口长度 `w = window[i]`；`w <= 0` 或 `w > i + 1` 时不产生窗口，结果为 NaN；
- 窗口内任一分量为 NaN，归约结果就是 NaN（由归约本身的 IEEE 语义自然传播）；
- 归约函数按窗口内从左到右顺序执行，不做增量维护 —— 增量加减会引入累积误差，
  跨语言无法逐位对齐。

窗口既可以是常量（`5`），也可以是逐位置序列（`PN + 1`）。
"""

from __future__ import annotations

from collections.abc import Callable

import numpy as np

from .dtype import NAN, as_series
from .window import as_window


def _blocks(values: np.ndarray, window, min_periods: int | None = None):
    """**枚举窗口**: 逐个位置产出 `(index, start, end)`，无效窗口产出 `None`

    :param values: 输入序列
    :param window: 常量窗口或逐位置窗口
    :param min_periods: 最少观测数，缺省等于窗口长度
    :return: 逐位置迭代表
    """
    sizes = as_window(window, len(values))
    for index in range(len(values)):
        size = int(sizes[index])
        if size <= 0:
            yield index, None
            continue
        available = min(index + 1, size)
        threshold = size if min_periods is None else min_periods
        if available < threshold:
            yield index, None
            continue
        yield index, (index + 1 - available, index + 1)


def rolling(values, window, min_periods: int | None = None) -> list[np.ndarray]:
    """**返回每个位置的窗口**: 无效位置返回空数组

    :param values: 输入序列
    :param window: 常量窗口或逐位置窗口
    :param min_periods: 最少观测数，缺省等于窗口长度
    :return: 窗口列表
    """
    data = as_series(values)
    out: list[np.ndarray] = []
    for _, block in _blocks(data, window, min_periods):
        out.append(np.empty(0, dtype=np.float64) if block is None else data[block[0]:block[1]])
    return out


def rolling_apply(
    values,
    window,
    apply: Callable[[np.ndarray], float],
    min_periods: int | None = None,
) -> np.ndarray:
    """**逐窗口归约**: 对每个窗口调用 `apply`，无效窗口为 NaN

    :param values: 输入序列
    :param window: 常量窗口或逐位置窗口
    :param apply: 窗口归约函数
    :param min_periods: 最少观测数，缺省等于窗口长度
    :return: 归约结果序列
    """
    data = as_series(values)
    out = np.full(len(data), NAN, dtype=np.float64)
    for index, block in _blocks(data, window, min_periods):
        if block is None:
            continue
        chunk = data[block[0]:block[1]]
        if np.isnan(chunk).any():
            continue
        out[index] = float(apply(chunk))
    return out


def _sequential_sum(block: np.ndarray) -> float:
    """**顺序求和**: 窗口内从左到右累加

    :param block: 窗口
    :return: 累加和
    """
    total = 0.0
    for value in block:
        total += float(value)
    return total


def rolling_sum(values, window, min_periods: int | None = None) -> np.ndarray:
    """**滚动求和**: 每个窗口的顺序累加和

    :param values: 输入序列
    :param window: 常量窗口或逐位置窗口
    :param min_periods: 最少观测数，缺省等于窗口长度
    :return: 求和序列
    """
    return rolling_apply(values, window, _sequential_sum, min_periods)


def rolling_mean(values, window, min_periods: int | None = None) -> np.ndarray:
    """**滚动均值**: 窗口和除以窗口长度

    :param values: 输入序列
    :param window: 常量窗口或逐位置窗口
    :param min_periods: 最少观测数，缺省等于窗口长度
    :return: 均值序列
    """
    def average(block: np.ndarray) -> float:
        return _sequential_sum(block) / len(block)

    return rolling_apply(values, window, average, min_periods)


def _two_pass_std(block: np.ndarray) -> float:
    """**两遍标准差**: 先均值、再离差平方均值、最后开方

    :param block: 窗口
    :return: 总体标准差
    """
    count = len(block)
    average = _sequential_sum(block) / count
    total = 0.0
    for value in block:
        delta = float(value) - average
        total += delta * delta
    return float(np.sqrt(total / count))


def rolling_std(values, window, min_periods: int | None = None) -> np.ndarray:
    """**滚动标准差**: 总体口径（分母为窗口长度）

    :param values: 输入序列
    :param window: 常量窗口或逐位置窗口
    :param min_periods: 最少观测数，缺省等于窗口长度
    :return: 标准差序列
    """
    return rolling_apply(values, window, _two_pass_std, min_periods)


def rolling_min(values, window, min_periods: int | None = None) -> np.ndarray:
    """**滚动最小值**

    :param values: 输入序列
    :param window: 常量窗口或逐位置窗口
    :param min_periods: 最少观测数，缺省等于窗口长度
    :return: 最小值序列
    """
    def minimum(block: np.ndarray) -> float:
        result = float(block[0])
        for value in block:
            if float(value) < result:
                result = float(value)
        return result

    return rolling_apply(values, window, minimum, min_periods)


def rolling_max(values, window, min_periods: int | None = None) -> np.ndarray:
    """**滚动最大值**

    :param values: 输入序列
    :param window: 常量窗口或逐位置窗口
    :param min_periods: 最少观测数，缺省等于窗口长度
    :return: 最大值序列
    """
    def maximum(block: np.ndarray) -> float:
        result = float(block[0])
        for value in block:
            if float(value) > result:
                result = float(value)
        return result

    return rolling_apply(values, window, maximum, min_periods)


def _mean_absolute_deviation(block: np.ndarray) -> float:
    """**平均绝对偏差**: 窗口内各值与窗口均值的绝对差的平均

    :param block: 窗口
    :return: 平均绝对偏差
    """
    count = len(block)
    average = _sequential_sum(block) / count
    total = 0.0
    for value in block:
        total += abs(float(value) - average)
    return total / count


def rolling_avedev(values, window, min_periods: int | None = None) -> np.ndarray:
    """**滚动平均绝对偏差**

    :param values: 输入序列
    :param window: 常量窗口或逐位置窗口
    :param min_periods: 最少观测数，缺省等于窗口长度
    :return: 平均绝对偏差序列
    """
    return rolling_apply(values, window, _mean_absolute_deviation, min_periods)


def _hhvbars(block: np.ndarray) -> float:
    """**最高点距当前的周期数**: 并列取最近的一个

    :param block: 窗口
    :return: 距窗口末端的周期数
    """
    position = 0
    for index in range(1, len(block)):
        if float(block[index]) >= float(block[position]):
            position = index
    return float(len(block) - 1 - position)


def _llvbars(block: np.ndarray) -> float:
    """**最低点距当前的周期数**: 并列取最近的一个

    :param block: 窗口
    :return: 距窗口末端的周期数
    """
    position = 0
    for index in range(1, len(block)):
        if float(block[index]) <= float(block[position]):
            position = index
    return float(len(block) - 1 - position)


def rolling_hhvbars(values, window, min_periods: int | None = None) -> np.ndarray:
    """**滚动最高点周期数**: 最高值到当前窗口末端的周期数

    :param values: 输入序列
    :param window: 常量窗口或逐位置窗口
    :param min_periods: 最少观测数，缺省等于窗口长度
    :return: 周期数序列
    """
    return rolling_apply(values, window, _hhvbars, min_periods)


def rolling_llvbars(values, window, min_periods: int | None = None) -> np.ndarray:
    """**滚动最低点周期数**: 最低值到当前窗口末端的周期数

    :param values: 输入序列
    :param window: 常量窗口或逐位置窗口
    :param min_periods: 最少观测数，缺省等于窗口长度
    :return: 周期数序列
    """
    return rolling_apply(values, window, _llvbars, min_periods)
