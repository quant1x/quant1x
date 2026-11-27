# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

"""通达信公式函数 —— **麦语言 / 同花顺指标语义的 Python 规格**

本模块实现 0 级（REF / HHV / MA …）、1 级（COUNT / CROSS / BARSLAST …）与 2 级（MACD /
KDJ / RSI …）公式函数。所有算子都走 `quant1x.num`，口径见 `formula.md`：

- 滚动窗口必须完整且不含 NaN；窗口参数的序列版本逐位置生效；
- `EMA` / `SMA` 为 `y_t = (1-alpha)·y_{t-1} + alpha·x_t`，首值作种子，NaN 跳过；
- `SLOPE` / `FORCAST` 用均值中心化最小二乘闭式解，不用 `numpy.polyfit`；
- `RD` 为半程远离零，不是银行家的四舍六入五成双。

边界：本模块只做「序列 → 序列」的纯计算，不读写行情、不持有 DataFrame；输入可以是
`list` / `numpy.ndarray` / `pandas.Series`，输出统一是 `numpy.ndarray`。

原始 MyTT 实现见 https://github.com/mpquant/MyTT 。
"""

from __future__ import annotations

from decimal import Decimal, ROUND_HALF_UP
from typing import Any, Callable

import numpy as np
import pandas as pd

from quant1x import num

# 通达信公式大量使用除零表达「无意义 / 无穷」，结果由 NaN 与 inf 承载；
# 这里关掉 NumPy 的除零与无效值告警，避免指标批量计算时刷屏。
np.seterr(divide="ignore", invalid="ignore")

# 以下所有函数如无特别说明, 输入参数S均为numpy序列或者列表list, N为整型int
# 应用层1级函数完美兼容通达信或同花顺, 具体使用方法请参考通达信


# ------------------ 0级: 核心工具函数 --------------------------------------------
def RD(N, D=3):
    """**四舍五入**: 半程远离零，保留 D 位小数

    :param N: 标量或序列
    :param D: 小数位
    :return: 舍入后的值
    """
    return num.decimal(N, D)


def RET(S, N=1):
    """**取倒数第 N 个值**: 缺省取最后一个

    :param S: 序列
    :param N: 倒数位置，从 1 开始
    :return: 标量
    """
    return np.asarray(S)[-N]


def ABS(S):
    """**绝对值**: 逐元素 `|S|`

    :param S: 序列
    :return: 绝对值序列
    """
    return num.abs(S)


def LN(S):
    """**自然对数**: 逐元素 `ln(S)`

    :param S: 序列
    :return: 对数序列
    """
    return num.log(S)


def POW(S, N):
    """**幂**: 逐元素 `S ** N`

    :param S: 底数
    :param N: 指数
    :return: 幂序列
    """
    return num.pow(S, N)


def SQRT(S):
    """**平方根**: 逐元素 `sqrt(S)`

    :param S: 序列
    :return: 平方根序列
    """
    return num.sqrt(S)


def MAX(S1, S2):
    """**序列取大**: 逐元素较大值

    :param S1: 左操作数
    :param S2: 右操作数
    :return: 逐元素较大值
    """
    return num.maximum(S1, S2)


def MIN(S1, S2):
    """**序列取小**: 逐元素较小值

    :param S1: 左操作数
    :param S2: 右操作数
    :return: 逐元素较小值
    """
    return num.minimum(S1, S2)


def IF(S, A, B):
    """**条件选择**: S 为真取 A，否则取 B

    :param S: 条件
    :param A: 真值
    :param B: 假值
    :return: 选择结果
    """
    return num.where(S, A, B)


def IFF(S, A, B):
    """**条件选择**: `IF` 的同义名

    :param S: 条件
    :param A: 真值
    :param B: 假值
    :return: 选择结果
    """
    return num.where(S, A, B)


def IFN(S, A, B):
    """**反向条件选择**: S 为假取 A，否则取 B

    :param S: 条件
    :param A: 条件为假时的取值
    :param B: 条件为真时的取值
    :return: 选择结果
    """
    return num.where(num.logic_not(S), A, B)


def rolling_apply(S, N, func: Callable[..., Any], raw: bool = False):
    """**逐窗口归约**: `Rolling(N).apply()` 的序列化版本

    :param S: 序列
    :param N: 窗口，常量或逐位置序列
    :param func: 接收 `(窗口, 窗口长度)` 的归约函数
    :param raw: 保留参数以对齐 pandas 签名，窗口恒按 ndarray 传入
    :return: 归约结果序列
    """
    return num.rolling_apply(S, N, lambda block: func(block, len(block)))


def REF(S, N=1):
    """**引用 N 周期前的值**: 常量 N 整体后移，序列 N 逐位置取

    :param S: 序列
    :param N: 周期，常量或逐位置序列
    :return: 引用序列，越界位置为 NaN
    """
    if isinstance(N, (int, float, np.integer, np.floating)):
        return num.shift(S, int(N))

    data = num.as_series(S)
    periods = np.asarray(N, dtype=np.float64)
    out = np.full(len(data), np.nan, dtype=np.float64)
    for index in range(len(data)):
        lag = periods[index]
        if np.isnan(lag) or lag > index:
            continue
        out[index] = data[index - int(lag)]
    return out


def DIFF(S, N=1):
    """**N 周期差分**: 本值减 N 周期前的值

    :param S: 序列
    :param N: 周期
    :return: 差分序列，前 N 个位置为 NaN
    """
    return num.diff(S, int(N))


def STD(S, N):
    """**N 日标准差**: 总体口径

    :param S: 序列
    :param N: 窗口
    :return: 标准差序列
    """
    return num.rolling_std(S, N)


def SUM(S, N):
    """**N 日累计和**: N 为 0 时从首个值开始累计

    :param S: 序列
    :param N: 窗口，常量或逐位置序列
    :return: 求和序列
    """
    if isinstance(N, (int, float, np.integer, np.floating)):
        if int(N) > 0:
            return num.rolling_sum(S, int(N))
        return num.cumsum(S)

    data = num.as_series(S)
    windows = np.asarray(N, dtype=np.float64)
    out = np.full(len(data), np.nan, dtype=np.float64)
    for index in range(len(data)):
        size = windows[index]
        if np.isnan(size) or size > index + 1:
            continue
        size = int(size)
        if size > 0:
            total = 0.0
            for value in data[index + 1 - size:index + 1]:
                total += float(value)
            out[index] = total
        else:
            total = 0.0
            for value in data[: index + 1]:
                total += float(value)
            out[index] = total
    return out


def CONST(S):
    """**常量序列**: 用最后一个值铺满整条序列

    :param S: 序列
    :return: 常量序列
    """
    data = num.as_series(S)
    return np.full(len(data), data[-1], dtype=np.float64)


def HHV(S, N):
    """**N 周期最高值**

    :param S: 序列
    :param N: 窗口，常量或逐位置序列
    :return: 最高值序列
    """
    if isinstance(N, (int, float, np.integer, np.floating)):
        return num.rolling_max(S, int(N))

    data = num.as_series(S)
    windows = np.asarray(N, dtype=np.float64)
    out = np.full(len(data), np.nan, dtype=np.float64)
    for index in range(len(data)):
        size = windows[index]
        if np.isnan(size) or size > index + 1:
            continue
        size = int(size)
        if size <= 0:
            continue
        block = data[index + 1 - size:index + 1]
        if np.isnan(block).any():
            continue
        result = float(block[0])
        for value in block:
            if float(value) > result:
                result = float(value)
        out[index] = result
    return out


def LLV(S, N):
    """**N 周期最低值**

    :param S: 序列
    :param N: 窗口，常量或逐位置序列
    :return: 最低值序列
    """
    if isinstance(N, (int, float, np.integer, np.floating)):
        return num.rolling_min(S, int(N))

    data = num.as_series(S)
    windows = np.asarray(N, dtype=np.float64)
    out = np.full(len(data), np.nan, dtype=np.float64)
    for index in range(len(data)):
        size = windows[index]
        if np.isnan(size) or size > index + 1:
            continue
        size = int(size)
        if size <= 0:
            continue
        block = data[index + 1 - size:index + 1]
        if np.isnan(block).any():
            continue
        result = float(block[0])
        for value in block:
            if float(value) < result:
                result = float(value)
        out[index] = result
    return out


def HHVBARS(S, N):
    """**最高值到当前的周期数**: 并列取最近的一个

    :param S: 序列
    :param N: 窗口
    :return: 周期数序列
    """
    return num.rolling_hhvbars(S, N)


def LLVBARS(S, N):
    """**最低值到当前的周期数**: 并列取最近的一个

    :param S: 序列
    :param N: 窗口
    :return: 周期数序列
    """
    return num.rolling_llvbars(S, N)


def MA(S, N):
    """**N 日简单移动平均**

    :param S: 序列
    :param N: 窗口，常量或逐位置序列
    :return: 均值序列
    """
    return num.rolling_mean(S, N)


def V1_MA(S, N):
    """**N 日简单移动平均**: `MA` 的同义实现

    :param S: 序列
    :param N: 窗口
    :return: 均值序列
    """
    return num.rolling_mean(S, N)


def EMA(S, N):
    """**指数移动平均**: `alpha = 2/(N+1)`

    :param S: 序列
    :param N: 周期
    :return: 均值序列
    """
    return num.ewm_mean(S, 2.0 / (float(N) + 1.0))


def SMA(S, N, M=1):
    """**中国式 SMA**: `alpha = M/N`

    :param S: 序列
    :param N: 周期
    :param M: 权重，缺省 1
    :return: 均值序列
    """
    if int(N) == 0:
        return num.ewm_mean(S, 1.0)
    return num.ewm_mean(S, float(M) / float(N))


def WMA(S, N):
    """**加权移动平均**: `Yn = (1·X1 + 2·X2 + … + n·Xn) / (1+2+…+n)`

    :param S: 序列
    :param N: 窗口
    :return: 加权均值序列
    """
    data = num.as_series(S)
    windows = num.as_window(N, len(data))

    def weighted(block) -> float:
        total = 0.0
        weighted_total = 0.0
        for position, value in enumerate(block):
            weight = float(position + 1)
            weighted_total += weight * float(value)
            total += weight
        return weighted_total / total

    return num.rolling_apply(data, windows, weighted)


def DMA(S, A):
    """**动态移动平均**: `Y = A·S + (1-A)·Y'`

    :param S: 序列
    :param A: 平滑因子，常量或逐位置序列，必须 `0 < A < 1`
    :return: 动态均值序列
    """
    if isinstance(A, (int, float, np.integer, np.floating)):
        return num.ewm_mean(S, float(A))

    data = num.as_series(S)
    weights = np.array(A, dtype=np.float64, copy=True)
    weights[np.isnan(weights)] = 1.0
    out = np.empty(len(data), dtype=np.float64)
    if len(data) == 0:
        return out
    out[0] = data[0]
    for index in range(1, len(data)):
        out[index] = weights[index] * data[index] + (1.0 - weights[index]) * out[index - 1]
    return out


def AVEDEV(S, N):
    """**平均绝对偏差**: 窗口内各值与窗口均值的绝对差的平均

    :param S: 序列
    :param N: 窗口
    :return: 平均绝对偏差序列
    """
    return num.rolling_avedev(S, N)


def V1_AVEDEV(S, N):
    """**平均绝对偏差**: `AVEDEV` 的同义实现

    :param S: 序列
    :param N: 窗口
    :return: 平均绝对偏差序列
    """
    return num.rolling_avedev(S, N)


def SLOPE(S, N):
    """**线性回归斜率**: 窗口对 `0..N-1` 回归

    :param S: 序列
    :param N: 窗口
    :return: 斜率序列
    """
    return num.rolling_apply(S, N, num.slope)


def FORCAST(S, N):
    """**线性回归预测值**: 回归线在窗口末端的取值

    :param S: 序列
    :param N: 窗口，常量或逐位置序列
    :return: 预测值序列
    """
    return num.rolling_apply(S, N, num.forcast)


def V2_FORCAST(S, N):
    """**线性回归预测值**: `FORCAST` 的同义实现

    :param S: 序列
    :param N: 窗口
    :return: 预测值序列
    """
    return FORCAST(S, N)


def V1_FORCAST(S, N):
    """**线性回归预测值**: `FORCAST` 的同义实现

    :param S: 序列
    :param N: 窗口
    :return: 预测值序列
    """
    return FORCAST(S, N)


def LAST(S, A, B):
    """**区间条件**: 从前 A 日到前 B 日是否一直满足 S

    :param S: 条件序列
    :param A: 远端周期，要求 `A > B`
    :param B: 近端周期，要求 `B >= 0`
    :return: 布尔序列
    """
    data = np.asarray(S) != 0
    windows = int(A) + 1
    out = np.zeros(len(data), dtype=np.bool_)
    for index in range(len(data)):
        if index + 1 < windows:
            continue
        block = data[index + 1 - windows:index + 1][::-1]
        out[index] = bool(np.all(block[int(B):]))
    return out


# ------------------   1级: 应用层函数(通过0级核心函数实现)使用方法请参考通达信--------------------------------
def COUNT(S, N):
    """**满足条件的天数**: 最近 N 天中为真的个数

    :param S: 条件序列
    :param N: 窗口
    :return: 计数序列
    """
    return SUM(S, N)


def EVERY(S, N):
    """**最近 N 天是否全为真**

    :param S: 条件序列
    :param N: 窗口
    :return: 布尔序列
    """
    return np.asarray(SUM(S, N) == N)


def EXIST(S, N):
    """**最近 N 天是否存在真值**

    :param S: 条件序列
    :param N: 窗口
    :return: 布尔序列
    """
    return np.asarray(SUM(S, N) > 0)


def FILTER(S, N):
    """**信号过滤**: S 为真后，其后 N 个周期置 0

    返回 float64 序列（0/1），不再写回原数组 —— 原数组可能是 bool，
    写入 0 会在 NumPy 2.x 抛 `TypeError`。

    :param S: 条件序列
    :param N: 过滤周期数
    :return: 过滤后的序列
    """
    data = num.as_series(S).copy()
    for index in range(len(data)):
        if data[index] != 0:
            end = min(index + 1 + int(N), len(data))
            data[index + 1:end] = 0.0
    return data


def BARSLAST(S):
    """**上一次条件成立到当前的周期数**: 成立当天为 0

    :param S: 条件序列
    :return: 周期数序列
    """
    data = np.asarray(S) != 0
    out = np.zeros(len(data), dtype=np.int64)
    previous = 0
    for index, value in enumerate(data):
        if value:
            previous = 0
        else:
            previous += 1
        out[index] = previous
    return out


def BARSLASTCOUNT(S):
    """**连续满足条件的周期数**

    :param S: 条件序列
    :return: 周期数序列
    """
    data = np.asarray(S) != 0
    out = np.zeros(len(data), dtype=np.int64)
    count = 0
    for index, value in enumerate(data):
        count = count + 1 if value else 0
        out[index] = count
    return out


def BARSSINCEN(S, N):
    """**N 周期内第一次条件成立到现在的周期数**

    :param S: 条件序列
    :param N: 窗口
    :return: 周期数序列，窗口无效处为 0
    """
    data = np.asarray(S) != 0
    out = np.zeros(len(data), dtype=np.int64)
    for index in range(len(data)):
        if index + 1 < int(N):
            continue
        block = data[index + 1 - int(N):index + 1]
        position = int(np.argmax(block))
        if position or block[0]:
            out[index] = int(N) - 1 - position
    return out


def CROSS(S1, S2):
    """**向上金叉**: 上一周期 S1 < S2 且本周期 S1 >= S2

    :param S1: 快线
    :param S2: 慢线
    :return: 布尔序列
    """
    previous = (REF(S1, 1) < REF(S2, 1)) & (np.asarray(S1) >= np.asarray(S2))
    return np.asarray(previous)


def V1_CROSS(S1, S2):
    """**向上金叉**: `CROSS` 的同义实现

    :param S1: 快线
    :param S2: 慢线
    :return: 布尔序列
    """
    left = num.as_series(S1)
    right = num.as_series(S2)
    out = np.zeros(len(left), dtype=np.bool_)
    if len(left) > 1:
        out[1:] = (left[:-1] < right[:-1]) & (left[1:] >= right[1:])
    return out


def LONGCROSS(S1, S2, N):
    """**维持 N 周期后上穿**: S1 之前 N 周期都小于 S2，本周期上穿

    :param S1: 快线
    :param S2: 慢线
    :param N: 维持周期
    :return: 布尔序列
    """
    return np.asarray(np.logical_and(LAST(np.asarray(S1) < np.asarray(S2), N, 1), np.asarray(S1) > np.asarray(S2)))


def VALUEWHEN(S, X):
    """**取值保持**: S 为真时取 X，否则沿用上一次的取值

    :param S: 条件序列
    :param X: 取值序列
    :return: 取值序列
    """
    values = np.where(np.asarray(S) != 0, num.as_series(X), np.nan)
    return pd.Series(values).ffill().to_numpy(dtype=np.float64)


def BETWEEN(S, A, B):
    """**区间判断**: S 处于 A 与 B 之间（含两种大小关系）

    :param S: 被判断序列
    :param A: 端点
    :param B: 端点
    :return: 布尔序列
    """
    data = num.as_series(S)
    return ((A < data) & (data < B)) | ((A > data) & (data > B))


def TOPRANGE(S):
    """**当前值距区间最高点的周期数**

    :param S: 序列
    :return: 周期数序列
    """
    data = num.as_series(S)
    out = np.zeros(len(data), dtype=np.int64)
    for index in range(1, len(data)):
        out[index] = int(np.argmin(np.flipud(data[:index] < data[index])))
    return out


def LOWRANGE(S):
    """**当前值距区间最低点的周期数**

    :param S: 序列
    :return: 周期数序列
    """
    data = num.as_series(S)
    out = np.zeros(len(data), dtype=np.int64)
    for index in range(1, len(data)):
        out[index] = int(np.argmin(np.flipud(data[:index] > data[index])))
    return out


# ------------------   2级: 技术指标函数(全部通过0级, 1级函数实现) ------------------------------
def MACD(CLOSE, SHORT=12, LONG=26, M=9):
    """**MACD**: 异同移动平均线

    :param CLOSE: 收盘价
    :param SHORT: 快线周期
    :param LONG: 慢线周期
    :param M: DEA 周期
    :return: `(DIF, DEA, MACD)`
    """
    DIF = EMA(CLOSE, SHORT) - EMA(CLOSE, LONG)
    DEA = EMA(DIF, M)
    MACD = (DIF - DEA) * 2
    return RD(DIF), RD(DEA), RD(MACD)


def KDJ(CLOSE, HIGH, LOW, N=9, M1=3, M2=3):
    """**KDJ**: 随机指标

    :param CLOSE: 收盘价
    :param HIGH: 最高价
    :param LOW: 最低价
    :param N: RSV 周期
    :param M1: K 平滑周期
    :param M2: D 平滑周期
    :return: `(K, D, J)`
    """
    RSV = (CLOSE - LLV(LOW, N)) / (HHV(HIGH, N) - LLV(LOW, N)) * 100
    K = EMA(RSV, (M1 * 2 - 1))
    D = EMA(K, (M2 * 2 - 1))
    J = K * 3 - D * 2
    return K, D, J


def RSI(CLOSE, N=24):
    """**RSI**: 相对强弱指标

    :param CLOSE: 收盘价
    :param N: 周期
    :return: RSI 序列
    """
    DIF = CLOSE - REF(CLOSE, 1)
    return RD(SMA(MAX(DIF, 0), N) / SMA(ABS(DIF), N) * 100)


def WR(CLOSE, HIGH, LOW, N=10, N1=6):
    """**W&R**: 威廉指标

    :param CLOSE: 收盘价
    :param HIGH: 最高价
    :param LOW: 最低价
    :param N: 周期
    :param N1: 短周期
    :return: `(WR, WR1)`
    """
    WR = (HHV(HIGH, N) - CLOSE) / (HHV(HIGH, N) - LLV(LOW, N)) * 100
    WR1 = (HHV(HIGH, N1) - CLOSE) / (HHV(HIGH, N1) - LLV(LOW, N1)) * 100
    return RD(WR), RD(WR1)


def BIAS(CLOSE, L1=6, L2=12, L3=24):
    """**BIAS**: 乖离率

    :param CLOSE: 收盘价
    :param L1: 短周期
    :param L2: 中周期
    :param L3: 长周期
    :return: `(BIAS1, BIAS2, BIAS3)`
    """
    BIAS1 = (CLOSE - MA(CLOSE, L1)) / MA(CLOSE, L1) * 100
    BIAS2 = (CLOSE - MA(CLOSE, L2)) / MA(CLOSE, L2) * 100
    BIAS3 = (CLOSE - MA(CLOSE, L3)) / MA(CLOSE, L3) * 100
    return RD(BIAS1), RD(BIAS2), RD(BIAS3)


def BOLL(CLOSE, N=20, P=2):
    """**BOLL**: 布林带

    :param CLOSE: 收盘价
    :param N: 周期
    :param P: 标准差倍数
    :return: `(UPPER, MID, LOWER)`
    """
    MID = MA(CLOSE, N)
    UPPER = MID + STD(CLOSE, N) * P
    LOWER = MID - STD(CLOSE, N) * P
    return RD(UPPER), RD(MID), RD(LOWER)


def PSY(CLOSE, N=12, M=6):
    """**PSY**: 心理线

    :param CLOSE: 收盘价
    :param N: 周期
    :param M: 均线周期
    :return: `(PSY, PSYMA)`
    """
    PSY = COUNT(CLOSE > REF(CLOSE, 1), N) / N * 100
    PSYMA = MA(PSY, M)
    return RD(PSY), RD(PSYMA)


def CCI(CLOSE, HIGH, LOW, N=14):
    """**CCI**: 顺势指标

    :param CLOSE: 收盘价
    :param HIGH: 最高价
    :param LOW: 最低价
    :param N: 周期
    :return: CCI 序列
    """
    TP = (HIGH + LOW + CLOSE) / 3
    return (TP - MA(TP, N)) / (0.015 * AVEDEV(TP, N))


def ATR(CLOSE, HIGH, LOW, N=20):
    """**ATR**: 真实波动 N 日平均

    :param CLOSE: 收盘价
    :param HIGH: 最高价
    :param LOW: 最低价
    :param N: 周期
    :return: ATR 序列
    """
    TR = MAX(MAX((HIGH - LOW), ABS(REF(CLOSE, 1) - HIGH)), ABS(REF(CLOSE, 1) - LOW))
    return MA(TR, N)


def BBI(CLOSE, M1=3, M2=6, M3=12, M4=20):
    """**BBI**: 多空指标

    :param CLOSE: 收盘价
    :param M1: 短周期
    :param M2: 中短周期
    :param M3: 中周期
    :param M4: 长周期
    :return: BBI 序列
    """
    return (MA(CLOSE, M1) + MA(CLOSE, M2) + MA(CLOSE, M3) + MA(CLOSE, M4)) / 4


def DMI(CLOSE, HIGH, LOW, M1=14, M2=6):
    """**DMI**: 动向指标

    :param CLOSE: 收盘价
    :param HIGH: 最高价
    :param LOW: 最低价
    :param M1: 主周期
    :param M2: 平均周期
    :return: `(PDI, MDI, ADX, ADXR)`
    """
    TR = SUM(MAX(MAX(HIGH - LOW, ABS(HIGH - REF(CLOSE, 1))), ABS(LOW - REF(CLOSE, 1))), M1)
    HD = HIGH - REF(HIGH, 1)
    LD = REF(LOW, 1) - LOW
    DMP = SUM(IF((HD > 0) & (HD > LD), HD, 0), M1)
    DMM = SUM(IF((LD > 0) & (LD > HD), LD, 0), M1)
    PDI = DMP * 100 / TR
    MDI = DMM * 100 / TR
    ADX = MA(ABS(MDI - PDI) / (PDI + MDI) * 100, M2)
    ADXR = (ADX + REF(ADX, M2)) / 2
    return PDI, MDI, ADX, ADXR


def TAQ(HIGH, LOW, N):
    """**唐安奇通道**: 海龟交易通道

    :param HIGH: 最高价
    :param LOW: 最低价
    :param N: 周期
    :return: `(UP, MID, DOWN)`
    """
    UP = HHV(HIGH, N)
    DOWN = LLV(LOW, N)
    MID = (UP + DOWN) / 2
    return UP, MID, DOWN


def KTN(CLOSE, HIGH, LOW, N=20, M=10):
    """**肯特纳通道**

    :param CLOSE: 收盘价
    :param HIGH: 最高价
    :param LOW: 最低价
    :param N: 中轨周期
    :param M: ATR 周期
    :return: `(UPPER, MID, LOWER)`
    """
    MID = EMA((HIGH + LOW + CLOSE) / 3, N)
    ATRN = ATR(CLOSE, HIGH, LOW, M)
    UPPER = MID + 2 * ATRN
    LOWER = MID - 2 * ATRN
    return UPPER, MID, LOWER


def TRIX(CLOSE, M1=12, M2=20):
    """**TRIX**: 三重指数平滑平均线

    :param CLOSE: 收盘价
    :param M1: 平滑周期
    :param M2: 均线周期
    :return: `(TRIX, TRMA)`
    """
    TR = EMA(EMA(EMA(CLOSE, M1), M1), M1)
    TRIX = (TR - REF(TR, 1)) / REF(TR, 1) * 100
    TRMA = MA(TRIX, M2)
    return TRIX, TRMA


def VR(CLOSE, VOL, M1=26):
    """**VR**: 容量比率

    :param CLOSE: 收盘价
    :param VOL: 成交量
    :param M1: 周期
    :return: VR 序列
    """
    LC = REF(CLOSE, 1)
    return SUM(IF(CLOSE > LC, VOL, 0), M1) / SUM(IF(CLOSE <= LC, VOL, 0), M1) * 100


def EMV(HIGH, LOW, VOL, N=14, M=9):
    """**EMV**: 简易波动指标

    :param HIGH: 最高价
    :param LOW: 最低价
    :param VOL: 成交量
    :param N: 周期
    :param M: 均线周期
    :return: `(EMV, MAEMV)`
    """
    VOLUME = MA(VOL, N) / VOL
    MID = 100 * (HIGH + LOW - REF(HIGH + LOW, 1)) / (HIGH + LOW)
    EMV = MA(MID * VOLUME * (HIGH - LOW) / MA(HIGH - LOW, N), N)
    MAEMV = MA(EMV, M)
    return EMV, MAEMV


def DPO(CLOSE, M1=20, M2=10, M3=6):
    """**DPO**: 区间震荡线

    :param CLOSE: 收盘价
    :param M1: 均线周期
    :param M2: 位移
    :param M3: 平滑周期
    :return: `(DPO, MADPO)`
    """
    DPO = CLOSE - REF(MA(CLOSE, M1), M2)
    MADPO = MA(DPO, M3)
    return DPO, MADPO


def BRAR(OPEN, CLOSE, HIGH, LOW, M1=26):
    """**BRAR**: 情绪指标

    :param OPEN: 开盘价
    :param CLOSE: 收盘价
    :param HIGH: 最高价
    :param LOW: 最低价
    :param M1: 周期
    :return: `(AR, BR)`
    """
    AR = SUM(HIGH - OPEN, M1) / SUM(OPEN - LOW, M1) * 100
    BR = SUM(MAX(0, HIGH - REF(CLOSE, 1)), M1) / SUM(MAX(0, REF(CLOSE, 1) - LOW), M1) * 100
    return AR, BR


def DFMA(CLOSE, N1=10, N2=50, M=10):
    """**DFMA**: 平行线差指标

    :param CLOSE: 收盘价
    :param N1: 短周期
    :param N2: 长周期
    :param M: 均线周期
    :return: `(DIF, DIFMA)`
    """
    DIF = MA(CLOSE, N1) - MA(CLOSE, N2)
    DIFMA = MA(DIF, M)
    return DIF, DIFMA


def MTM(CLOSE, N=12, M=6):
    """**MTM**: 动量指标

    :param CLOSE: 收盘价
    :param N: 周期
    :param M: 均线周期
    :return: `(MTM, MTMMA)`
    """
    MTM = CLOSE - REF(CLOSE, N)
    MTMMA = MA(MTM, M)
    return MTM, MTMMA


def MASS(HIGH, LOW, N1=9, N2=25, M=6):
    """**MASS**: 梅斯线

    :param HIGH: 最高价
    :param LOW: 最低价
    :param N1: 短周期
    :param N2: 长周期
    :param M: 均线周期
    :return: `(MASS, MA_MASS)`
    """
    MASS = SUM(MA(HIGH - LOW, N1) / MA(MA(HIGH - LOW, N1), N1), N2)
    MA_MASS = MA(MASS, M)
    return MASS, MA_MASS


def ROC(CLOSE, N=12, M=6):
    """**ROC**: 变动率指标

    :param CLOSE: 收盘价
    :param N: 周期
    :param M: 均线周期
    :return: `(ROC, MAROC)`
    """
    ROC = 100 * (CLOSE - REF(CLOSE, N)) / REF(CLOSE, N)
    MAROC = MA(ROC, M)
    return ROC, MAROC


def EXPMA(CLOSE, N1=12, N2=50):
    """**EXPMA**: 指数平均数指标

    :param CLOSE: 收盘价
    :param N1: 短周期
    :param N2: 长周期
    :return: `(EMA(N1), EMA(N2))`
    """
    return EMA(CLOSE, N1), EMA(CLOSE, N2)


def OBV(CLOSE, VOL):
    """**OBV**: 能量潮指标

    :param CLOSE: 收盘价
    :param VOL: 成交量
    :return: OBV 序列
    """
    return SUM(IF(CLOSE > REF(CLOSE, 1), VOL, IF(CLOSE < REF(CLOSE, 1), -VOL, 0)), 0) / 10000


def MFI(CLOSE, HIGH, LOW, VOL, N=14):
    """**MFI**: 成交量的 RSI

    :param CLOSE: 收盘价
    :param HIGH: 最高价
    :param LOW: 最低价
    :param VOL: 成交量
    :param N: 周期
    :return: MFI 序列
    """
    TYP = (HIGH + LOW + CLOSE) / 3
    V1 = SUM(IF(TYP > REF(TYP, 1), TYP * VOL, 0), N) / SUM(IF(TYP < REF(TYP, 1), TYP * VOL, 0), N)
    return 100 - (100 / (1 + V1))


def ASI(OPEN, CLOSE, HIGH, LOW, M1=26, M2=10):
    """**ASI**: 振动升降指标

    :param OPEN: 开盘价
    :param CLOSE: 收盘价
    :param HIGH: 最高价
    :param LOW: 最低价
    :param M1: 累计周期
    :param M2: 均线周期
    :return: `(ASI, ASIT)`
    """
    LC = REF(CLOSE, 1)
    AA = ABS(HIGH - LC)
    BB = ABS(LOW - LC)
    CC = ABS(HIGH - REF(LOW, 1))
    DD = ABS(LC - REF(OPEN, 1))
    R = IF((AA > BB) & (AA > CC), AA + BB / 2 + DD / 4, IF((BB > CC) & (BB > AA), BB + AA / 2 + DD / 4, CC + DD / 4))
    X = (CLOSE - LC + (CLOSE - OPEN) / 2 + LC - REF(OPEN, 1))
    SI = 16 * X / R * MAX(AA, BB)
    ASI = SUM(SI, M1)
    ASIT = MA(ASI, M2)
    return ASI, ASIT


def XSII(CLOSE, HIGH, LOW, N=102, M=7):
    """**XSII**: 薛斯通道 II

    :param CLOSE: 收盘价
    :param HIGH: 最高价
    :param LOW: 最低价
    :param N: 通道参数
    :param M: 偏移百分比
    :return: `(TD1, TD2, TD3, TD4)`
    """
    AA = MA((2 * CLOSE + HIGH + LOW) / 4, 5)
    TD1 = AA * N / 100
    TD2 = AA * (200 - N) / 100
    CC = ABS((2 * CLOSE + HIGH + LOW) / 4 - MA(CLOSE, 20)) / MA(CLOSE, 20)
    DD = DMA(CLOSE, CC)
    TD3 = (1 + M / 100) * DD
    TD4 = (1 - M / 100) * DD
    return TD1, TD2, TD3, TD4


def ZTPRICE(price, ratio):
    """**涨停价**: 前收盘价乘涨停比例，按分四舍五入

    支持标量、`pandas.Series` 与 numpy 数组；NaN 原样保留。

    :param price: 前收盘价
    :param ratio: 涨停比例，如 0.10
    :return: 与 `price` 同形态的涨停价
    """
    if ratio <= 0:
        return price
    factor = Decimal(str(1.0 + ratio))

    def convert(value):
        if value is None:
            return value
        try:
            if np.isnan(value):
                return value
        except TypeError:
            return value
        return float(
            (Decimal(str(value)) * factor).quantize(
                Decimal("0.01"), rounding=ROUND_HALF_UP
            )
        )

    if isinstance(price, pd.Series):
        return price.map(convert)
    if isinstance(price, np.ndarray):
        return np.array([convert(value) for value in price], dtype=float)
    return convert(price)
