# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from enum import Enum, IntEnum
from dataclasses import dataclass
from typing import List, Iterable, Any

from sqlalchemy import desc

from .exchange import Exchange
from .region import Region

    
class Subtype(IntEnum):
    """资产子类型(高4位), 语义由主类型(InstrumentType)决定"""
    DEFAULT = 0x00  # 默认/无特殊子类(如A股、普通指数)
    """默认市场"""
    CHINEXT = 0x10  # 深交所, 创业板, ChiNext
    STAR    = 0x20  # 上交所, 科创板, STAR(The Science and Technology Innovation Board)
    B       = 0x30  # B股(STOCK)/ 认购(OPTION预留)
    """B股市场"""
    H       = 0x40  # H股(STOCK)/ 认沽(OPTION预留)
    """H股市场"""
    GEM     = 0x50  # 港交所创业板, 成长型企业市场(Growth_Enterprises_Market)
    """港交所创业板市场"""
    EXCHANGE_TRADED = 0x60  # 交易型开放式
    """交易型开放式"""
    LISTED     = 0x70  # 上市型开放式
    """上市型开放式"""
    OPEN_ENDED = 0x80  # 开放式
    """开放式"""
    
    """上市型开放式指数"""
    MUTUAL  = 0xB0  # 公募市场
    """公募市场"""
    PRIVATE = 0xC0  # 私募市场
    """私募市场"""
    MONEY   = 0xD0  # 货币(FOREX)
    """货币市场"""
    SPECIAL = 0xE0  # 特殊变体：IPO(STOCK)、板块(INDEX)等
    """特殊变体"""
    TEMPORARY = 0xF0  # 临时市场：临时合约(FUTURE)等

class InstrumentType(IntEnum):
    """合约类型(低4位=资产大类, 高4位=子类型扩展)"""
    Unknown   = 0x00  # 未知类型
    INDEX     = 0x01  # 指数(含普通指数、板块等)
    """指数"""
    STOCK     = 0x02  # 股票(默认A股)
    """股票"""
    FUND      = 0x03  # 基金
    """基金"""
    BOND      = 0x04  # 债券
    """债券"""
    FOREX     = 0x05  # 外汇
    """外汇"""
    COMMODITY = 0x06  # 商品现货
    """商品现货"""
    FUTURE    = 0x07  # 期货
    """期货"""
    OPTION    = 0x08  # 期权
    """期权"""
    WARRANT   = 0x09  # 权证
    """权证"""
    # 0x0B-0x0E 预留基础类型扩展
    MACRO     = 0x0F  # 宏观指标
    """宏观指标"""
    
    # === 组合类型(命名空间化, 便于使用)===
    # 股票子类
    BSTOCK = Subtype.B.value | STOCK
    """B股"""
    HSTOCK = Subtype.H.value | STOCK
    """H股"""
    IPO    = Subtype.SPECIAL.value | STOCK
    """IPO"""
    
    CHINEXT_MARKET = Subtype.CHINEXT.value | STOCK
    """深交所, 创业板"""
    STAR_MARKET    = Subtype.STAR.value | STOCK
    """上交所, 科创板"""
    GEM_MARKET     = Subtype.GEM.value | STOCK
    """港交所, 创业板"""
    TEMPORARY_STOCK = Subtype.TEMPORARY.value | STOCK
    """港交所, 临时柜台"""
    
    
    # 基金子类
    ETF             = Subtype.EXCHANGE_TRADED.value | FUND
    """ETF基金"""
    LOF             = Subtype.LISTED.value | FUND # 上市型开放式基金, 是中国特色的交易所交易基金品种
    """LOF基金"""
    OPEN_ENDED_FUND = Subtype.OPEN_ENDED.value | FUND
    """开放式基金"""
    MONEY_FUND      = Subtype.MONEY.value | FUND
    """货币基金"""
    MACRO_INDICATOR = MACRO
    """宏观指标"""
    
    # 指数子类(板块作为指数的特殊变体)
    SECTOR = Subtype.SPECIAL.value | INDEX 
    """板块"""
    
    NEEQ = 0xFE
    """新三板/股转系统"""
    
    OTHER  = 0xFF
    """其他未分类"""

    # === 辅助方法 ===
    def base_type(self) -> 'InstrumentType':
        """提取基础资产类型(低4位)"""
        return InstrumentType(self.value & 0x0F)
    
    def subtype(self) -> int:
        """提取子类型扩展位(高4位)"""
        return self.value & 0xF0
    
    def is_stock(self) -> bool:
        return self.base_type() == InstrumentType.STOCK
    
    def is_index(self) -> bool:
        """判断是否为指数类(含普通指数、板块等)"""
        return self.base_type() == InstrumentType.INDEX  # ✅ 仅需判断低位
    
    def __str__(self) -> str:
        return self.name.lower()
    
    @classmethod
    def from_string(cls, s: str) -> "InstrumentType":
        key = s.strip().lower()
        cache_attr = "_from_string_cache"
        if not hasattr(cls, cache_attr):
            setattr(cls, cache_attr, {
                name.lower(): member
                for name, member in cls.__members__.items()
            })
        cache = getattr(cls, cache_attr)
        return cache.get(key, cls.Unknown)

# 构建反向映射字典: 小写名称 -> 枚举成员
_INSTRUMENT_TYPE_BY_LOWER_NAME = {
    member.name.lower(): member
    for member in InstrumentType
}

@dataclass
class Instrument:
    """证券信息结构体"""
    exchange: Exchange       # 交易所代码(如 SH, SZ, NASDAQ)
    type: InstrumentType     # 证券类型(股票, 债券, 期货等)
    ticker: str              # 交易所分配的证券代码(ticker)
    name: str                # 证券名称
    lot_size: int = 100      # 每手股数
    price_precision: int = 2 # 价格小数位数
    ext_market: int = 0      # 扩展市场代码(如 US, HK)
    ext_category: int = 0    # 扩展类别代码(如 STK, FUT, OPT, ...)
    desc: str = ""           # 证券描述 
    
    def __str__(self) -> str:
        """
        返回对象的字符串表示形式, 即调用symbol()方法的结果
        
        Returns:
            str: 对象的符号表示字符串
        """
        return self.symbol()
    
    def symbol(self) -> str:
        # 构建交易符号字符串
        # normalize
        if self.exchange.region == Region.CN:
            return f"{self.exchange.identifier}{self.ticker}"
        return f"{self.ticker}.{self.exchange.identifier}"
    
    def cache_dir(self) -> str:
        """
        获取缓存目录路径，用于存储交易所相关数据文件
        
        Note:
            返回的路径包含交易所标识符作为目录名的一部分，以便区分不同交易所的数据
        
        Returns:
            str: 缓存目录路径
        """
        return f'{self.exchange.name.lower()}'
    
    def to_string(self) -> str:
        return f"Instrument(exchange={self.exchange}, type={self.type}, ticker={self.ticker}, name={self.name}, lot_size={self.lot_size}, price_precision={self.price_precision}, ext_market={self.ext_market}, ext_category={self.ext_category})"
    
    @classmethod
    def headers(cls) -> List[str]:
        return ['exchange', 'type', 'code', 'name', 'lot_size', 'price_precision', 'ext_market', 'ext_category']
    
    def to_dict(self) -> dict:
        """
        将证券对象转换为字典格式
        
        Returns:
            dict: 包含证券基本信息的字典, 键包括:
                - exchange: 交易所枚举值
                - type: 证券类型枚举值  
                - code: 证券代码
                - name: 证券名称
                - lot_size: 每手股数
                - price_precision: 价格精度
                - ext_market: 扩展市场代码
                - ext_category: 扩展类别代码
        """
        return {
                'exchange': self.exchange.identifier,
                'type': self.type,
                'code': self.ticker,
                'name': self.name,
                'lot_size': self.lot_size,
                'price_precision': self.price_precision,
                'ext_market': self.ext_market,
                'ext_category': self.ext_category
            }
    
    
    def to_iterable(self) -> Iterable[Any]:
        """
        将证券对象转换为可迭代对象
        
        Returns:

            Iterable: 包含证券基本信息的可迭代对象, 顺序为:
                - exchange: 交易所枚举值
                - type: 证券类型枚举值  
                - code: 证券代码
                - name: 证券名称
                - lot_size: 每手股数
                - price_precision: 价格精度
                - ext_market: 扩展市场代码
                - ext_category: 扩展类别代码
        """

        return [
            self.exchange.identifier,
            self.type,
            self.ticker,
            self.name,
            self.lot_size,
            self.price_precision,
            self.ext_market,
            self.ext_category
        ]
    
    def __repr__(self) -> str:
        return self.to_string()
    
    def can_construct_symbol(self) -> bool:
        """
        检查当前对象是否可以构造有效的交易符号。
        
        Args:
            无显式参数, 但依赖于对象属性:
                exchange (Exchange): 交易所类型
                type (InstrumentType): 品种类型
        
        Returns:
            bool: 如果交易所类型和品种类型都已知则返回True, 否则返回False
        """
        return self.exchange != Exchange.UNKNOWN and self.type != InstrumentType.Unknown
    
    def is_valid(self) -> bool:
        """
        检查当前证券对象是否有效.

        Returns:
            bool: 如果证券的交易所, 类型, 最小交易单位和价格精度都有效则返回True, 否则返回False
        """
        return self.exchange != Exchange.UNKNOWN and self.type != InstrumentType.Unknown and self.lot_size > 0 and self.price_precision > 0
    
    

