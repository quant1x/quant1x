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
    """日期: YYYY-MM-DD, 用于查询和除权"""
    open: float = 0.0
    """开盘价"""
    close: float = 0.0
    """收盘价"""
    high: float = 0.0
    """最高价"""
    low: float = 0.0
    """最低价"""
    volume: float = 0.0
    """成交量"""
    amount: float = 0.0
    """成交额"""
    up: int = 0
    """上涨家数: 仅指数有效"""
    down: int = 0
    """下跌家数: 仅指数有效"""
    timestamp: str = ""
    """时间戳: YYYY-MM-DD HH:MM:SS, 为该条数据的收盘时间"""
    adjustment_count: int = 0
    """复权次数: 0表示未复权, 大于0表示已复权的次数, 该字段用来校验复权"""

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

    # 计算属性
    @property
    def change(self) -> float:
        """涨跌额"""
        return self.close - self.open
    
    @property
    def change_pct(self) -> float:
        """涨跌幅(百分比)"""
        if self.open == 0:
            return 0.0
        return (self.close - self.open) / self.open * 100
    
    @property
    def is_positive(self) -> bool:
        """是否阳线"""
        return self.close > self.open
    
    @property
    def is_negative(self) -> bool:
        """是否阴线"""
        return self.close < self.open
    
    # 可选: 技术指标相关
    @property
    def body_size(self) -> float:
        """K线实体大小"""
        return abs(self.close - self.open)
    
    @property
    def upper_shadow(self) -> float:
        """上影线长度"""
        return self.high - max(self.open, self.close)
    
    @property
    def lower_shadow(self) -> float:
        """下影线长度"""
        return min(self.open, self.close) - self.low
    
    # 可选: 量价关系
    @property
    def avg_price(self) -> float:
        """均价(成交额/成交量)"""
        if self.volume == 0:
            return 0.0
        return self.amount / self.volume
    
    @property
    def price_range(self) -> float:
        """价格区间(最高-最低)"""
        return self.high - self.low
    
    @classmethod
    def headers(cls) -> List[str]:
        """K线数据CSV头部"""
        return ["date", "open", "close", "high", "low", "volume", "amount", 
                "up", "down", "timestamp", "adjustment_count"]

    def to_list(self) -> List:
        """转为列表, 便于写入CSV"""
        return [
            self.date, self.open, self.close, self.high, self.low,
            self.volume, self.amount, self.up, self.down,
            self.timestamp, self.adjustment_count
        ]
    
    @classmethod
    def from_list(cls, data: List) -> "Bar":
        """从列表创建Bar实例"""
        return cls(
            date=str(data[0]) if len(data) > 0 else "",
            open=float(data[1]) if len(data) > 1 else 0.0,
            close=float(data[2]) if len(data) > 2 else 0.0,
            high=float(data[3]) if len(data) > 3 else 0.0,
            low=float(data[4]) if len(data) > 4 else 0.0,
            volume=float(data[5]) if len(data) > 5 else 0.0,
            amount=float(data[6]) if len(data) > 6 else 0.0,
            up=int(data[7]) if len(data) > 7 else 0,
            down=int(data[8]) if len(data) > 8 else 0,
            timestamp=str(data[9]) if len(data) > 9 else "",
            adjustment_count=int(data[10]) if len(data) > 10 else 0
        )

    def to_dict(self) -> dict:
        """转为扁平字典, 适配 DataFrame"""
        return {
            "date": self.date,
            "open": self.open,
            "close": self.close,
            "high": self.high,
            "low": self.low,
            "volume": self.volume,
            "amount": self.amount,
            "up": self.up,
            "down": self.down,
            "timestamp": self.timestamp,
            "adjustment_count": self.adjustment_count,
        }
    
    @classmethod
    def create_daily(cls, date: str, market: str = "cn", **kwargs) -> "Bar":
        """创建日线数据
        
        Args:
            date: 日期, YYYY-MM-DD
            market: 市场代码
                    "cn": 中国 (15:00)
                    "us": 美国 (16:00 ET)
                    "hk": 香港 (16:00 HKT)
                    "jp": 日本 (15:00 JST)
        """
        market_close_times = {
            "cn": 15,  # 中国股市
            "us": 16,  # 美股
            "hk": 16,  # 港股
            "jp": 15,  # 日股
            "kr": 15,  # 韩股
        }
        
        if "timestamp" not in kwargs:
            close_hour = market_close_times.get(market, 15)
            kwargs["timestamp"] = f"{date} {close_hour:02d}:00:00"
        
        return cls(date=date, **kwargs)

