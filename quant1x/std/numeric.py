# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import math
import numpy as np


def is_nan(n) -> bool:
    """
    判断是否nan或inf
    :param n:
    :return:
    """
    return np.isnan(n) or np.isinf(n)


def float_round(num: float, digits: int = 2) -> float:
    """
    浮点四舍五入
    :param num:
    :param digits: 小数点后几位数字, 默认两位
    :return:
    """
    # retain previous behaviour using Decimal for exact rounding
    from decimal import Decimal, ROUND_HALF_UP
    s = str(num)
    x = Decimal(s).quantize((Decimal('0.' + '0' * digits)), rounding=ROUND_HALF_UP)
    return float(x)


def fix_float(f: float) -> float:
    """
    修复f, 处理存在Nan和±Inf的情况
    :param f:
    :return:
    """
    return f if not is_nan(f) else 0


def change_rate(base: float, value: float) -> float:
    """
    计算变化率
    :param base: 基准值
    :param value: 变化值
    :return: 变化率
    """
    if base == 0:
        return 0.0
    return value / base


def decimal(f: float, digits: int = 2) -> float:
    """
    保留小数位，使用与 C++ 等效的无分支算法
    :param f: 浮点数
    :param digits: 小数位数
    :return:
    """
    # Implement same branchless algorithm as C++ numeric::decimal
    # clamp digits to [0,9]
    if math.isnan(f):
        return 0.0
    if digits < 0:
        digits = 0
    if digits > 9:
        digits = 9

    kPowersOf10 = [10.0 ** i for i in range(0, 11)]

    half = math.copysign(5.0, f)
    nj1 = kPowersOf10[digits + 1]
    scaled = f * nj1 + half
    truncated = math.trunc(scaled / 10.0)
    return truncated / (nj1 / 10.0)
