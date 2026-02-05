# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from dataclasses import dataclass
from typing import List
from .adjustment import CumulativeAdjustment

@dataclass
class Bar:
    """K线数据结构体"""
    date: str = ""
    open: float = 0.0
    close: float = 0.0
    high: float = 0.0
    low: float = 0.0
    volume: float = 0.0
    amount: float = 0.0
    up: int = 0
    down: int = 0
    datetime: str = ""
    adjustment_count: int = 0

    def adjust(self, adj: CumulativeAdjustment):
        """复权"""
        self.open = self.open * adj.m + adj.a
        self.close = self.close * adj.m + adj.a
        self.high = self.high * adj.m + adj.a
        self.low = self.low * adj.m + adj.a
        
        # 成交量复权
        if self.volume != 0:
            # 1. 计算均价
            ap = self.amount / self.volume
            # 2. 均价复权
            ap_adjusted = ap * adj.m + adj.a
            # 3. 成交量复权
            self.volume *= (1 + adj.share_adjustment_ratio)
            # 4. 以新成交量*均价计算成交额
            self.amount = self.volume * ap_adjusted
        
        # 5. 更新除权除息次数
        self.adjustment_count = adj.no

    @staticmethod
    def headers() -> List[str]:
        """K线数据头部"""
        return ["date", "open", "close", "high", "low", "volume", "amount", "up", "down", "datetime", "adjustment_count"]
