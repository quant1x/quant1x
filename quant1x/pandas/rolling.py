"""滚动窗口门面 —— **语义全部委托 quant1x.num**

`Rolling` 只负责把 Series 的取值和窗口长度交给 `quant1x.num.rolling_*`，自身不实现算法。
窗口不足、窗口内含 NaN 的处理因此与公式、其它语言完全一致。
"""

from __future__ import annotations

from quant1x import num

from .series import Series


class Rolling:
    """**滚动窗口** —— 绑定序列与窗口长度

    :param series: 源序列
    :param window: 窗口长度
    """

    __slots__ = ("_series", "_window")

    def __init__(self, series: Series, window: int):
        self._series = series
        self._window = int(window)

    def _wrap(self, values) -> Series:
        """**包装结果**: 生成同名列的新序列

        :param values: ndarray
        :return: 新序列
        """
        return Series(values, name=self._series.name)

    def sum(self) -> Series:
        """**滚动求和**

        :return: 新序列
        """
        return self._wrap(num.rolling_sum(self._series.values, self._window))

    def mean(self) -> Series:
        """**滚动均值**

        :return: 新序列
        """
        return self._wrap(num.rolling_mean(self._series.values, self._window))

    def std(self, ddof: int = 0) -> Series:
        """**滚动标准差**: 只支持总体口径

        :param ddof: 自由度，仅接受 0
        :return: 新序列
        :raises ValueError: `ddof` 不为 0
        """
        if ddof != 0:
            raise ValueError("Rolling.std 只支持总体口径 ddof=0")
        return self._wrap(num.rolling_std(self._series.values, self._window))

    def min(self) -> Series:
        """**滚动最小值**

        :return: 新序列
        """
        return self._wrap(num.rolling_min(self._series.values, self._window))

    def max(self) -> Series:
        """**滚动最大值**

        :return: 新序列
        """
        return self._wrap(num.rolling_max(self._series.values, self._window))

    def apply(self, func, raw: bool = True) -> Series:
        """**逐窗口归约**

        :param func: 接收窗口 ndarray 的归约函数
        :param raw: 保留参数以对齐 pandas 签名，恒按 ndarray 传入
        :return: 新序列
        """
        return self._wrap(num.rolling_apply(self._series.values, self._window, func))
