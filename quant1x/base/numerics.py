# -*- coding: UTF-8 -*-

from decimal import Decimal, ROUND_HALF_UP

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
    if isinstance(num, float):
        num = str(num)
    x = Decimal(num).quantize((Decimal('0.' + '0' * digits)), rounding=ROUND_HALF_UP)
    return float(x)


def fix_float(f: float) -> float:
    """
    修复f, 处理存在Nan和±Inf的情况
    :param f:
    :return:
    """
    return f if not is_nan(f) else 0
