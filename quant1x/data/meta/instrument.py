# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from enum import Enum
from dataclasses import dataclass
from typing import List
from .exchange import Exchange

class InstrumentType(Enum):
    """证券类型"""
    Unknown   = 0 # 未知类型
    Stock     = 1 # 股票
    ETF       = 2 # ETF
    Fund      = 3 # 基金
    Bond      = 4 # 债券
    BStock    = 5 # B股
    IPO       = 6 # IPO
    Index     = 7 # 指数
    Option    = 9 # 期权
    Future    = 10 # 期货
    Warrant   = 11 # 权证
    Forex     = 12 # 外汇
    Commodity = 13 # 商品
    Block     = 14 # 板块
    Other     = 255 # 其他类型
    
    def __str__(self) -> str:
        return self.name.lower()
    
    @classmethod
    def from_string(cls, s: str) -> "InstrumentType":
        key = s.strip().lower()
        # 将映射缓存到类属性（每个类独立）
        cache_attr = "_from_string_cache"
        if not hasattr(cls, cache_attr):
            setattr(cls, cache_attr, {
                name.lower(): member
                for name, member in cls.__members__.items()
            })
        cache = getattr(cls, cache_attr)
        return cache.get(key, cls.Unknown)
    
    def is_index(self) -> bool:
        """
        判断当前工具类型是否为指数或板块类型
        
        Returns:
            bool: 如果是指数或板块类型则返回True，否则返回False
        """
        return self in (InstrumentType.Index, InstrumentType.Block)

# 构建反向映射字典：小写名称 → 枚举成员
_INSTRUMENT_TYPE_BY_LOWER_NAME = {
    member.name.lower(): member
    for member in InstrumentType
}

@dataclass
class Instrument:
    """证券信息结构体"""
    exchange: Exchange       # 交易所代码（如 SH, SZ, NASDAQ）
    type: InstrumentType     # 证券类型（股票、债券、期货等）
    ticker: str              # 交易所分配的证券代码（ticker）
    name: str                # 证券名称
    lot_size: int = 100      # 每手股数
    price_precision: int = 2 # 价格小数位数
    
    def __str__(self) -> str:
        return f"{self.exchange.value}{self.ticker}"
    
    def symbol(self) -> str:
        return f"{self.exchange.value}{self.ticker}"
    
    def is_valid(self) -> bool:
        return self.exchange != Exchange.UNKNOWN and self.type != InstrumentType.Unknown
    
    def headers(self) -> List[str]:
        return ['exchange', 'type', 'code', 'name', 'lot_size', 'price_precision']

