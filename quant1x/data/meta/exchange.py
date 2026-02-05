# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from enum import Enum

class Exchange(Enum):
    """交易所"""
    SSE = "sh" # 上交所
    SZSE = "sz" # 深交所
    BSE = "bj" # 北交所
    HKEX = "hk" # 港交所
    USA = "us" # 美国证券市场(泛指)
    UNKNOWN = "unknown" # 未知交易所