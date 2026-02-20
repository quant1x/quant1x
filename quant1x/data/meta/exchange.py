# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

# exchange.py
from enum import Enum

from .region import Region

class Exchange(Enum):
    """交易所"""
    # 格式: (mic, identifier, region_enum, label)
    SSE      = ("XSHG",    "SH",        Region.CN,       "上海证券交易所")
    """上海证券交易所"""
    XSSC     = ("XSSC",    "SH",        Region.CN,       "上海证券交易所")
    """XSSC: 上海证券交易所 - 沪股通"""
    SZSE     = ("XSHE",    "SZ",        Region.CN,       "深圳证券交易所")
    """深圳证券交易所"""
    XSEC     = ("XSEC",    "SZ",        Region.CN,       "深圳证券交易所")
    """XSEC: 深证证券交易所 - 深股通"""
    BSE      = ("BJSE",    "BJ",        Region.CN,       "北京证券交易所")
    """北京证券交易所"""
    
    SHFE     = ("XSGE",    "SHFE",      Region.CN,       "上海期货交易所")
    """上海期货交易所, 主要品种: 金属(铜、黄金)、能源(原油)、化工(橡胶)、钢材(螺纹钢)等; 国际化品种: 上海国际能源交易中心INE, 隶属SHFE, 负责原油等国际化品种交易"""
    XINE     = ("XINE",    "INE",       Region.CN,       "上海国际能源交易中心")
    """上海国际能源交易中心, 主要品种: 国际化品种(原油、天然气、铜、铝、锌、黄金、白银、石油、天然气)"""
    CZCE      = ("XZCE",    "ZCE",      Region.CN,       "郑州商品交易所")
    """郑州商品交易所, 主要品种: 农产品(棉花、苹果)、化工(PTA)、期权等"""
    DCE      = ("XDCE",    "DCE",       Region.CN,       "大连商品交易所")
    """大连商品交易所, 主要品种: 农产品(大豆、玉米)、黑色系(铁矿石)、化工(塑料)等"""
    CFFEX    = ("CCFX",    "CFF",       Region.CN,       "中国金融期货交易所")
    """中国金融期货交易所, 主要品种: 股指期货(沪深300指数IF)、国债期货等"""
    GFEX     = ("GFEX",    "GFEX",      Region.CN,       "广州期货交易所")
    """广州期货交易所, 主要品种: 绿色金融(碳排放权、工业硅)"""
    SGE      = ("SGEX",    "SGE",       Region.CN,       "上海黄金交易所")
    """上海黄金交易所, 主要品种: 黄金T+D、白银T+D等"""
    
    HKEX     = ("XHKG",    "HK",        Region.HK,       "香港交易所(现货股票)")
    """香港交易所"""
    HKSC     = ("XHKG",    "HKSC",      Region.HK,       "香港交易所-港股通")
    """香港交易所-港股通, 虚拟MIC"""
    HKFE     = ("XHKF",    "HKF",       Region.HK,       "香港期货交易所(香港指数市场, 指数期货, 商品期货)")
    """香港期货交易所"""
    
    CSI     = ("CSI",    "CSI",         Region.CN,       "中证指数, China Securities Index, 中证指数有限公司")
    """中证指数, 虚拟MIC"""
    CNI     = ("CNI",    "CNI",         Region.CN,       "国证指数, CNI Index, 深证证券交易所指数机构")
    """CNI指数, 虚拟MIC"""
    
    EXTENDED = ("EXTENDED", "EXT",      Region.GLB,    "扩展市场, Extended")
    """扩展市场, 虚拟MIC"""
    
    OFFSHORE = ("OFFSHORE", "OS",       Region.OFFSHORE,  "国际, 其它离岸市场") # Offshore Indexes, 离岸指数
    """(其它)离岸市场, 虚拟MIC"""
    ONSHORE = ("ONSHORE",   "ON",       Region.ONSHORE,   "国内, 其它在岸市场") # Onshore Indexes, 在岸指数
    """(其它)在岸市场, 虚拟MIC"""
    OTC      = ("OTC",      "OTC",      Region.ONSHORE,   "国内, 场外") # Over-the-Counter, 场外交易
    """国内(场内)市场, 虚拟MIC"""
    OFFEX = ("OFFEX",       "OFFEX",    Region.ONSHORE,  "场外申赎市场, Off-exchange Subscription/Redemption")
    """场外申赎市场, 虚拟MIC"""
    
    MACRO    = ("MACRO",    "MACRO",    Region.GLB,    "宏观经济市场, Macro-economic")
    """宏观经济市场, 虚拟MIC"""
    
    # 美国
    USA      = ("USA",      "US",       Region.US,       "美国证券市场(泛指)")
    """美国证券市场(泛指), 虚拟MIC"""
    NYSE     = ("XNYS",     "US",       Region.US,       "纽约证券交易所")
    NASDAQ   = ("XNAS",     "US",       Region.US,       "纳斯达克")
    
    # 英国
    LSE      = ("XLON",     "UK",       Region.UK,       "伦敦证券交易所")
    """伦敦证券交易所"""
    GBR     = ("GBR",       "UK",       Region.UK,       "英国证券市场(泛指)")
    """英国证券市场(泛指)"""
    
    # 新加坡
    SGX     = ("XSES",      "SG",       Region.SG,       "新加坡交易所")
    """新加坡交易所"""
    
    # 其它
    MIRROR  = ("MIRROR",    "MIRROR",   Region.GLB,    "镜像市场, Mirror")
    """镜像市场, 虚拟MIC"""
    TEMP    = ("TEMP",      "TEMP",     Region.GLB,    "临时市场, Temporary")
    """临时市场, 虚拟MIC"""
    
    UNKNOWN  = ("UNKNOWN",  "UNKNOWN",  Region.UNKNOWN,  "未知交易所")
    """未知交易所, 虚拟MIC"""

    def __init__(
        self,
        mic: str,
        identifier: str,
        region: Region,
        label: str
    ):
        self.mic = mic
        """MIC: Market Identifier Code, used for exchanges and market identification"""
        self.identifier = identifier.lower()
        """标识: 交易所的小写缩写, 如 sh/sz/bj/hk, 与系统缓存的证券代码列表对应"""
        self.region = region  # 可为 None(如 UNKNOWN)
        """市场"""
        self.label = label
        """交易所名称"""

    @classmethod
    def parse(cls, s: str) -> "Exchange":
        """智能解析字符串为 Exchange 实例"""
        if not s:
            raise ValueError("Empty string cannot be parsed to Exchange")
        name_ = s.strip().upper()
        
        # 1. By code (enum name)
        try:
            return cls[name_]
        except KeyError:
            pass
        
        # 2. By identifier
        identifier_ = name_.lower()
        for ex in cls:
            if ex.identifier == identifier_:
                return ex

        # 3. By MIC
        for ex in cls:
            if ex.mic == name_:
                return ex

        raise ValueError(f"Cannot parse exchange from: '{s}'")

    @property
    def code(self) -> str:
        return self.name

    def __str__(self) -> str:
        return self.name

    def to_string(self) -> str:
        region_code = self.region.value if self.region else "None"
        return f"<Exchange.{self.name}: {self.identifier} ({region_code}) - {self.label}>"
    
    def __repr__(self) -> str:
       return self.to_string()

    @classmethod
    def from_code(cls, code: str) -> "Exchange":
        try:
            return cls[code]
        except KeyError:
            raise ValueError(f"Unknown exchange code: {code}")

    @classmethod
    def from_abbr(cls, abbr: str) -> "Exchange":
        for ex in cls:
            if ex.identifier == abbr:
                return ex
        raise ValueError(f"Unknown exchange abbreviation: {abbr}")

    @classmethod
    def from_mic(cls, mic: str) -> "Exchange":
        mic = mic.upper()
        for ex in cls:
            if ex.mic == mic:
                return ex
        raise ValueError(f"Unknown MIC: {mic}")
    
    # @property
    # def identifier(self) -> str:
    #     """市场标识符, 用于构建完整代码"""
    #     identifiers = {
    #         Exchange.SSE: "SH",
    #         Exchange.SZSE: "SZ",
    #         Exchange.BSE: "BJ",
    #         Exchange.HKEX: "HK",
    #     }
    #     return identifiers.get(self, "UNKNOWN")
    

if __name__ == "__main__":

    print(Exchange.SSE.identifier)
    exchange = Exchange.parse('sh')
    print(exchange.to_string())