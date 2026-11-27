"""列容器规格层 —— 用 pandas 承载的 **Series / DataFrame 门面**

本包只做容器与列对齐：取值、取列、写列、复制，以及把逐元素运算转给 `quant1x.num`。
滚动、指数加权、归约的语义都在 `quant1x.num`，这里不重复实现。

边界：不收行情结构（`quant1x.data.schema.Bar`）、不做 IO、不实现绘图。
口径见 `pandas.md`。
"""

from __future__ import annotations

from .dataframe import DataFrame
from .ewm import EWM
from .rolling import Rolling
from .series import Series

__all__ = ["DataFrame", "EWM", "Rolling", "Series"]
