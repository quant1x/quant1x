# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from .rule import CodeRule
from ..exchange import Exchange
from ..instrument import InstrumentType

# USA 美国证券交易所规则
usa_rules = [
    CodeRule(Exchange.OFFSHORE, "IXIC", InstrumentType.INDEX, "指数", "纳斯达克指数"),
    CodeRule(Exchange.OFFSHORE, "DAX", InstrumentType.INDEX, "指数", "德国DAX指数"),
    CodeRule(Exchange.EXTENDED, "US", InstrumentType.SECTOR, "指数", "美国板块指数"),
    CodeRule(Exchange.USA, "", InstrumentType.STOCK, "挂牌公司普通股", ""),
]

_ticker_to_protocol_symbol_mapping = {
    "IXIC": "A_IXIC", # 纳斯达克指数
    "DAX": "B_DAX", # 德国DAX指数
}

def usa_ticker_to_code(ticker: str) -> str:
    """
    将美国股票代码转换为行情标准的代码
    
    Args:
        ticker (str): 输入的美国股票代码
    
    Returns:
        str: 转换后的标准符号，如果未找到映射则返回原代码
    """
    ticker = ticker.upper()
    code = _ticker_to_protocol_symbol_mapping.get(ticker, ticker)
    return code

def usa_code_to_ticker(code: str) -> str:
    """
    将美国股票协议代码转换为对应的股票代码
    
    Args:
        code (str): 输入的美国股票协议代码
    
    Returns:
        str: 对应的股票代码，如果未找到映射则返回原输入代码
    """
    for ticker, mapped_code in _ticker_to_protocol_symbol_mapping.items():
        if mapped_code == code:
            return ticker
    return ''
