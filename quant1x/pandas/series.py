"""Series 门面 —— **一列同构数据，数值算子委托 quant1x.num**

`Series` 是 `pandas.Series` 的轻量包装：只保留「取值、取长度、移位、滚动、指数加权、
逐元素运算」这些公式与指标真正用到的能力，底层容器仍是 pandas。

数值语义不在这里定义：滚动、EWM、归约都转调 `quant1x.num`，保证与其它语言同一口径。
布尔序列单独保留 `bool`  dtype（比较、逻辑运算的结果），不强行转成 0/1。
"""

from __future__ import annotations

import numpy as np
import pandas as pd

from quant1x import num


class Series:
    """**一列数据** —— 值 + 名称

    :param data: 一维序列、标量或 `pandas.Series`
    :param name: 列名
    """

    __slots__ = ("_series",)

    def __init__(self, data=None, name: str | None = None):
        if isinstance(data, Series):
            self._series = data._series.copy()
            if name is not None:
                self._series.name = name
            return
        if isinstance(data, pd.Series):
            self._series = data.copy()
            if name is not None:
                self._series.name = name
            return
        self._series = pd.Series(data, name=name)

    @classmethod
    def _wrap(cls, series: pd.Series) -> "Series":
        """**包装已有 Series**: 不复制底层数据

        :param series: pandas 序列
        :return: 门面对象
        """
        instance = cls.__new__(cls)
        instance._series = series
        return instance

    @property
    def values(self) -> np.ndarray:
        """**取值**: 返回底层 ndarray（数值列为 float64，布尔列为 bool）

        :return: 一维数组
        """
        array = self._series.to_numpy()
        if array.dtype == np.bool_:
            return array
        return array.astype(np.float64, copy=False)

    @property
    def dtypes(self):
        """**元素类型**: 底层 pandas dtype

        :return: dtype
        """
        return self._series.dtype

    @property
    def name(self) -> str | None:
        """**列名**

        :return: 列名或 None
        """
        return self._series.name

    @property
    def size(self) -> int:
        """**长度**

        :return: 元素个数
        """
        return len(self._series)

    def __len__(self) -> int:
        """**长度**

        :return: 元素个数
        """
        return len(self._series)

    def __getitem__(self, index):
        """**按位置取值**

        :param index: 位置
        :return: 元素
        """
        return self._series.iloc[index]

    def __setitem__(self, index, value) -> None:
        """**按位置赋值**

        :param index: 位置
        :param value: 新值
        """
        self._series.iloc[index] = value

    def copy(self) -> "Series":
        """**复制**

        :return: 独立副本
        """
        return Series._wrap(self._series.copy())

    def to_numpy(self) -> np.ndarray:
        """**转 ndarray**: 数值列转 float64

        :return: 一维数组
        """
        return self.values

    def to_pandas(self) -> pd.Series:
        """**转 pandas**: 返回底层序列的副本

        :return: `pandas.Series`
        """
        return self._series.copy()

    def shift(self, periods: int = 1) -> "Series":
        """**移位**: 取 N 周期前的值，越界为 NaN

        :param periods: 周期数
        :return: 新序列
        """
        return Series._wrap(pd.Series(num.shift(self.values, periods), name=self.name))

    def diff(self, periods: int = 1) -> "Series":
        """**差分**: 本值减 N 周期前的值

        :param periods: 周期数
        :return: 新序列
        """
        return Series._wrap(pd.Series(num.diff(self.values, periods), name=self.name))

    def cumsum(self) -> "Series":
        """**累计和**

        :return: 新序列
        """
        return Series._wrap(pd.Series(num.cumsum(self.values), name=self.name))

    def fill_na(self, value: float = 0.0) -> "Series":
        """**填充缺失值**

        :param value: 替换值
        :return: 新序列
        """
        return Series._wrap(pd.Series(num.fill_na(self.values, value), name=self.name))

    def rolling(self, window: int):
        """**滚动窗口**: 返回 `Rolling` 对象

        :param window: 窗口长度
        :return: `Rolling`
        """
        from .rolling import Rolling

        return Rolling(self, window)

    def ewm(self, span: int | None = None, alpha: float | None = None):
        """**指数加权**: 返回 `EWM` 对象

        :param span: 周期，`alpha = 2/(span+1)`
        :param alpha: 直接给定平滑系数
        :return: `EWM`
        """
        from .ewm import EWM

        return EWM(self, span=span, alpha=alpha)

    def apply(self, func) -> "Series":
        """**逐元素映射**

        :param func: 单元函数
        :return: 新序列
        """
        return Series._wrap(self._series.map(func))

    def _binary(self, other, op) -> "Series":
        """**二元运算**: 两侧都收敛成数组后逐元素计算

        :param other: 另一操作数
        :param op: 二元函数
        :return: 新序列
        """
        right = other.values if isinstance(other, Series) else other
        return Series._wrap(pd.Series(op(self.values, right), name=self.name))

    def __add__(self, other) -> "Series":
        """**加**

        :param other: 另一操作数
        :return: 新序列
        """
        return self._binary(other, num.add)

    def __radd__(self, other) -> "Series":
        """**右加**

        :param other: 另一操作数
        :return: 新序列
        """
        return self._binary(other, lambda a, b: num.add(b, a))

    def __sub__(self, other) -> "Series":
        """**减**

        :param other: 另一操作数
        :return: 新序列
        """
        return self._binary(other, num.sub)

    def __rsub__(self, other) -> "Series":
        """**右减**

        :param other: 另一操作数
        :return: 新序列
        """
        return self._binary(other, lambda a, b: num.sub(b, a))

    def __mul__(self, other) -> "Series":
        """**乘**

        :param other: 另一操作数
        :return: 新序列
        """
        return self._binary(other, num.mul)

    def __rmul__(self, other) -> "Series":
        """**右乘**

        :param other: 另一操作数
        :return: 新序列
        """
        return self._binary(other, lambda a, b: num.mul(b, a))

    def __truediv__(self, other) -> "Series":
        """**除**

        :param other: 另一操作数
        :return: 新序列
        """
        return self._binary(other, num.div)

    def __rtruediv__(self, other) -> "Series":
        """**右除**

        :param other: 另一操作数
        :return: 新序列
        """
        return self._binary(other, lambda a, b: num.div(b, a))

    def __gt__(self, other) -> "Series":
        """**大于**

        :param other: 另一操作数
        :return: 布尔序列
        """
        return self._binary(other, num.gt)

    def __ge__(self, other) -> "Series":
        """**大于等于**

        :param other: 另一操作数
        :return: 布尔序列
        """
        return self._binary(other, num.gte)

    def __lt__(self, other) -> "Series":
        """**小于**

        :param other: 另一操作数
        :return: 布尔序列
        """
        return self._binary(other, num.lt)

    def __le__(self, other) -> "Series":
        """**小于等于**

        :param other: 另一操作数
        :return: 布尔序列
        """
        return self._binary(other, num.lte)

    def __eq__(self, other) -> "Series":
        """**等于**

        :param other: 另一操作数
        :return: 布尔序列
        """
        return self._binary(other, num.equal)

    def __ne__(self, other) -> "Series":
        """**不等于**

        :param other: 另一操作数
        :return: 布尔序列
        """
        return self._binary(other, num.not_equal)

    def __and__(self, other) -> "Series":
        """**逻辑与**

        :param other: 另一操作数
        :return: 布尔序列
        """
        return self._binary(other, num.logic_and)

    def __or__(self, other) -> "Series":
        """**逻辑或**

        :param other: 另一操作数
        :return: 布尔序列
        """
        return self._binary(other, num.logic_or)

    def __invert__(self) -> "Series":
        """**逻辑非**

        :return: 布尔序列
        """
        return Series._wrap(pd.Series(num.logic_not(self.values), name=self.name))

    def __repr__(self) -> str:
        """**文本表示**

        :return: 名称与长度
        """
        return f"Series(name={self.name!r}, size={self.size})"
