# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from enum import Enum
from dataclasses import dataclass
from typing import List

class Direction(Enum):
    """交易方向"""
    BUY = 0 # 主动买入
    SELL = 1 # 主动卖出
    NEUTRAL = 2 # 中性盘

@dataclass
class Transaction:
    """交易数据结构体"""
    time: str = "" # 时间
    price: float = 0.0 # 价格
    volume: int = 0 # 成交量
    num: int = 0 # 成交笔数
    amount: float = 0.0 # 成交额
    direction: int = 2 # 交易方向

    @staticmethod
    def headers() -> List[str]:
        """逐笔交易数据头部"""
        return ["time", "price", "volume", "num", "amount", "direction"]    

