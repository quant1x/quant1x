"""数值规格层 —— 用 NumPy 承载的 **可移植数值语义**

本包是 `quant1x.formula` / `quant1x.indicators` 的数值底座：把「窗口不足怎么算、NaN 怎么传播、
回归用哪种公式」这类口径收敛到一处，其它语言按同名语义对齐。

边界：本包**不**定义行情结构、不做 IO、不持有时间索引；容器与列对齐在 `quant1x.pandas`。
调用方传 `list` / `numpy.ndarray` / `pandas.Series` 均可，返回值统一是 `float64` 的 ndarray。

口径以 `num.md` 为准。
"""

from __future__ import annotations

from .align import align, pad_to, repeat
from .decimals import change_rate, decimal, net_change_rate
from .dtype import DEFAULT, NAN, DType, as_series, default_like, is_nan, to_scalar
from .elementwise import (
    abs,
    add,
    add_scalar,
    div,
    div_scalar,
    log,
    maximum,
    maximum_scalar,
    minimum,
    minimum_scalar,
    mul,
    mul_scalar,
    pow,
    sqrt,
    sub,
    sub_scalar,
    where,
)
from .ewm import ewm_mean
from .fillna import fill, fill_na, is_na
from .logic import (
    all_of,
    any_of,
    count_true,
    equal,
    gt,
    gte,
    logic_and,
    logic_not,
    logic_or,
    lt,
    lte,
    not_equal,
    to_bools,
)
from .matrix import arange, concat1d, linspace, ones, shape, transpose2d, zeros
from .reduce import (
    argmax,
    argmin,
    covariance,
    max_value,
    mean,
    median,
    min_value,
    std,
    sum_value,
    variance,
)
from .regression import linear_regression, forcast, slope
from .rolling import (
    rolling,
    rolling_apply,
    rolling_avedev,
    rolling_hhvbars,
    rolling_llvbars,
    rolling_max,
    rolling_mean,
    rolling_min,
    rolling_std,
    rolling_sum,
)
from .shift import cumsum, diff, ref, shift
from .window import as_window, window_at
from .ndarray import NDArray

__all__ = [
    "DEFAULT", "NAN", "DType", "as_series", "default_like", "is_nan", "to_scalar",
    "abs", "add", "add_scalar", "div", "div_scalar", "log", "maximum", "maximum_scalar",
    "minimum", "minimum_scalar", "mul", "mul_scalar", "pow", "sqrt", "sub", "sub_scalar", "where",
    "ewm_mean",
    "fill", "fill_na", "is_na",
    "all_of", "any_of", "count_true", "equal", "gt", "gte", "logic_and", "logic_not",
    "logic_or", "lt", "lte", "not_equal", "to_bools",
    "arange", "concat1d", "linspace", "ones", "shape", "transpose2d", "zeros",
    "argmax", "argmin", "covariance", "max_value", "mean", "median", "min_value", "std",
    "sum_value", "variance",
    "linear_regression", "forcast", "slope",
    "rolling", "rolling_apply", "rolling_avedev", "rolling_hhvbars", "rolling_llvbars",
    "rolling_max", "rolling_mean", "rolling_min", "rolling_std", "rolling_sum",
    "cumsum", "diff", "ref", "shift",
    "as_window", "window_at",
    "align", "pad_to", "repeat",
    "NDArray",
]
