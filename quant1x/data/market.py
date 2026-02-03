# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
from __future__ import annotations
from typing import List
from dataclasses import dataclass, field
from enum import Enum
#from typing import List, Optional

class Exchange(Enum):
    """交易所"""
    SSE = "sh" # 上交所
    SZSE = "sz" # 深交所
    BSE = "bj" # 北交所
    HKEX = "hk" # 港交所
    USA = "us" # 美国证券市场(泛指)
    UNKNOWN = "unknown" # 未知交易所

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

@dataclass
class Sector:
    """板块信息结构体"""
    name: str = ""
    code: str = ""
    type: int = 0
    count: int = 0
    block: str = ""
    constituent_stocks: List[str] = field(default_factory=List)

# 证券代码规则定义

@dataclass
class CodeRule:
    """证券代码规则"""
    exchange: Exchange # 交易所
    prefix: str        # 代码前缀
    type: InstrumentType # 证券类型
    name: str          # 证券类型名称
    desc: str          # 规则描述

# 全局规则
global_rules = [
    CodeRule(Exchange.SSE, "880", InstrumentType.Block, "板块指数", "通达信"),
    CodeRule(Exchange.SSE, "881", InstrumentType.Block, "板块指数", "通达信"),
]

# SSE 上海证券交易所规则
sse_rules = [
    CodeRule(Exchange.SSE, "000", InstrumentType.Index, "上证指数", "上证指数系列；000680-000689 用于科创板相关指数"),
    CodeRule(Exchange.SSE, "009", InstrumentType.Bond, "国债", "国债（2000年前发行）"),
    CodeRule(Exchange.SSE, "010", InstrumentType.Bond, "国债", "国债（2000-2009年发行）"),
    CodeRule(Exchange.SSE, "018", InstrumentType.Bond, "政策性银行债", "政策性银行金融债"),
    CodeRule(Exchange.SSE, "019", InstrumentType.Bond, "国债", "国债（2010年及以后发行）"),
    CodeRule(Exchange.SSE, "020", InstrumentType.Bond, "记账式贴现国债", "记账式贴现国债"),
    CodeRule(Exchange.SSE, "090", InstrumentType.Bond, "国债质押回购出入库", "国债质押式回购质押券出入库"),
    CodeRule(Exchange.SSE, "091", InstrumentType.Bond, "国债质押回购出入库", "对应019***"),
    CodeRule(Exchange.SSE, "099", InstrumentType.Bond, "国债质押回购出入库", "对应009***"),
    
    CodeRule(Exchange.SSE, "0", InstrumentType.Index, "指数/国债", "首位 0：指数、国债"),

    CodeRule(Exchange.SSE, "100", InstrumentType.Bond, "债券回售/可转债", "100000-100899 用于可转换公司债券（对应600***）；100900-100999 用于债券回售（不再增用部分）"),
    CodeRule(Exchange.SSE, "101", InstrumentType.Bond, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "102", InstrumentType.Bond, "企业债质押出入库", "对应127000-127999"),
    CodeRule(Exchange.SSE, "103", InstrumentType.Bond, "企业债质押出入库", "对应124000-124999"),
    CodeRule(Exchange.SSE, "104", InstrumentType.Bond, "公司/企业债质押出入库", "104000-104499 用于公司债质押（对应122000-122499）；104500-104999 用于企业债质押（对应122500-122999）"),
    CodeRule(Exchange.SSE, "105", InstrumentType.Bond, "债券质押出入库", "105000-105699 分离交易的可转债质押（对应126***）；105700-105799 债券ETF质押；105800-105899 可转债质押（对应110***、113***）；105900-105999 企业债质押（对应120***、129***）"),
    CodeRule(Exchange.SSE, "106", InstrumentType.Bond, "地方政府债质押出入库", "对应130***"),
    CodeRule(Exchange.SSE, "107", InstrumentType.Bond, "记账式贴现国债质押出入库", "对应020***"),
    CodeRule(Exchange.SSE, "108", InstrumentType.Bond, "政策性银行债质押出入库", "对应018***"),
    CodeRule(Exchange.SSE, "109", InstrumentType.Bond, "地方政府债", "地方政府债券"),

    CodeRule(Exchange.SSE, "110", InstrumentType.Bond, "可转换公司债", "110000-110799 上市公司公开发行可转债（对应600***）；110800-110999 非公开发行"),
    CodeRule(Exchange.SSE, "111", InstrumentType.Bond, "可转换公司债", "111000-111499 对应605***"),
    CodeRule(Exchange.SSE, "112", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "113", InstrumentType.Bond, "可转换公司债", "113000-113499 对应601***；113500-113999 对应603***"),
    CodeRule(Exchange.SSE, "114", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "115", InstrumentType.Bond, "公开公司债", "公开发行公司债券"),
    CodeRule(Exchange.SSE, "118", InstrumentType.Bond, "科创板可转债", "118000-118499 用于科创板上市公司公开发行可转债"),

    CodeRule(Exchange.SSE, "120", InstrumentType.Bond, "企业/公司债", "122000-122499 用于公司债券；122500-122999 用于企业债券（见122）"),
    CodeRule(Exchange.SSE, "121", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "122", InstrumentType.Bond, "公司债/企业债", "122000-122499 用于公司债券；122500-122999 用于企业债券"),
    CodeRule(Exchange.SSE, "123", InstrumentType.Bond, "公司/企业债/ABS", "123000-123499 用于企业/公司债；123500-123999 用于资产支持证券"),
    CodeRule(Exchange.SSE, "124", InstrumentType.Bond, "企业债质押出入库", "对应124000-124999"),
    CodeRule(Exchange.SSE, "125", InstrumentType.Bond, "中小企业私募债/非公开公司债", "中小企业私募债券、非公开发行公司债券"),
    CodeRule(Exchange.SSE, "126", InstrumentType.Bond, "分离交易可转债", "分离交易的可转换公司债券"),
    CodeRule(Exchange.SSE, "127", InstrumentType.Bond, "企业债", "127000-127899 用于企业债券；127900-127999 用于政府支持债（中国铁路建设债专用）"),
    CodeRule(Exchange.SSE, "128", InstrumentType.Bond, "信贷资产支持证券", "信贷资产支持证券"),
    CodeRule(Exchange.SSE, "129", InstrumentType.Bond, "企业债", "企业债券"),

    CodeRule(Exchange.SSE, "130", InstrumentType.Bond, "地方政府债", "地方政府债券(对应130***)"),
    CodeRule(Exchange.SSE, "131", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "132", InstrumentType.Bond, "可交换公司债", "可交换公司债券"),
    CodeRule(Exchange.SSE, "133", InstrumentType.Bond, "可交换债质押出入库", "对应132***"),
    CodeRule(Exchange.SSE, "134", InstrumentType.Bond, "公开公司债质押出入库", "对应136***"),
    CodeRule(Exchange.SSE, "135", InstrumentType.Bond, "证券公司短期债/并购私募债", "证券公司短期债、并购重组私募债券、非公开发行公司债券"),
    CodeRule(Exchange.SSE, "136", InstrumentType.Bond, "公开公司债质押出入库", "对应136***"),
    CodeRule(Exchange.SSE, "137", InstrumentType.Bond, "可交换/公开公司债", "137000-137499 非公开可交换；137500-137999 公开公司债"),
    CodeRule(Exchange.SSE, "138", InstrumentType.Bond, "可交换换股/公开公司债", "138000-138499 非公开可交换换股(对应137000-137499)；138500-138999 公开公司债"),
    CodeRule(Exchange.SSE, "139", InstrumentType.Bond, "企业债", "企业债券"),

    CodeRule(Exchange.SSE, "140", InstrumentType.Bond, "地方政府债质押出入库", "对应140***"),
    CodeRule(Exchange.SSE, "141", InstrumentType.Bond, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "142", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "143", InstrumentType.Bond, "公开公司债质押出入库", "对应143***"),
    CodeRule(Exchange.SSE, "144", InstrumentType.Bond, "公开公司债", "公开发行公司债券"),
    CodeRule(Exchange.SSE, "145", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "146", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "147", InstrumentType.Bond, "地方政府债质押出入库", "对应147***"),
    CodeRule(Exchange.SSE, "148", InstrumentType.Bond, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "149", InstrumentType.Bond, "资产支持证券", "资产支持证券"),

    CodeRule(Exchange.SSE, "150", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "151", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "152", InstrumentType.Bond, "企业债质押出入库", "对应152***"),
    CodeRule(Exchange.SSE, "153", InstrumentType.Bond, "企业债", "企业债券"),
    CodeRule(Exchange.SSE, "154", InstrumentType.Bond, "公司债质押出入库", "对应155***"),
    CodeRule(Exchange.SSE, "155", InstrumentType.Bond, "公司债质押出入库", "对应155***"),
    CodeRule(Exchange.SSE, "156", InstrumentType.Bond, "公司债", "公司债券"),
    CodeRule(Exchange.SSE, "157", InstrumentType.Bond, "地方政府债质押出入库", "对应157***"),
    CodeRule(Exchange.SSE, "158", InstrumentType.Bond, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "159", InstrumentType.Bond, "资产支持证券", "资产支持证券"),

    CodeRule(Exchange.SSE, "160", InstrumentType.Bond, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "161", InstrumentType.Bond, "地方政府债质押出入库", "对应160***"),
    CodeRule(Exchange.SSE, "162", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "163", InstrumentType.Bond, "公开公司债质押出入库", "对应163***"),
    CodeRule(Exchange.SSE, "164", InstrumentType.Bond, "公开公司债", "公开发行公司债券"),
    CodeRule(Exchange.SSE, "165", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "166", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "167", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "168", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "169", InstrumentType.Bond, "资产支持证券", "资产支持证券"),

    CodeRule(Exchange.SSE, "170", InstrumentType.Bond, "信用保护工具", "170000-170499 用于信用保护凭证；170900-170999 用于组合型信用保护合约"),
    CodeRule(Exchange.SSE, "171", InstrumentType.Bond, "地方政府债质押出入库", "对应171***"),
    CodeRule(Exchange.SSE, "172", InstrumentType.Bond, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "173", InstrumentType.Bond, "地方政府债质押出入库", "对应173***"),
    CodeRule(Exchange.SSE, "174", InstrumentType.Bond, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "175", InstrumentType.Bond, "公开公司债质押出入库", "对应175***"),
    CodeRule(Exchange.SSE, "176", InstrumentType.Bond, "公开公司债", "公开发行公司债券"),
    CodeRule(Exchange.SSE, "177", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "178", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "179", InstrumentType.Bond, "资产支持证券", "资产支持证券"),

    CodeRule(Exchange.SSE, "180", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "181", InstrumentType.Bond, "可转债转股/非公开公司债", "对应600*** 的转股等/182000 系列为回售或非公开"),
    CodeRule(Exchange.SSE, "182", InstrumentType.Bond, "债券回售/非公开公司债", "182000-182299 用于债券回售；182300-182999 用于非公开发行公司债券"),
    CodeRule(Exchange.SSE, "183", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "184", InstrumentType.Bond, "企业债/政府支持债", "184000-184799 企业债券；184800-184999 政府支持债（中国铁路建设债专用）"),
    CodeRule(Exchange.SSE, "185", InstrumentType.Bond, "公开公司债", "公开发行公司债券"),
    CodeRule(Exchange.SSE, "186", InstrumentType.Bond, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "187", InstrumentType.Bond, "公开公司债质押出入库", "对应188***"),
    CodeRule(Exchange.SSE, "188", InstrumentType.Bond, "公开公司债质押出入库", "对应188***"),
    CodeRule(Exchange.SSE, "189", InstrumentType.Bond, "资产支持证券", "资产支持证券"),

    CodeRule(Exchange.SSE, "190", InstrumentType.Bond, "可转债转股", "对应600***"),
    CodeRule(Exchange.SSE, "191", InstrumentType.Bond, "可转债转股", "191000-191499 对应601***；191500-191999 对应603***"),
    CodeRule(Exchange.SSE, "192", InstrumentType.Bond, "可交换债换股", "对应132***"),
    CodeRule(Exchange.SSE, "193", InstrumentType.Bond, "创新创业转股/ABS", "193000-193099 创新创业公司非公开可转债转股（对应145900-145999）；193100-193999 用于资产支持证券"),
    CodeRule(Exchange.SSE, "194", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "195", InstrumentType.Bond, "可转债转股", "195000-195499 用于可转债转股，对应605***"),
    CodeRule(Exchange.SSE, "196", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "197", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "198", InstrumentType.Bond, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "199", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    
    CodeRule(Exchange.SSE, "1", InstrumentType.Bond, "债券现券", "首位 1：债券现券"),

    CodeRule(Exchange.SSE, "201", InstrumentType.Bond, "国债回购", "国债回购（席位托管方式）"),
    CodeRule(Exchange.SSE, "202", InstrumentType.Bond, "企业债回购", "企业债回购（席位托管方式）"),
    CodeRule(Exchange.SSE, "203", InstrumentType.Bond, "国债买断式回购", "国债买断式回购"),
    CodeRule(Exchange.SSE, "204", InstrumentType.Bond, "债券质押式回购(账户托管)", "债券质押式回购（账户托管方式）"),
    CodeRule(Exchange.SSE, "205", InstrumentType.Bond, "质押式报价回购", "质押式报价回购"),
    CodeRule(Exchange.SSE, "206", InstrumentType.Bond, "质押式协议回购", "债券质押式协议回购"),
    CodeRule(Exchange.SSE, "207", InstrumentType.Bond, "质押式三方回购", "债券质押式三方回购"),
    CodeRule(Exchange.SSE, "208", InstrumentType.Bond, "债券借贷", "208000-208009 用于债券借贷业务"),

    CodeRule(Exchange.SSE, "230", InstrumentType.Bond, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "231", InstrumentType.Bond, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "232", InstrumentType.Bond, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "233", InstrumentType.Bond, "地方政府债", "地方政府债券"),

    CodeRule(Exchange.SSE, "240", InstrumentType.Bond, "公开公司债", "公开发行公司债券"),
    CodeRule(Exchange.SSE, "241", InstrumentType.Bond, "公开公司债", "公开发行公司债券"),

    CodeRule(Exchange.SSE, "250", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "251", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "252", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "253", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "254", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "255", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "256", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "257", InstrumentType.Bond, "非公开公司债", "非公开发行公司债券"),

    CodeRule(Exchange.SSE, "260", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "261", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "262", InstrumentType.Bond, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "263", InstrumentType.Bond, "资产支持证券", "资产支持证券"),

    CodeRule(Exchange.SSE, "270", InstrumentType.Bond, "企业债", "企业债券"),
    CodeRule(Exchange.SSE, "271", InstrumentType.Bond, "企业债", "企业债券"),
    CodeRule(Exchange.SSE, "272", InstrumentType.Bond, "企业债", "企业债券"),
    
    CodeRule(Exchange.SSE, "2", InstrumentType.Bond, "债券回购/借贷", "首位 2：债券回购、债券借贷等"),

    CodeRule(Exchange.SSE, "310", InstrumentType.Bond, "国债期货", "国债期货（已暂停）"),
    CodeRule(Exchange.SSE, "330", InstrumentType.IPO, "优先股(公开)", "公开发行优先股"),
    CodeRule(Exchange.SSE, "360", InstrumentType.Other, "非公开优先股", "非公开发行优先股"),
    
    CodeRule(Exchange.SSE, "3", InstrumentType.Other, "优先股/国债期货", "首位 3：优先股、国债期货（已暂停）"),
    
    CodeRule(Exchange.SSE, "4", InstrumentType.Other, "备用", "首位 4：备用"),
    
    CodeRule(Exchange.SSE, "500", InstrumentType.Fund, "封闭式基金", "契约型封闭式基金"),
    CodeRule(Exchange.SSE, "501", InstrumentType.Fund, "上市开放式基金", "上市开放式基金"),
    CodeRule(Exchange.SSE, "502", InstrumentType.Fund, "上市开放式基金", "上市开放式基金"),
    CodeRule(Exchange.SSE, "505", InstrumentType.Fund, "创新封闭式基金", "505800-505899 用于创新型封闭式证券投资基金"),
    CodeRule(Exchange.SSE, "506", InstrumentType.Fund, "科创板LOF", "506000-506099 用于科创板相关 LOF"),
    CodeRule(Exchange.SSE, "508", InstrumentType.Fund, "公募REITs", "508000-508099 用于公募 REITs"),
    CodeRule(Exchange.SSE, "510", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为沪市指数、跨市场指数或跨境指数"),
    CodeRule(Exchange.SSE, "511", InstrumentType.ETF, "债券ETF/货基", "511000-511299 单市场债券（沪）ETF；511300-511599 现金申赎类债券ETF；511600-511999 交易型货币基金"),
    CodeRule(Exchange.SSE, "512", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "513", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨境指数"),
    CodeRule(Exchange.SSE, "515", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "516", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "517", InstrumentType.ETF, "跨市场股票ETF", "517000-517999 用于跨市场股票（沪港深京）ETF"),
    CodeRule(Exchange.SSE, "518", InstrumentType.ETF, "商品交易型开放式证券投资基金", "商品类 ETF"),
    CodeRule(Exchange.SSE, "519", InstrumentType.Fund, "开放式基金申赎/认购", "519*** 系列用于开放式基金的申赎/认购/跨市场转托管/分红/转换等；5198** 用于实时申赎货币基金（实时申赎）"),
    CodeRule(Exchange.SSE, "520", InstrumentType.ETF, "跨境ETF", "520500-520999 用于跨境 ETF"),
    CodeRule(Exchange.SSE, "521", InstrumentType.Fund, "开放式基金认购", "对应519*** 系列的认购业务"),
    CodeRule(Exchange.SSE, "522", InstrumentType.Fund, "开放式基金跨市场转托管", "对应519*** 系列的跨市场转托管业务"),
    CodeRule(Exchange.SSE, "523", InstrumentType.Fund, "开放式基金分红", "对应519*** 系列的分红业务"),
    CodeRule(Exchange.SSE, "524", InstrumentType.Fund, "开放式基金基金转换", "对应519*** 系列的基金转换业务"),
    CodeRule(Exchange.SSE, "530", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为沪市指数"),
    CodeRule(Exchange.SSE, "550", InstrumentType.Fund, "基金", ""),
    CodeRule(Exchange.SSE, "560", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "561", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "562", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "563", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "580", InstrumentType.Warrant, "权证", "含股改权证、公司权证"),
    CodeRule(Exchange.SSE, "582", InstrumentType.Warrant, "权证行权", "用于权证行权/行权相关代码"),
    CodeRule(Exchange.SSE, "588", InstrumentType.ETF, "科创板ETF", "588000-588299 单市场（科创板）ETF；588300-588699 跨市场（含科创板）ETF；588700-588999 单市场（科创板）ETF"),
    
    CodeRule(Exchange.SSE, "5", InstrumentType.Fund, "基金/REITs/权证", "首位 5：基金、公募 REITs、权证"),
    
    CodeRule(Exchange.SSE, "600", InstrumentType.Stock, "主板A股", "主板 A 股"),
    CodeRule(Exchange.SSE, "601", InstrumentType.Stock, "主板A股", "主板 A 股"),
    CodeRule(Exchange.SSE, "603", InstrumentType.Stock, "主板A股", "主板 A 股"),
    CodeRule(Exchange.SSE, "605", InstrumentType.Stock, "主板A股", "主板 A 股（配套号段）"),
    CodeRule(Exchange.SSE, "688", InstrumentType.Stock, "科创板", "科创板股票"),
    CodeRule(Exchange.SSE, "689", InstrumentType.Stock, "科创板存托凭证", "科创板存托凭证"),
    
    CodeRule(Exchange.SSE, "6", InstrumentType.Stock, "A股/存托凭证", "首位 6：A 股、存托凭证"),
    
    CodeRule(Exchange.SSE, "700", InstrumentType.Other, "配股", "配股（对应600***）"),
    CodeRule(Exchange.SSE, "701", InstrumentType.Other, "转配股", "转配股"),
    CodeRule(Exchange.SSE, "702", InstrumentType.Other, "职工股配股", "对应600***"),
    CodeRule(Exchange.SSE, "703", InstrumentType.Other, "配售", "配售"),
    CodeRule(Exchange.SSE, "704", InstrumentType.Other, "可转债配债", "可转换公司债券持股配债（对应600***）"),
    CodeRule(Exchange.SSE, "706", InstrumentType.Other, "要约收购/现金选择权", "706000-706599 主板；706600-706999 科创板"),
    CodeRule(Exchange.SSE, "707", InstrumentType.Other, "网上按市值申购/增发", "对应605***"),
    CodeRule(Exchange.SSE, "708", InstrumentType.Other, "网上按市值申购配号", "对应605***"),
    CodeRule(Exchange.SSE, "713", InstrumentType.Other, "可转债申购", "对应605***"),
    CodeRule(Exchange.SSE, "714", InstrumentType.Other, "可转债申购配号", "对应605***"),
    CodeRule(Exchange.SSE, "715", InstrumentType.Other, "可转债持股配债", "对应605***"),
    CodeRule(Exchange.SSE, "718", InstrumentType.Other, "科创板可转债申购", "对应118000-118499"),
    CodeRule(Exchange.SSE, "726", InstrumentType.Other, "科创板可转债配债", "对应118000-118499"),
    CodeRule(Exchange.SSE, "730", InstrumentType.IPO, "新股申购", "新股申购/网上申购"),
    CodeRule(Exchange.SSE, "758", InstrumentType.Other, "可交换债配号", "758000-758099"),
    CodeRule(Exchange.SSE, "759", InstrumentType.Other, "可交换债申购", "759000-759099"),
    CodeRule(Exchange.SSE, "786", InstrumentType.Other, "科创板配售/存托配售", "786000-786899 科创板股票配售；786900-786999 科创板存托凭证配售"),
    CodeRule(Exchange.SSE, "799", InstrumentType.Other, "特殊业务代码", "指定交易/融资融券/网络投票/资金前端控制/身份认证等（见799xxx 具体编码）"),
    
    CodeRule(Exchange.SSE, "7", InstrumentType.Other, "非交易业务", "首位 7：非交易业务"),

    CodeRule(Exchange.SSE, "880", InstrumentType.Block, "板块指数", "通达信"),
    CodeRule(Exchange.SSE, "881", InstrumentType.Block, "板块指数", "通达信"),
    CodeRule(Exchange.SSE, "888", InstrumentType.Bond, "标准券", "888880 为新标准券，用于债券回购转换成标准券"),
    
    CodeRule(Exchange.SSE, "8", InstrumentType.Bond, "标准券/备用", "首位 8：标准券、备用"),

    CodeRule(Exchange.SSE, "900", InstrumentType.BStock, "B股", "B 股"),
    CodeRule(Exchange.SSE, "901", InstrumentType.BStock, "B转H", "901000-901099 用于 B 转 H"),
    CodeRule(Exchange.SSE, "938", InstrumentType.Other, "网络投票", "对应 B 股（不再增用）"),
    CodeRule(Exchange.SSE, "939", InstrumentType.Other, "密码服务", "939988 用于 B 股网络投票密码服务"),
    
    CodeRule(Exchange.SSE, "9", InstrumentType.BStock, "B股", "首位 9：B 股"),
]

# SZSE 深圳证券交易所规则
szse_rules = [
    CodeRule(Exchange.SZSE, "395", InstrumentType.Index, "成交量统计指数", ""),
    CodeRule(Exchange.SZSE, "399", InstrumentType.Index, "深证指数", ""),
    
    CodeRule(Exchange.SZSE, "000", InstrumentType.Stock, "主板A股", ""),
    CodeRule(Exchange.SZSE, "001", InstrumentType.Stock, "主板A股", ""),
    CodeRule(Exchange.SZSE, "002", InstrumentType.Stock, "主板A股", ""),
    CodeRule(Exchange.SZSE, "003", InstrumentType.Stock, "主板A股", ""),
    CodeRule(Exchange.SZSE, "030", InstrumentType.Warrant, "权证", ""),
    CodeRule(Exchange.SZSE, "031", InstrumentType.Warrant, "权证", ""),
    CodeRule(Exchange.SZSE, "032", InstrumentType.Warrant, "权证", ""),
    CodeRule(Exchange.SZSE, "036", InstrumentType.Warrant, "创业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0370", InstrumentType.Warrant, "主板A股股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0371", InstrumentType.Warrant, "主板A股股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0372", InstrumentType.Warrant, "创业板股权激励计划审计的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0373", InstrumentType.Warrant, "主板A股股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0374", InstrumentType.Warrant, "主板A股股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0375", InstrumentType.Warrant, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0376", InstrumentType.Warrant, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0377", InstrumentType.Warrant, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0378", InstrumentType.Warrant, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0379", InstrumentType.Warrant, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "038", InstrumentType.Warrant, "主板A股及中小企业股票认沽权证", ""),
    CodeRule(Exchange.SZSE, "039", InstrumentType.Warrant, "主板A股及中小企业股票认沽权证", ""),
    CodeRule(Exchange.SZSE, "070", InstrumentType.Warrant, "主板A股增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "071", InstrumentType.Warrant, "主板A股增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "072", InstrumentType.Warrant, "中小企业板增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "073", InstrumentType.Warrant, "中小企业板增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "074", InstrumentType.Warrant, "中小企业板增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "080", InstrumentType.Warrant, "A股配股", ""),
    
    CodeRule(Exchange.SZSE, "0", InstrumentType.Stock, "股票", ""),
    
    CodeRule(Exchange.SZSE, "10", InstrumentType.Bond, "国债", ""),
    CodeRule(Exchange.SZSE, "11", InstrumentType.Bond, "企业债", ""),
    CodeRule(Exchange.SZSE, "120", InstrumentType.Bond, "企业债券", ""),
    CodeRule(Exchange.SZSE, "123", InstrumentType.Bond, "可转债", ""),
    CodeRule(Exchange.SZSE, "127", InstrumentType.Bond, "可转债", ""),
    CodeRule(Exchange.SZSE, "128", InstrumentType.Bond, "可转债", ""),
    CodeRule(Exchange.SZSE, "13", InstrumentType.Bond, "债券回购", ""),
    CodeRule(Exchange.SZSE, "159", InstrumentType.ETF, "深交所ETF", ""),
    CodeRule(Exchange.SZSE, "15", InstrumentType.Fund, "ETF", ""),
    CodeRule(Exchange.SZSE, "16", InstrumentType.Fund, "LOF", ""),
    CodeRule(Exchange.SZSE, "17", InstrumentType.Fund, "传统投资基金", ""),
    CodeRule(Exchange.SZSE, "184", InstrumentType.Fund, "封闭式基金", ""),
    CodeRule(Exchange.SZSE, "18", InstrumentType.Fund, "封闭式基金", ""),
    
    CodeRule(Exchange.SZSE, "1", InstrumentType.Bond, "债券", ""),
    
    CodeRule(Exchange.SZSE, "200", InstrumentType.BStock, "B股", ""),
    CodeRule(Exchange.SZSE, "238", InstrumentType.Other, "B股现金选择权", ""),
    CodeRule(Exchange.SZSE, "28", InstrumentType.Other, "B股配股优先权", ""),
    
    CodeRule(Exchange.SZSE, "2", InstrumentType.BStock, "B股", ""),
    
    CodeRule(Exchange.SZSE, "300", InstrumentType.Stock, "创业板", ""),
    CodeRule(Exchange.SZSE, "301", InstrumentType.Stock, "创业板注册制", ""),
    CodeRule(Exchange.SZSE, "30", InstrumentType.Stock, "创业板", ""),
    CodeRule(Exchange.SZSE, "36", InstrumentType.Other, "投票", ""),
    CodeRule(Exchange.SZSE, "37", InstrumentType.Other, "增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "38", InstrumentType.Other, "配股/可转债优先权", ""),
    
    CodeRule(Exchange.SZSE, "50", InstrumentType.Bond, "资产支持证券ABS", ""),
    CodeRule(Exchange.SZSE, "56", InstrumentType.Bond, "资产支持证券ABS", ""),
    
    CodeRule(Exchange.SZSE, "5", InstrumentType.Bond, "资产支持证券ABS", ""),
    
    CodeRule(Exchange.SZSE, "700", InstrumentType.Warrant, "B股增发", ""),
    CodeRule(Exchange.SZSE, "730", InstrumentType.Warrant, "跨市场申购", ""),
]

# BSE 香港证券交易所规则
bse_rules = [
    CodeRule(Exchange.BSE, "899", InstrumentType.Index, "指数", "证券指数首三位代码为899"),
    
    CodeRule(Exchange.BSE, "920", InstrumentType.Stock, "北交所新上市", "2024-04-22 起新上市使用920号段；已上市公司继续沿用原代码直到统一切换"),
    CodeRule(Exchange.BSE, "92", InstrumentType.Stock, "上市公司普通股", "首两位92：上市公司普通股票；920号段自2024-04-22起用于新上市公司"),
    
    CodeRule(Exchange.BSE, "400", InstrumentType.Stock, "两网/退市A股", "两网公司及退市公司A股首三位代码为400"),
    CodeRule(Exchange.BSE, "420", InstrumentType.BStock, "退市B股", "退市公司B股首三位代码为420"),
    
    CodeRule(Exchange.BSE, "810", InstrumentType.Bond, "可转换公司债", "向特定对象发行的可转换公司债券首三位代码为810"),
    CodeRule(Exchange.BSE, "81", InstrumentType.Bond, "优先股(极少)", "其他极少数代码"),
    CodeRule(Exchange.BSE, "820", InstrumentType.Bond, "优先股", "优先股票首三位代码为820"),
    CodeRule(Exchange.BSE, "821", InstrumentType.Bond, "优先股", "优先股票首三位代码为820"),
    CodeRule(Exchange.BSE, "82", InstrumentType.Bond, "优先股(极少)", "其他极少数代码"),
    CodeRule(Exchange.BSE, "83", InstrumentType.Stock, "挂牌公司普通股", "挂牌公司普通股票首两位为83"),
    CodeRule(Exchange.BSE, "840", InstrumentType.Other, "要约收购", "要约收购证券代码首三位代码为84"),
    CodeRule(Exchange.BSE, "841", InstrumentType.Other, "要约回购", "要约回购证券代码首三位代码为841"),
    CodeRule(Exchange.BSE, "87", InstrumentType.Stock, "挂牌公司普通股", "挂牌公司普通股票首两位为87"),
    CodeRule(Exchange.BSE, "88", InstrumentType.Stock, "挂牌公司普通股", "挂牌公司普通股票首两位为88"),
    CodeRule(Exchange.BSE, "850", InstrumentType.Option, "股权激励期权", "股权激励期权首三位代码为850"),
]

# HKEX 香港交易所规则
hkex_rules = [
    CodeRule(Exchange.HKEX, "HSI", InstrumentType.Index, "恒生指数", ""),
    CodeRule(Exchange.HKEX, "HSCEI", InstrumentType.Index, "国企指数", ""),
    CodeRule(Exchange.HKEX, "HSCCI", InstrumentType.Index, "红筹指数", ""),
    CodeRule(Exchange.HKEX, "028", InstrumentType.ETF, "ETF", ""),
    CodeRule(Exchange.HKEX, "030", InstrumentType.ETF, "ETF", ""),
    CodeRule(Exchange.HKEX, "031", InstrumentType.ETF, "ETF", ""),
    CodeRule(Exchange.HKEX, "090", InstrumentType.ETF, "ETF", ""),
    CodeRule(Exchange.HKEX, "091", InstrumentType.ETF, "ETF", ""),
    CodeRule(Exchange.HKEX, "08", InstrumentType.Stock, "港股", "GEM"),
    CodeRule(Exchange.HKEX, "0", InstrumentType.Stock, "港股", ""),
    CodeRule(Exchange.HKEX, "1", InstrumentType.Bond, "权证", ""),
    CodeRule(Exchange.HKEX, "2", InstrumentType.Bond, "权证", ""),
    CodeRule(Exchange.HKEX, "4", InstrumentType.Bond, "牛熊证", ""),
    CodeRule(Exchange.HKEX, "5", InstrumentType.Bond, "牛熊证", ""),
    CodeRule(Exchange.HKEX, "6", InstrumentType.Bond, "牛熊证", ""),
]


def match_rule(code: str, rules: List[CodeRule]) -> CodeRule:
    """
    根据代码前缀匹配最优规则
    
    Args:
        code (str): 需要匹配的代码字符串
        rules (List[CodeRule]): 可匹配的规则列表
    
    Returns:
        CodeRule: 匹配到的最优规则，若无匹配则返回默认的未知规则
    """
    best_match: CodeRule | None = None
    best_len = 0

    for entry in rules:
        prefix = entry.prefix
        # 跳过空前缀（可选，根据业务）
        if not prefix:
            continue
        if code.startswith(prefix) and len(prefix) > best_len:
            best_len = len(prefix)
            best_match = entry

    if best_match is not None:
        return best_match
    else:
        return CodeRule(
            exchange=Exchange.UNKNOWN,
            prefix="",
            type=InstrumentType.Unknown,
            name="",
            desc="未匹配到规则"
        )

def detect_instrument_type_by_rule(exchange: Exchange, code: str) -> InstrumentType:
    """
    根据交易所和代码，使用对应规则检测证券类型
    
    Args:
        exchange (Exchange): 交易所枚举
        code (str): 证券代码字符串
    
    Returns:
        InstrumentType: 检测到的证券类型，若无匹配则返回 Unknown
    """
    rules = None
    match exchange:
        case Exchange.SSE:
            rules = sse_rules
        case Exchange.SZSE:
            rules = szse_rules
        case Exchange.BSE:
            rules = bse_rules
        case Exchange.HKEX:
            rules = hkex_rules
        case _:
            return InstrumentType.Unknown

    cr = match_rule(code, rules)
    return cr.type

ALL_EXCHANGE_CODES = {
    Exchange.SSE.value,
    Exchange.SZSE.value,
    Exchange.BSE.value,
    Exchange.HKEX.value,
    Exchange.USA.value,
}

def detect_symbol(input_str: str) -> Instrument:
    """
    检测并解析证券代码的市场类型及证券类型
    
    Args:
        input_str (str): 输入的证券代码字符串，支持多种格式：
            - 前缀形式：sh600000（上海交易所）
            - 后缀形式：600000.sh 或 APPL.us
            - 纯数字形式：600000（自动推断交易所）
            - 4字母全大写：AAPL（自动识别为美股）
    
    Returns:
        Instrument: 包含以下属性的对象：
            - exchange_id: 交易所标识
            - symbol: 纯证券代码
            - instrument_type: 证券类型
    
    Note:
        支持识别以下交易所：
            - 上海证券交易所(SSE)
            - 深圳证券交易所(SZSE) 
            - 北京证券交易所(BSE)
            - 香港交易所(HKEX)
            - 美国市场(USA)
        自动根据代码规则推断证券类型（股票、债券等）
    """
    s = (input_str or "").strip()
    if not s:
        return Instrument(Exchange.SSE, InstrumentType.Unknown, "", "", 0, 0)
    s = s.lower()
    pure_code = s

    symbol = ""
    exchange = Exchange.UNKNOWN
    typ = InstrumentType.Unknown

    # 1. 判断前缀: sh600000
    if len(pure_code) >= 2 and pure_code[:2] in ALL_EXCHANGE_CODES:
        symbol = pure_code[2:]
        prefix_code = pure_code[:2]
        exchange = Exchange(prefix_code)
        # 走指定市场规则
    # 2. 判断后缀: 600000.sh or APPL.us
    elif len(pure_code) >= 3 and pure_code[-3] == '.' and pure_code[-2:] in ALL_EXCHANGE_CODES:
        symbol = pure_code[:-3]
        suffix_code = pure_code[-2:]
        exchange = Exchange(suffix_code)
        # 走指定市场规则
    else:
        # 纯数字或者字母
        code_len = len(pure_code)
        match code_len:
            case 4:
                if pure_code.isalpha():
                    exchange = Exchange.USA
                    typ = InstrumentType.Stock
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
                else:
                    return Instrument(Exchange.UNKNOWN, InstrumentType.Unknown, "", "", 0, 0)
            case 5:
                if pure_code.isdigit():
                    exchange = Exchange.HKEX
                    typ = InstrumentType.Stock
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
                else:
                    return Instrument(Exchange.UNKNOWN, InstrumentType.Unknown, "", "", 0, 0)
            case 6:
                # 1. 全局规则优先匹配
                cr = match_rule(pure_code, global_rules)
                if cr.exchange != Exchange.UNKNOWN:
                    exchange = cr.exchange
                    typ = cr.type
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
                # 2. 按市场匹配规则
                # 2.1 0、159和3开头，优先匹配深交所
                if pure_code.startswith(('0', '159', '3')):
                    cr = match_rule(pure_code, szse_rules)
                    if cr.exchange != Exchange.UNKNOWN:
                        exchange = cr.exchange
                        typ = cr.type
                        return Instrument(exchange, typ, pure_code, "", 0, 0)
                # 2.2 6和5开头，优先匹配上交所
                if pure_code.startswith(('6', '5')):
                    cr = match_rule(pure_code, sse_rules)
                    if cr.exchange != Exchange.UNKNOWN:
                        exchange = cr.exchange
                        typ = cr.type
                        return Instrument(exchange, typ, pure_code, "", 0, 0)
                # 2.3 匹配上交所
                cr = match_rule(pure_code, sse_rules)
                if cr.exchange != Exchange.UNKNOWN:
                    exchange = cr.exchange
                    typ = cr.type
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
                # 2.4 匹配深交所
                cr = match_rule(pure_code, szse_rules)
                if cr.exchange != Exchange.UNKNOWN:
                    exchange = cr.exchange
                    typ = cr.type
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
                # 2.5 匹配北交所
                cr = match_rule(pure_code, bse_rules)
                if cr.exchange != Exchange.UNKNOWN:
                    exchange = cr.exchange
                    typ = cr.type
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
            case _:
                exchange = Exchange.UNKNOWN
                typ = InstrumentType.Unknown

    # 3. 如果exchange是UNKNOWN，则返回未知规则
    if exchange == Exchange.UNKNOWN:
        return Instrument(Exchange.UNKNOWN, InstrumentType.Unknown, "", "", 0, 0)

    if typ == InstrumentType.Unknown:
        rules = None
        if exchange == Exchange.SSE:
            rules = sse_rules
        elif exchange == Exchange.SZSE:
            rules = szse_rules
        elif exchange == Exchange.BSE:
            rules = bse_rules
        elif exchange == Exchange.HKEX:
            rules = hkex_rules
        elif exchange == Exchange.USA:
            return Instrument(exchange, InstrumentType.Stock, pure_code, "", 0, 0)
        else:
            return Instrument(Exchange.UNKNOWN, InstrumentType.Unknown, "", "", 0, 0)

        cr = match_rule(symbol, rules)
        if cr.type != InstrumentType.Unknown:
            return Instrument(cr.exchange, cr.type, symbol, "", 0, 0)
        else:
            return Instrument(Exchange.UNKNOWN, InstrumentType.Unknown, "", "", 0, 0)
    else:
        return Instrument(exchange, typ, symbol, "", 0, 0)

def correct_security_code(code: str) -> str:
    """
    纠正证券代码格式，补全前缀或后缀
    
    Args:
        code (str): 输入的证券代码字符串，支持多种格式：
            - 前缀形式：sh600000（上海交易所）
            - 后缀形式：600000.sh 或 APPL.us
            - 纯数字形式：600000（自动推断交易所）
            - 4字母全大写：AAPL（自动识别为美股）  
            - 6位数字：600000（自动推断交易所）
            - 4字母全大写：AAPL（自动识别为美股）
    
    Returns:
        str: 纠正后的证券代码字符串
    """
    instrument = detect_symbol(code)
    if instrument.is_valid():
        return str(instrument)
    else:
        return ""

PRE_MARKET_HOUR = 9
PRE_MARKET_MINUTE = 0
PRE_MARKET_SECOND = 0
cn_cron_expr_daily_init = f"0 {PRE_MARKET_HOUR} {PRE_MARKET_MINUTE} * * *"

__all__ = [
    "detect_instrument_type_by_rule",
    "detect_symbol",
    "Instrument",
    "Exchange",
    "InstrumentType",
    "cn_cron_expr_daily_init"
    ]
