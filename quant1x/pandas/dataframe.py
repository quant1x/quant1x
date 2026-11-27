"""DataFrame 门面 —— **列容器，不做数值计算**

`DataFrame` 只承担「取列、写列、复制、转 pandas」，列对齐与数值语义在
`quant1x.pandas.Series` 与 `quant1x.num`。指标函数从这里取 `open` / `close` 等列，
不直接依赖 pandas 的索引或对齐行为。
"""

from __future__ import annotations

import numpy as np
import pandas as pd

from .series import Series


class DataFrame:
    """**多列容器** —— 列名到 Series 的映射

    :param data: 字典、`pandas.DataFrame` 或 `DataFrame`
    """

    __slots__ = ("_frame",)

    def __init__(self, data=None):
        if isinstance(data, DataFrame):
            self._frame = data._frame.copy()
        elif isinstance(data, pd.DataFrame):
            self._frame = data.copy()
        else:
            self._frame = pd.DataFrame(data)

    @classmethod
    def from_pandas(cls, frame: pd.DataFrame) -> "DataFrame":
        """**从 pandas 构造**: 复制底层数据

        :param frame: `pandas.DataFrame`
        :return: 门面对象
        """
        return cls(frame)

    @property
    def columns(self) -> list[str]:
        """**列名列表**

        :return: 列名
        """
        return [str(name) for name in self._frame.columns]

    @property
    def shape(self) -> tuple[int, int]:
        """**形状**: `(行数, 列数)`

        :return: 形状
        """
        return tuple(self._frame.shape)

    def __len__(self) -> int:
        """**行数**

        :return: 行数
        """
        return len(self._frame)

    def __getitem__(self, name):
        """**取列**: 字符串返回 Series，列表返回 DataFrame

        :param name: 列名或列名列表
        :return: `Series` 或 `DataFrame`
        """
        if isinstance(name, (list, tuple)):
            return DataFrame(self._frame[list(name)])
        return Series._wrap(self._frame[name])

    def __setitem__(self, name: str, value) -> None:
        """**写列**: 接受 Series、数组或标量

        :param name: 列名
        :param value: 列数据
        """
        if isinstance(value, Series):
            self._frame[name] = value.to_pandas()
            return
        array = np.asarray(value)
        self._frame[name] = pd.Series(array, index=self._frame.index)
        return

    def copy(self) -> "DataFrame":
        """**复制**

        :return: 独立副本
        """
        return DataFrame(self._frame.copy())

    def to_pandas(self) -> pd.DataFrame:
        """**转 pandas**: 返回底层表格的副本

        :return: `pandas.DataFrame`
        """
        return self._frame.copy()

    def __repr__(self) -> str:
        """**文本表示**

        :return: 形状与列名
        """
        return f"DataFrame(shape={self.shape}, columns={self.columns})"
