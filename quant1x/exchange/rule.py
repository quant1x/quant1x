# -*- coding: UTF-8 -*-
from dataclasses import dataclass
from enum import Enum
import re
from .code import ExchangeId, ExchangeSSE, ExchangeSZSE, ExchangeBJSE, ExchangeHK, ExchangeUS


class SecurityType(Enum):
    Unknown = 0
    Stock = 1
    Index = 2
    ETF = 3
    Fund = 4
    BStock = 5
    Bond = 6
    IPO = 7
    Block = 8


@dataclass
class SecurityCode:
    exchange: ExchangeId
    symbol: str
    typ: SecurityType


# Exchange code strings used by detection
ALL_EXCHANGE_CODES = {
    ExchangeSSE: ExchangeId.ShangHai,
    ExchangeSZSE: ExchangeId.ShenZhen,
    ExchangeBJSE: ExchangeId.BeiJing,
    ExchangeHK: ExchangeId.HongKong,
    ExchangeUS: ExchangeId.USA,
}


# Rule tables (copied from C++ rule.cpp)
global_rules = [
    ("880", SecurityType.Block, "板块指数(通达信)"),
    ("881", SecurityType.Block, "板块指数(通达信)"),
]

sse_rules = [
    ("000", SecurityType.Index, "上证指数"),
    ("51", SecurityType.ETF, "上交所ETF(510-519)"),
    ("588", SecurityType.ETF, "科创板ETF"),
    ("50", SecurityType.Fund, "LOF/封闭式基金"),
    ("52", SecurityType.Fund, "其他基金"),
    ("600", SecurityType.Stock, "主板A股"),
    ("601", SecurityType.Stock, "主板A股"),
    ("603", SecurityType.Stock, "主板A股"),
    ("605", SecurityType.Stock, "主板A股"),
    ("688", SecurityType.Stock, "科创板"),
    ("689", SecurityType.Stock, "科创板CDR"),
    ("900", SecurityType.BStock, "B股"),
    ("110", SecurityType.Bond, "债券"),
    ("113", SecurityType.Bond, "可转债"),
    ("118", SecurityType.Bond, "可交换债"),
    ("120", SecurityType.Bond, "公司债"),
    ("123", SecurityType.Bond, "可转债"),
    ("127", SecurityType.Bond, "可转债"),
    ("128", SecurityType.Bond, "可转债"),
    ("730", SecurityType.IPO, "新股申购"),
    ("780", SecurityType.IPO, "新股申购"),
]

szse_rules = [
    ("399", SecurityType.Index, "深证指数"),
    ("159", SecurityType.ETF, "深交所ETF"),
    ("150", SecurityType.Fund, "LOF"),
    ("160", SecurityType.Fund, "LOF"),
    ("161", SecurityType.Fund, "LOF"),
    ("162", SecurityType.Fund, "LOF"),
    ("163", SecurityType.Fund, "LOF"),
    ("164", SecurityType.Fund, "LOF"),
    ("167", SecurityType.Fund, "LOF"),
    ("168", SecurityType.Fund, "LOF"),
    ("169", SecurityType.Fund, "LOF"),
    ("184", SecurityType.Fund, "封闭式基金"),
    ("000", SecurityType.Stock, "主板A股"),
    ("001", SecurityType.Stock, "主板A股"),
    ("002", SecurityType.Stock, "主板A股"),
    ("003", SecurityType.Stock, "主板A股"),
    ("300", SecurityType.Stock, "创业板"),
    ("301", SecurityType.Stock, "创业板"),
    ("200", SecurityType.BStock, "B股"),
    ("110", SecurityType.Bond, "可转债"),
    ("111", SecurityType.Bond, "可转债"),
    ("118", SecurityType.Bond, "可交换债"),
    ("123", SecurityType.Bond, "可转债"),
    ("127", SecurityType.Bond, "可转债"),
    ("128", SecurityType.Bond, "可转债"),
]

bjse_rules = [
    ("899", SecurityType.Index, "北交所指数"),
    ("920", SecurityType.Stock, "北交所股票(2024年起新上市)"),
    ("83", SecurityType.Stock, "北交所股票(原精选层)"),
    ("87", SecurityType.Stock, "北交所股票(原精选层)"),
    ("88", SecurityType.Stock, "北交所股票(2022-2023年上市)"),
    ("82", SecurityType.Bond, "优先股"),
    ("89", SecurityType.Bond, "可转债"),
]

hkse_rules = [
    ("HSI", SecurityType.Index, "恒生指数"),
    ("HSCEI", SecurityType.Index, "国企指数"),
    ("HSCCI", SecurityType.Index, "红筹指数"),
    ("028", SecurityType.ETF, "ETF"),
    ("030", SecurityType.ETF, "ETF"),
    ("031", SecurityType.ETF, "ETF"),
    ("090", SecurityType.ETF, "ETF"),
    ("091", SecurityType.ETF, "ETF"),
    ("08", SecurityType.Stock, "港股(GEM)"),
    ("0", SecurityType.Stock, "港股"),
    ("1", SecurityType.Bond, "权证"),
    ("2", SecurityType.Bond, "权证"),
    ("4", SecurityType.Bond, "牛熊证"),
    ("5", SecurityType.Bond, "牛熊证"),
    ("6", SecurityType.Bond, "牛熊证"),
]


def match_rule(code: str, rules):
    """Return (SecurityType, desc) for the longest matching prefix, or (Unknown, "")"""
    best_len = 0
    matched = SecurityType.Unknown
    desc = ""
    for prefix, typ, d in rules:
        if code.startswith(prefix) and len(prefix) > best_len:
            best_len = len(prefix)
            matched = typ
            desc = d
    return (matched, desc) if best_len > 0 else (SecurityType.Unknown, "")


def detect(input_str: str) -> SecurityCode:
    s = (input_str or "").strip()
    if not s:
        return SecurityCode(ExchangeId.ShangHai, "", SecurityType.Unknown)
    s = s.lower()
    pure_code = s

    symbol = ""
    exchange_id = ExchangeId.Unknown
    typ = SecurityType.Unknown

    # 1. explicit market flag prefix: sh600000
    if len(pure_code) >= 2 and pure_code[:2] in ALL_EXCHANGE_CODES:
        symbol = pure_code[2:]
        exchange_id = ALL_EXCHANGE_CODES[pure_code[:2]]
    # suffix form: 600000.sh or APPL.us
    elif len(pure_code) >= 3 and pure_code[-3] == '.' and pure_code[-2:] in ALL_EXCHANGE_CODES:
        symbol = pure_code[:-3]
        exchange_id = ALL_EXCHANGE_CODES[pure_code[-2:]]

    # 2. infer market if not set
    if exchange_id == ExchangeId.Unknown:
        if re.fullmatch(r"\d{6}", pure_code):
            symbol = pure_code
            if pure_code.startswith("6") or pure_code.startswith("5") or pure_code.startswith("9") or pure_code.startswith("7") or pure_code.startswith("000"):
                exchange_id = ExchangeId.ShangHai
            elif pure_code.startswith("0") or pure_code.startswith("3") or pure_code.startswith("1") or pure_code.startswith("2"):
                exchange_id = ExchangeId.ShenZhen
            elif pure_code.startswith("8") or pure_code.startswith("92"):
                exchange_id = ExchangeId.BeiJing
            else:
                return SecurityCode(ExchangeId.Unknown, "", SecurityType.Unknown)
        elif re.fullmatch(r"\d{5}", pure_code):
            symbol = pure_code
            exchange_id = ExchangeId.HongKong
        else:
            symbol = pure_code
    elif not symbol:
        symbol = pure_code

    # 4-letter all-alpha -> US stock (pure form)
    if exchange_id == ExchangeId.Unknown and len(symbol) == 4 and symbol.isalpha():
        exchange_id = ExchangeId.USA
        typ = SecurityType.Stock

    # 4. global rules priority
    if re.fullmatch(r"\d{6}", symbol):
        typ_, _ = match_rule(symbol, global_rules)
        if typ_ != SecurityType.Unknown:
            # global rules belong to SSE
            return SecurityCode(ExchangeId.ShangHai, symbol, typ_)

    # 5. match market rules
    if exchange_id == ExchangeId.Unknown:
        return SecurityCode(ExchangeId.Unknown, "", SecurityType.Unknown)

    if typ == SecurityType.Unknown:
        rules = None
        if exchange_id == ExchangeId.ShangHai:
            rules = sse_rules
        elif exchange_id == ExchangeId.ShenZhen:
            rules = szse_rules
        elif exchange_id == ExchangeId.BeiJing:
            rules = bjse_rules
        elif exchange_id == ExchangeId.HongKong:
            rules = hkse_rules
        elif exchange_id == ExchangeId.USA:
            return SecurityCode(exchange_id, symbol, SecurityType.Stock)
        else:
            return SecurityCode(ExchangeId.Unknown, "", SecurityType.Unknown)

        typ_, _ = match_rule(symbol, rules)
        if typ_ != SecurityType.Unknown:
            return SecurityCode(exchange_id, symbol, typ_)
        else:
            return SecurityCode(ExchangeId.Unknown, "", SecurityType.Unknown)
    else:
        return SecurityCode(exchange_id, symbol, typ)


__all__ = ["detect", "SecurityCode", "ExchangeId", "SecurityType"]
