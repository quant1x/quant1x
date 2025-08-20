#!/usr/bin/python
# -*- coding: UTF-8 -*-
from __future__ import annotations

from datetime import timedelta
from pydoc import doc
from typing import final, Callable, Any

import numpy as np
import pandas
from pandas._libs.tslibs import BaseOffset
from pandas._typing import Dtype, Axis
from pandas.core.generic import bool_t
from pandas.core.indexers.objects import BaseIndexer
from pandas.core.window import Rolling, Window


class Series(pandas.Series):
    """
    扩展pandas.Series
    """

    def __init__(self,
                 data=None,
                 index=None,
                 dtype: Dtype | None = None,
                 name=None,
                 copy: bool = False,
                 fastpath: bool = False,
                 ) -> None:
        super().__init__(data=data, index=index, dtype=dtype, name=name, copy=copy, fastpath=fastpath)

    def rolling(
            self,
            window: int | timedelta | BaseOffset | BaseIndexer,
            min_periods: int | None = None,
            center: bool_t = False,
            win_type: str | None = None,
            on: str | None = None,
            axis: Axis = 0,
            closed: str | None = None,
            step: int | None = None,
            method: str = "single",
    ) -> Window | Rolling:
        axis = self._get_axis_number(axis)

        if win_type is not None:
            return Window(
                self,
                window=window,
                min_periods=min_periods,
                center=center,
                win_type=win_type,
                on=on,
                axis=axis,
                closed=closed,
                step=step,
                method=method,
            )

        return Rolling(
            self,
            window=window,
            min_periods=min_periods,
            center=center,
            win_type=win_type,
            on=on,
            axis=axis,
            closed=closed,
            step=step,
            method=method,
        )

    def rolling_apply(self, N, func: Callable[..., Any], raw: bool = False):
        """
        Rolling(N).apply()的序列化版本
        :param S:
        :param N:
        :param func:
        :param raw:
        :return:
        """
        S = self.values
        s_len = len(S)
        ret = np.repeat(np.nan, s_len)
        pos = 0
        for i in range(s_len):
            pos = i
            if (not np.isnan(N[i])) and N[i] <= i + 1:
                window = N[i]
                T = S[i + 1 - window:i + 1]
                ret[pos] = func(T, window)
        return ret
