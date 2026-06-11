# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from typing import Tuple, Dict
from quant1x.data.meta import Exchange
from quant1x.data.meta import InstrumentType

# 正向映射：(market, category) → (mic, asset_class, protocol_segment)
TDX_MARKET_CATEGORY_MAPPING: Dict[Tuple[int, int], Tuple[Exchange, InstrumentType]] = {
    # 1,1,临时股,TP
    (0, 0): (Exchange.TEMP, InstrumentType.OTHER),
    (1, 1): (Exchange.TEMP, InstrumentType.STOCK),
    (1, 12): (Exchange.TEMP, InstrumentType.OPTION),
    # 43,1,B股转H股,HB
    (43, 1): (Exchange.TEMP, InstrumentType.BSTOCK),
    # 44,1,股转系统,SB
    (44, 1): (Exchange.TEMP, InstrumentType.NEEQ),
    
    # 香港市场
    # 27,5,香港指数,FH
    (27, 5): (Exchange.HKEX, InstrumentType.INDEX),
    # 31,2,香港主板,KH
    (31, 2): (Exchange.HKEX, InstrumentType.STOCK),
    # 48,2,香港创业板,KG
    (48, 2): (Exchange.HKEX, InstrumentType.GEM_MARKET),
    # 22,2,香港债券,KB
    (22, 2): (Exchange.HKEX, InstrumentType.BOND),
    # 32,2,香港权证,KR
    (32, 2): (Exchange.HKEX, InstrumentType.WARRANT),
    # 49,2,香港基金,KT
    (49, 2): (Exchange.HKEX, InstrumentType.FUND),
    # 71,2,港股通,GH
    (71, 2): (Exchange.HKSC, InstrumentType.STOCK),
    
    # 期权
    
    # 8,12,上海股票期权,QQ
    (8, 12): (Exchange.SSE, InstrumentType.OPTION),
    # 9,12,深圳股票期权,SQ
    
    # 4,12,郑州商品期权,OZ
    (4, 12): (Exchange.CZCE, InstrumentType.OPTION),
    # 5,12,大连商品期权,OD
    (5, 12): (Exchange.DCE, InstrumentType.OPTION),
    # 6,12,上海商品期权,OS
    (6, 12): (Exchange.SHFE, InstrumentType.OPTION),
    # 7,12,中金所期权,OJ
    (7, 12): (Exchange.CFFEX, InstrumentType.OPTION),
    (9, 12): (Exchange.SZSE, InstrumentType.OPTION),
    # 67,12,广州期权,OG
    (67, 12): (Exchange.GFEX, InstrumentType.OPTION),
    
    # 期货
    # 28,3,郑州商品,QZ
    (28, 3): (Exchange.CZCE, InstrumentType.FUTURE),
    # 29,3,大连商品,QD
    (29, 3): (Exchange.DCE, InstrumentType.FUTURE),
    # 30,3,上海期货,QS
    (30, 3): (Exchange.SHFE, InstrumentType.FUTURE),
    # 46,11,上海黄金,SG
    (46, 11): (Exchange.SGE, InstrumentType.FUTURE),
    # 55,3,上海黄金,HJ
    (55, 3): (Exchange.SGE, InstrumentType.COMMODITY),
    # 47,3,中金所期货,CZ
    (47, 3): (Exchange.CFFEX, InstrumentType.FUTURE),
    (47, 5): (Exchange.CFFEX, InstrumentType.INDEX),
    # 50,3,渤海商品,BH
    # 53,3,大宗连续,DL
    # 65,3,广州套利期货,EG
    #(65, 3): (Exchange.GFEX, InstrumentType.FUTURE),
    # 66,3,广州期货,QG
    (66, 3): (Exchange.GFEX, InstrumentType.FUTURE),
    # 76,3,齐鲁商品,QL
    #(76, 3): (Exchange.CFE, InstrumentType.FUTURE),
    (23, 3): (Exchange.HKFE, InstrumentType.FUTURE),
    
    # 13,3,国际贵金属,GO
    # 14,3,伦敦金属,LM
    # 15,3,伦敦石油,IP
    # 16,3,纽约商品,CO
    # 17,3,纽约石油,NY
    # 20,3,纽约期货,NB
    # 18,3,芝加哥谷,CB
    # 19,3,东京工业品,TO
    # 39,3,马来期货,ML
    # 52,3,东北亚商品,DB
    
    
    # 10,4,基本汇率,FE
    # 11,4,交叉汇率,FX
    # 12,5,国际指数,WI
    (12, 5): (Exchange.OFFSHORE, InstrumentType.INDEX),
    
    # 33,8,开放式基金,FU
    (33, 8): (Exchange.OFFEX, InstrumentType.FUND),
    # 34,9,货币型基金,FB
    (34, 9): (Exchange.OFFEX, InstrumentType.MONEY_FUND),
    # 37,11,全球指数(静态),FW
    # 38,10,宏观指标,HG
    (38, 10): (Exchange.MACRO, InstrumentType.MACRO_INDICATOR),
    # 40,11,中国概念股,CH
    # 41,11,美股知名公司,MG
    # 42,3,商品指数,TI
    (42, 3): (Exchange.ONSHORE, InstrumentType.INDEX),
    # 45,6,OTC市场,OT
    (45, 6): (Exchange.OTC, InstrumentType.OTHER),
    # 54,6,国债预发行,GY
    # 56,8,阳光私募基金,TA
    # 57,8,券商集合理财,TB
    # 58,9,券商货币理财,TC
    # 60,3,主力期货合约,MA
    # 62,5,中证指数,ZZ
    (62, 5): (Exchange.CSI, InstrumentType.INDEX),
    # 70,5,扩展板块指数,UZ
    (70, 5): (Exchange.EXTENDED, InstrumentType.INDEX),
    # 74,13,美国股票,US
    (74, 13): (Exchange.USA, InstrumentType.STOCK),
    # 75,14,英国股票,UK
    (75, 14): (Exchange.GBR, InstrumentType.STOCK),
    # 78,15,新加坡股票,SE
    (78, 15): (Exchange.SGX, InstrumentType.STOCK),
    # 100,11,代码镜像,CM
    (100, 11): (Exchange.MIRROR, InstrumentType.OTHER),
    # 102,5,国证指数,GZ
    (102, 5): (Exchange.CNI, InstrumentType.INDEX),
}

# 反向映射：(mic, asset_class, protocol_segment) → (market, category, endpoint)
TDX_REVERSE_ROUTING: Dict[Tuple[Exchange, InstrumentType], Tuple[int, int]] = {
    (mic, asset_cls): (market, category) for (market, category), (mic, asset_cls) in TDX_MARKET_CATEGORY_MAPPING.items()
}

def find_exchange_by_market_and_category(market: int, category: int) -> Tuple[Exchange, InstrumentType]:
    """
    根据市场编号和类别编号查找对应的交易所和资产类型
    
    Args:
        market (int): 市场编号
        category (int): 类别编号
    
    Returns:
        Tuple[Exchange, InstrumentType]: 包含交易所和资产类型的元组
    
    Raises:
        ValueError: 当找不到指定市场编号和类别编号的映射时
    """
    if market in (0,1):
        return Exchange.TEMP, InstrumentType.OTHER
    result = TDX_MARKET_CATEGORY_MAPPING.get((market, category))
    if result is None:
        raise ValueError(f"未找到市场 {market} 和类别 {category} 的映射配置")
    return result

def find_market_by_exchange_and_asset_class(exchange: Exchange, asset_class: InstrumentType) -> Tuple[int, int]:
    """
    根据交易所和资产类别查找对应的市场编号和类别编号
    
    Args:
        exchange (Exchange): 交易所枚举值
        asset_class (InstrumentType): 资产类别枚举值
    
    Returns:
        Tuple[int, int]: 返回对应的市场编号和类别编号元组
    
    Raises:
        ValueError: 当找不到对应的映射配置时抛出
    """
    if exchange == Exchange.TEMP:
        return 1, 1
    result = TDX_REVERSE_ROUTING.get((exchange, asset_class))
    if result is None:
        raise ValueError(f"未找到交易所 {exchange} 和资产类别 {asset_class} 的映射配置")
    return result

if __name__ == '__main__':
    print(find_exchange_by_market_and_category(27, 51))
    print(find_market_by_exchange_and_asset_class(Exchange.HKEX, InstrumentType.INDEX))