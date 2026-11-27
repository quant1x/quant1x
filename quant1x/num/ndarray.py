"""ndarray —— **对标 np.ndarray 的可移植一维容器**

numpy 就是 spec。本模块把「一维数值数组」收敛成一个**方法集合固定、语义可逐行复刻**的
容器 `NDArray`：Python 用 numpy 承载（存储与互转），其它语言（C++ / Rust / Go / Zig）按同一张
方法表用显式循环实现，从而让研究代码能落到生产。

- **契约以命名方法为准**（`add` / `mean` / `rolling_mean` …）：Go 没有运算符重载，其它语言只需
  照抄方法；Python 额外提供运算符（`+` / `>` / `&` …）作为糖，一一映射到方法（见 `ndarray.md`）。
- 数值口径（窗口、NaN、归约顺序、回归、舍入）全部委托 `quant1x.num` 的自由函数，本类只做容器封装，
  不另写算法。
- 只支持一维 `float64` / `bool`；不实现 N 维广播、花式索引、复数/字符串 dtype。
"""

from __future__ import annotations

import numpy as np

from . import ewm as _ewm
from . import elementwise as _ew
from . import fillna as _fn
from . import logic as _lg
from . import matrix as _mx
from . import reduce as _rd
from .rolling import (
    rolling_apply, rolling_avedev, rolling_max, rolling_mean, rolling_min, rolling_std, rolling_sum,
)
from .shift import cumsum, diff, shift

#: 支持的元素类型
_DTYPES = ("float64", "bool")


def _as_array(data) -> np.ndarray:
    """**收敛为一维数组**: list / ndarray / NDArray / 标量 → 1-D

    :param data: 输入
    :return: 一维 numpy 数组（float64 或 bool）
    """
    if isinstance(data, NDArray):
        array = data._data
    elif isinstance(data, np.ndarray):
        array = data
    else:
        array = np.asarray(data)
    if array.ndim == 0:
        array = array.reshape(1)
    if array.ndim != 1:
        raise ValueError(f"NDArray 只支持一维数组, 收到 ndim={array.ndim}")
    if array.dtype == np.bool_:
        return array.astype(np.bool_, copy=True)
    return array.astype(np.float64, copy=False)


class NDArray:
    """**一维数值数组** —— 方法集合即跨语言契约"""

    __slots__ = ("_data",)

    def __init__(self, data):
        self._data = _as_array(data)

    # -- 构造 -------------------------------------------------------------
    @classmethod
    def zeros(cls, length: int) -> "NDArray":
        """**全零**"""
        return cls(_mx.zeros(length))

    @classmethod
    def ones(cls, length: int) -> "NDArray":
        """**全一**"""
        return cls(_mx.ones(length))

    @classmethod
    def full(cls, length: int, value: float) -> "NDArray":
        """**定值填充**"""
        return cls(np.full(length, value, dtype=np.float64))

    @classmethod
    def arange(cls, start: float, stop: float | None = None, step: float = 1.0) -> "NDArray":
        """**等差序列**"""
        return cls(_mx.arange(start, stop, step))

    @classmethod
    def linspace(cls, start: float, stop: float, count: int) -> "NDArray":
        """**等分序列**"""
        return cls(_mx.linspace(start, stop, count))

    # -- 属性 -------------------------------------------------------------
    @property
    def size(self) -> int:
        """**元素个数**"""
        return int(self._data.size)

    @property
    def shape(self) -> tuple[int, ...]:
        """**形状**（恒为一维）"""
        return (int(self._data.size),)

    @property
    def ndim(self) -> int:
        """**维度**（恒为 1）"""
        return 1

    @property
    def dtype(self) -> str:
        """**元素类型**: `float64` 或 `bool`"""
        return "bool" if self._data.dtype == np.bool_ else "float64"

    @property
    def values(self) -> np.ndarray:
        """**底层数组**（副本）"""
        return self._data.copy()

    def to_numpy(self) -> np.ndarray:
        """**转 numpy**: 返回副本"""
        return self._data.copy()

    def to_list(self) -> list:
        """**转 list**"""
        return self._data.tolist()

    def copy(self) -> "NDArray":
        """**复制**"""
        return NDArray(self._data.copy())

    def slice(self, start: int, end: int) -> "NDArray":
        """**切片**: 返回新数组"""
        return NDArray(self._data[start:end])

    def astype(self, dtype: str) -> "NDArray":
        """**类型转换**: 仅支持 `float64` / `bool`

        :param dtype: 目标类型
        :return: 新数组
        """
        if dtype not in _DTYPES:
            raise ValueError(f"不支持的 dtype: {dtype}")
        target = np.bool_ if dtype == "bool" else np.float64
        return NDArray(self._data.astype(target, copy=True))

    def __array__(self, dtype=None):
        return self._data.astype(dtype, copy=False) if dtype is not None else self._data

    # -- 序列协议 ---------------------------------------------------------
    def __len__(self) -> int:
        return int(self._data.size)

    def __iter__(self):
        return iter(self._data.tolist())

    def __getitem__(self, index):
        if isinstance(index, slice):
            return NDArray(self._data[index])
        return self._data[index].item()

    def __setitem__(self, index, value) -> None:
        if isinstance(index, slice):
            self._data[index] = _as_array(value)._data
        else:
            self._data[index] = value

    def __repr__(self) -> str:
        return f"NDArray({self._data.tolist()!r}, dtype='{self.dtype}')"

    # -- 内部 -------------------------------------------------------------
    def _float(self) -> np.ndarray:
        """收敛为 float64 数组（布尔转 0/1）。"""
        return _as_array(self._data.astype(np.float64, copy=False))

    def _other_float(self, other) -> np.ndarray:
        if isinstance(other, NDArray):
            return other._float()
        if np.isscalar(other):
            return np.full(self._data.size, float(other), dtype=np.float64)
        return _as_array(np.asarray(other, dtype=np.float64))

    # -- 四则（方法即契约） ----------------------------------------------
    def add(self, other) -> "NDArray":
        """**逐元素加**"""
        if np.isscalar(other):
            return NDArray(_ew.add_scalar(self._float(), float(other)))
        return NDArray(_ew.add(self._float(), self._other_float(other)))

    def sub(self, other) -> "NDArray":
        """**逐元素减**"""
        if np.isscalar(other):
            return NDArray(_ew.sub_scalar(self._float(), float(other)))
        return NDArray(_ew.sub(self._float(), self._other_float(other)))

    def mul(self, other) -> "NDArray":
        """**逐元素乘**"""
        if np.isscalar(other):
            return NDArray(_ew.mul_scalar(self._float(), float(other)))
        return NDArray(_ew.mul(self._float(), self._other_float(other)))

    def div(self, other) -> "NDArray":
        """**逐元素除**"""
        if np.isscalar(other):
            return NDArray(_ew.div_scalar(self._float(), float(other)))
        return NDArray(_ew.div(self._float(), self._other_float(other)))

    def maximum(self, other) -> "NDArray":
        """**逐元素取大**"""
        if np.isscalar(other):
            return NDArray(_ew.maximum_scalar(self._float(), float(other)))
        return NDArray(_ew.maximum(self._float(), self._other_float(other)))

    def minimum(self, other) -> "NDArray":
        """**逐元素取小**"""
        if np.isscalar(other):
            return NDArray(_ew.minimum_scalar(self._float(), float(other)))
        return NDArray(_ew.minimum(self._float(), self._other_float(other)))

    # -- 一元数学 ---------------------------------------------------------
    def abs(self) -> "NDArray":
        """**绝对值**"""
        return NDArray(_ew.abs(self._float()))

    def sqrt(self) -> "NDArray":
        """**平方根**"""
        return NDArray(_ew.sqrt(self._float()))

    def log(self) -> "NDArray":
        """**自然对数**"""
        return NDArray(_ew.log(self._float()))

    def pow(self, exponent: float) -> "NDArray":
        """**幂**"""
        return NDArray(_ew.pow(self._float(), exponent))

    def __neg__(self) -> "NDArray":
        return NDArray(_ew.mul_scalar(self._float(), -1.0))

    # -- 比较 -------------------------------------------------------------
    def gt(self, other) -> "NDArray":
        """**大于**"""
        return NDArray(_lg.gt(self._float(), self._other_float(other)))

    def gte(self, other) -> "NDArray":
        """**大于等于**"""
        return NDArray(_lg.gte(self._float(), self._other_float(other)))

    def lt(self, other) -> "NDArray":
        """**小于**"""
        return NDArray(_lg.lt(self._float(), self._other_float(other)))

    def lte(self, other) -> "NDArray":
        """**小于等于**"""
        return NDArray(_lg.lte(self._float(), self._other_float(other)))

    def equal(self, other) -> "NDArray":
        """**相等**"""
        return NDArray(_lg.equal(self._float(), self._other_float(other)))

    def not_equal(self, other) -> "NDArray":
        """**不等**"""
        return NDArray(_lg.not_equal(self._float(), self._other_float(other)))

    # -- 逻辑（布尔数组） -------------------------------------------------
    def logic_and(self, other) -> "NDArray":
        """**逻辑与**"""
        right = other._data if isinstance(other, NDArray) else np.asarray(other)
        return NDArray(_lg.logic_and(self._data, right))

    def logic_or(self, other) -> "NDArray":
        """**逻辑或**"""
        right = other._data if isinstance(other, NDArray) else np.asarray(other)
        return NDArray(_lg.logic_or(self._data, right))

    def logic_not(self) -> "NDArray":
        """**逻辑非**"""
        return NDArray(_lg.logic_not(self._data))

    # -- 归约 -------------------------------------------------------------
    def sum(self) -> float:
        """**求和**"""
        return float(_rd.sum_value(self._float()))

    def mean(self) -> float:
        """**均值**"""
        return float(_rd.mean(self._float()))

    def median(self) -> float:
        """**中位数**"""
        return float(_rd.median(self._float()))

    def min(self) -> float:
        """**最小值**"""
        return float(_rd.min_value(self._float()))

    def max(self) -> float:
        """**最大值**"""
        return float(_rd.max_value(self._float()))

    def argmin(self) -> int:
        """**最小值下标**"""
        return int(_rd.argmin(self._float()))

    def argmax(self) -> int:
        """**最大值下标**"""
        return int(_rd.argmax(self._float()))

    def var(self) -> float:
        """**总体方差**"""
        return float(_rd.variance(self._float()))

    def std(self) -> float:
        """**总体标准差**"""
        return float(_rd.std(self._float()))

    def cumsum(self) -> "NDArray":
        """**累计和**"""
        return NDArray(cumsum(self._float()))

    # -- 移位 / 差分 / 填充 ----------------------------------------------
    def shift(self, periods: int = 1) -> "NDArray":
        """**移位**: 越界补 NaN"""
        return NDArray(shift(self._float(), periods))

    def diff(self, periods: int = 1) -> "NDArray":
        """**差分**"""
        return NDArray(diff(self._float(), periods))

    def fill_na(self, value: float = 0.0) -> "NDArray":
        """**填充缺失值**"""
        return NDArray(_fn.fill_na(self._float(), value))

    # -- 滚动 / EWM -------------------------------------------------------
    def rolling_sum(self, window: int) -> "NDArray":
        """**滚动求和**"""
        return NDArray(rolling_sum(self._float(), window))

    def rolling_mean(self, window: int) -> "NDArray":
        """**滚动均值**"""
        return NDArray(rolling_mean(self._float(), window))

    def rolling_std(self, window: int) -> "NDArray":
        """**滚动总体标准差**"""
        return NDArray(rolling_std(self._float(), window))

    def rolling_min(self, window: int) -> "NDArray":
        """**滚动最小值**"""
        return NDArray(rolling_min(self._float(), window))

    def rolling_max(self, window: int) -> "NDArray":
        """**滚动最大值**"""
        return NDArray(rolling_max(self._float(), window))

    def rolling_avedev(self, window: int) -> "NDArray":
        """**滚动平均绝对偏差**"""
        return NDArray(rolling_avedev(self._float(), window))

    def rolling_apply(self, window: int, func) -> "NDArray":
        """**逐窗口归约**"""
        return NDArray(rolling_apply(self._float(), window, func))

    def ewm_mean(self, alpha: float) -> "NDArray":
        """**指数加权均值**"""
        return NDArray(_ewm.ewm_mean(self._float(), alpha))

    # -- 运算符糖（映射到上面的方法） ------------------------------------
    __add__ = add
    __sub__ = sub
    __mul__ = mul
    __truediv__ = div
    __gt__ = gt
    __ge__ = gte
    __lt__ = lt
    __le__ = lte
    __eq__ = equal
    __ne__ = not_equal
    __and__ = logic_and
    __or__ = logic_or
    __invert__ = logic_not

    def __radd__(self, other) -> "NDArray":
        return self.add(other)

    def __rsub__(self, other) -> "NDArray":
        return NDArray(_ew.sub(self._other_float(other), self._float()))

    def __rmul__(self, other) -> "NDArray":
        return self.mul(other)

    def __rtruediv__(self, other) -> "NDArray":
        return NDArray(_ew.div(self._other_float(other), self._float()))
