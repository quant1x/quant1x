# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from typing import List

from quant1x.std.numeric import NumberRange

from .meta.exchange import Exchange
from .meta.instrument import Instrument, InstrumentType

# 证券代码规则定义
from .meta.ticker_rules.rule import CodeRule, global_rules
from .meta.ticker_rules.market_sse import sse_rules
from .meta.ticker_rules.market_szse import szse_rules
from .meta.ticker_rules.market_bse import bse_rules
from .meta.ticker_rules.market_hkex import hkex_rules

def match_rule(code: str, rules: List[CodeRule]) -> CodeRule:
    """
    根据代码前缀匹配最优规则
    
    Args:
        code (str): 需要匹配的代码字符串
        rules (List[CodeRule]): 可匹配的规则列表
    
    Returns:
        CodeRule: 匹配到的最优规则，若无匹配则返回默认的未知规则
    """
    code = code.upper().strip()
    best_match: CodeRule | None = None
    best_len = 0

    for entry in rules:
        prefix = entry.prefix
        #print(prefix)
        # 跳过空前缀(可选，根据业务)
        if not prefix:
            continue
        if isinstance(prefix, str) and code.startswith(prefix) and len(prefix) > best_len:
            best_len = len(prefix)
            best_match = entry
        elif isinstance(prefix, NumberRange):
            prefix_len = prefix.max_value_length()
            if code in prefix and prefix_len > best_len:
                best_len = prefix_len
                best_match = entry
                break

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

# 所有交易所标识
ALL_EXCHANGE_IDENTIFIERS = {
    Exchange.SSE.identifier,
    Exchange.SZSE.identifier,
    Exchange.BSE.identifier,
    Exchange.HKEX.identifier,
    Exchange.HKFE.identifier,
    Exchange.USA.identifier,
}

def detect_symbol(input_str: str) -> Instrument:
    """
    检测并解析证券代码的市场类型及证券类型
    
    Args:
        input_str (str): 输入的证券代码字符串，支持多种格式: 
            - 前缀形式: sh600000(上海交易所)
            - 后缀形式: 600000.sh 或 APPL.us
            - 纯数字形式: 600000(自动推断交易所)
            - 4字母全大写: AAPL(自动识别为美股)
    
    Returns:
        Instrument: 包含以下属性的对象: 
            - exchange_id: 交易所标识
            - symbol: 纯证券代码
            - instrument_type: 证券类型
    
    Note:
        支持识别以下交易所: 
            - 上海证券交易所(SSE)
            - 深圳证券交易所(SZSE) 
            - 北京证券交易所(BSE)
            - 香港交易所(HKEX)
            - 香港期货交易所(HKFE)
            - 美国市场(USA)
        自动根据代码规则推断证券类型(股票、债券等)
    """
    #print(f"detect_symbol: {input_str}")
    s = (input_str or "").strip()
    if not s:
        return Instrument(Exchange.SSE, InstrumentType.Unknown, "", "", 0, 0)
    s = s.lower()
    pure_code = s

    ticker = ""
    exchange = Exchange.UNKNOWN
    typ = InstrumentType.Unknown

    # 1. 判断前缀: sh600000
    if len(pure_code) >= 2 and pure_code[:2] in ALL_EXCHANGE_IDENTIFIERS:
        ticker = pure_code[2:]
        prefix_code = pure_code[:2]
        exchange = Exchange.parse(prefix_code)
        # 走指定市场规则
    # 2. 判断后缀: 600000.sh or APPL.us
    elif len(pure_code) >= 3 and pure_code[-3] == '.' and pure_code[-2:] in ALL_EXCHANGE_IDENTIFIERS:
        ticker = pure_code[:-3]
        suffix_code = pure_code[-2:]
        exchange = Exchange.parse(suffix_code)
        # 走指定市场规则
        #print(f"ticker: {ticker}, exchange: {exchange}")
    else:
        # 纯数字或者字母
        code_len = len(pure_code)
        match code_len:
            case 4:
                if pure_code.isalpha():
                    exchange = Exchange.USA
                    typ = InstrumentType.STOCK
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
            case 5:
                if pure_code.isdigit():
                    exchange = Exchange.HKEX
                    typ = InstrumentType.STOCK
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
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
            return Instrument(exchange, InstrumentType.STOCK, pure_code, "", 0, 0)
        else:
            return Instrument(Exchange.UNKNOWN, InstrumentType.Unknown, "", "", 0, 0)

        cr = match_rule(ticker, rules)
        #print(f"cr: {cr}")
        if cr.type != InstrumentType.Unknown:
            return Instrument(cr.exchange, cr.type, ticker, "", 0, 0)
        else:
            return Instrument(Exchange.UNKNOWN, InstrumentType.Unknown, "", "", 0, 0)
    else:
        return Instrument(exchange, typ, ticker, "", 0, 0)

def correct_security_code(code: str) -> str:
    """
    纠正证券代码格式，补全前缀或后缀
    
    Args:
        code (str): 输入的证券代码字符串，支持多种格式: 
            - 前缀形式: sh600000(上海交易所)
            - 后缀形式: 600000.sh 或 APPL.us
            - 纯数字形式: 600000(自动推断交易所)
            - 4字母全大写: AAPL(自动识别为美股)  
            - 6位数字: 600000(自动推断交易所)
            - 4字母全大写: AAPL(自动识别为美股)
    
    Returns:
        str: 纠正后的证券代码字符串
    """
    inst = detect_symbol(code)
    if inst.can_construct_symbol():
        return inst.symbol()
    else:
        raise ValueError(f"无法纠正证券代码: {code}")

PRE_MARKET_HOUR = 9
PRE_MARKET_MINUTE = 0
PRE_MARKET_SECOND = 0
cn_cron_expr_daily_init = f"0 {PRE_MARKET_HOUR} {PRE_MARKET_MINUTE} * * *"

__all__ = [
    "detect_instrument_type_by_rule",
    "detect_symbol",
    "cn_cron_expr_daily_init",
]


if __name__ == "__main__":
    symbol = detect_symbol("600000.sh")
    print(symbol.to_string())
    
    symbol = detect_symbol("hsi.hk")
    print(symbol.to_string())
    
    symbol = detect_symbol("85000.hk")
    print(symbol.to_string())