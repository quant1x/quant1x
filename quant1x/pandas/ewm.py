"""指数加权门面 —— **span / alpha 二选一，语义委托 quant1x.num**

`span` 与 `alpha` 互斥：给 `span` 时按 `alpha = 2/(span+1)` 换算；给 `alpha` 时直接使用。
`adjust` 固定为 False —— 公式与指标只接受「首值作种子」的口径。
"""

from __future__ import annotations

from quant1x import num

from .series import Series


class EWM:
    """**指数加权窗口** —— 绑定序列与平滑系数

    :param series: 源序列
    :param span: 周期，与 `alpha` 二选一
    :param alpha: 平滑系数，与 `span` 二选一
    :raises ValueError: 两者都缺或同时给出
    """

    __slots__ = ("_series", "_alpha")

    def __init__(self, series: Series, span: int | None = None, alpha: float | None = None):
        if (span is None) == (alpha is None):
            raise ValueError("EWM 需要且只能给出 span 或 alpha 之一")
        self._series = series
        self._alpha = float(alpha) if alpha is not None else 2.0 / (float(span) + 1.0)

    def mean(self) -> Series:
        """**指数加权均值**

        :return: 新序列
        """
        return Series(num.ewm_mean(self._series.values, self._alpha), name=self._series.name)
