"""指数加权移动平均 —— **NaN 跳过，首个有效值作种子**

递推式固定为 `y_t = (1 - alpha) * y_{t-1} + alpha * x_t`，`y_0 = x_0`（`adjust=False` 口径）。

缺失值按 pandas 的 `ewm` 约定处理：跳过 NaN，输出沿用上一有效值；首个有效值之前仍为 NaN。
首值作为种子、不做 `1/n` 修正 —— 这是通达信 `EMA` / `SMA` 的行为，也是本包的唯一口径。

实现刻意拆成三个乘加步骤，避免编译器把 `a*b + c*d` 收缩成 FMA 造成跨语言末位差异。
"""

from __future__ import annotations

import numpy as np

from .dtype import NAN, as_series


def ewm_mean(values, alpha: float) -> np.ndarray:
    """**指数加权均值**: `y_t = (1-alpha) * y_{t-1} + alpha * x_t`

    :param values: 输入序列
    :param alpha: 平滑系数，取值 `0 < alpha <= 1`
    :return: 指数加权均值序列
    """
    data = as_series(values)
    out = np.full(len(data), NAN, dtype=np.float64)
    previous = NAN
    seeded = False
    decay = 1.0 - alpha
    for index, value in enumerate(data):
        current = float(value)
        if np.isnan(current):
            if seeded:
                out[index] = previous
            continue
        if not seeded:
            previous = current
            seeded = True
            out[index] = previous
            continue
        weighted_previous = decay * previous
        weighted_current = alpha * current
        previous = weighted_previous + weighted_current
        out[index] = previous
    return out
