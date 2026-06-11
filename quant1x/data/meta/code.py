# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from dataclasses import dataclass
from enum import IntEnum


# Market first list time
MarketCnFirstListTime = "1990-12-19"

# Exchange codes
ExchangeSSE = "sh"
ExchangeSZSE = "sz"
ExchangeBSE = "bj"
ExchangeHKEX = "hk"
ExchangeUS = "us"


class ExchangeId(IntEnum):
    Unknown = 255
    ShenZhen = 0
    ShangHai = 1
    BeiJing = 2
    HongKong = 21
    USA = 22

    def code(self) -> str:
        if self == ExchangeId.ShenZhen:
            return ExchangeSZSE
        if self == ExchangeId.ShangHai:
            return ExchangeSSE
        if self == ExchangeId.BeiJing:
            return ExchangeBSE
        if self == ExchangeId.HongKong:
            return ExchangeHKEX
        if self == ExchangeId.USA:
            return ExchangeUS
        raise ValueError(f"unknown ExchangeId: {int(self)}")

    def __str__(self) -> str:  # emulate Go's String()
        return self.code()


@dataclass
class ExchangeInfo:
    id: ExchangeId
    code: str
    name: str
    description: str = ""
    is_active: bool = True

    def __str__(self) -> str:
        return f"{self.name}({self.code})"

    def validate(self) -> None:
        if not self.code:
            raise ValueError("exchange code cannot be empty")
        if not self.name:
            raise ValueError("exchange name cannot be empty")


def new_exchange(code: str, name: str, desc: str, id: ExchangeId) -> ExchangeInfo:
    return ExchangeInfo(id=id, code=code, name=name, description=desc, is_active=True)


@dataclass
class SecurityCode:
    market: ExchangeId
    symbol: str

    def __str__(self) -> str:
        return f"{self.market}{self.symbol}"

    def validate(self) -> None:
        if not self.symbol:
            raise ValueError("security code symbol cannot be empty")


from enum import Enum
from typing import Tuple, List

# Canonical exchange flag aliases (use these for cross-language consistency)
EXCHANGE_SSE = "sh"
EXCHANGE_SZSE = "sz"
EXCHANGE_BSE = "bj"
EXCHANGE_HKEX = "hk"
EXCHANGE_US = "us"

MARKET_FLAGS = [
    EXCHANGE_SSE,
    EXCHANGE_SZSE,
    EXCHANGE_BSE,
    EXCHANGE_HKEX,
    EXCHANGE_US,
    EXCHANGE_SSE.upper(),
    EXCHANGE_SZSE.upper(),
    EXCHANGE_BSE.upper(),
    EXCHANGE_HKEX.upper(),
    EXCHANGE_US.upper(),
]

SHANGHAI_MAIN_BOARD_PREFIXES = ["50", "51", "60", "68", "90", "110", "113", "132", "204"]
SHANGHAI_SPECIAL_PREFIXES = ["5", "6", "9", "7"]
SECTOR_PREFIXES = ["880", "881"]
SHENZHEN_MAIN_BOARD_PREFIXES = ["00", "12", "13", "18", "15", "16", "18", "20", "30", "39", "115", "1318"]
BEIJING_MAIN_BOARD_PREFIXES = ["40", "43", "83", "87", "88", "420", "820", "899", "920"]

class MarketType(Enum):
    ShenZhen = 0
    ShangHai = 1
    BeiJing = 2
    HongKong = 21
    USA = 22

class TargetKind(Enum):
    STOCK = 0
    INDEX = 1
    BLOCK = 2
    ETF = 3

def starts_with(s: str, prefixes: List[str]) -> bool:
    return any(s.startswith(prefix) for prefix in prefixes)

def ends_with(s: str, suffixes: List[str]) -> bool:
    return any(s.endswith(suffix) for suffix in suffixes)

def get_security_code(market: MarketType, symbol: str) -> str:
    if market == MarketType.USA:
        return EXCHANGE_US + symbol
    elif market == MarketType.HongKong:
        return EXCHANGE_HKEX + symbol[:5]
    elif market == MarketType.BeiJing:
        return EXCHANGE_BSE + symbol[:6]
    elif market == MarketType.ShenZhen:
        return EXCHANGE_SZSE + symbol[:6]
    else:
        return EXCHANGE_SSE + symbol[:6]

def get_market(symbol: str) -> str:
    code = symbol.strip()
    market = EXCHANGE_SSE

    if starts_with(code, MARKET_FLAGS):
        market = code[:2].lower()
    elif ends_with(code, MARKET_FLAGS):
        market = code[-2:].lower()
    elif starts_with(code, SHANGHAI_MAIN_BOARD_PREFIXES):
        market = EXCHANGE_SSE
    elif starts_with(code, SHENZHEN_MAIN_BOARD_PREFIXES):
        market = EXCHANGE_SZSE
    elif starts_with(code, SHANGHAI_SPECIAL_PREFIXES):
        market = EXCHANGE_SSE
    elif starts_with(code, SECTOR_PREFIXES):
        market = EXCHANGE_SSE
    elif starts_with(code, BEIJING_MAIN_BOARD_PREFIXES):
        market = EXCHANGE_BSE
    elif code.isalpha():
        market = EXCHANGE_US
    
    return market

def get_market_id(symbol: str) -> MarketType:
    market = get_market(symbol)
    if market == EXCHANGE_SSE:
        return MarketType.ShangHai
    if market == EXCHANGE_SZSE:
        return MarketType.ShenZhen
    if market == EXCHANGE_BSE:
        return MarketType.BeiJing
    if market == EXCHANGE_HKEX:
        return MarketType.HongKong
    if market == EXCHANGE_US:
        return MarketType.USA
    return MarketType.ShangHai

def get_market_flag(market_id: MarketType) -> str:
    if market_id == MarketType.ShenZhen:
        return EXCHANGE_SZSE
    elif market_id == MarketType.BeiJing:
        return EXCHANGE_BSE
    elif market_id == MarketType.HongKong:
        return EXCHANGE_HKEX
    elif market_id == MarketType.USA:
        return EXCHANGE_US
    else:
        return EXCHANGE_SSE

def detect_market(symbol: str) -> Tuple[MarketType, str, str]:
    pure_code = symbol.strip()
    market_code = EXCHANGE_SSE

    if starts_with(pure_code, MARKET_FLAGS):
        market_code = pure_code[:2].lower()
        if len(pure_code) > 2 and pure_code[2] == '.':
            pure_code = pure_code[3:]
        else:
            pure_code = pure_code[2:]
    elif ends_with(pure_code, MARKET_FLAGS):
        market_code = pure_code[-2:].lower()
        pure_code = pure_code[:-3]
    elif starts_with(pure_code, SHANGHAI_MAIN_BOARD_PREFIXES):
        market_code = EXCHANGE_SSE
    elif starts_with(pure_code, SHENZHEN_MAIN_BOARD_PREFIXES):
        market_code = EXCHANGE_SZSE
    elif starts_with(pure_code, SHANGHAI_SPECIAL_PREFIXES):
        market_code = EXCHANGE_SSE
    elif starts_with(pure_code, SECTOR_PREFIXES):
        market_code = EXCHANGE_SSE
    elif starts_with(pure_code, BEIJING_MAIN_BOARD_PREFIXES):
        market_code = EXCHANGE_BSE

    market_id = MarketType.ShangHai
    if market_code == EXCHANGE_SSE:
        market_id = MarketType.ShangHai
    elif market_code == EXCHANGE_SZSE:
        market_id = MarketType.ShenZhen
    elif market_code == EXCHANGE_BSE:
        market_id = MarketType.BeiJing
    elif market_code == EXCHANGE_HKEX:
        market_id = MarketType.HongKong
    elif market_code == EXCHANGE_US:
        market_id = MarketType.USA
    
    return market_id, market_code, pure_code

def assert_index_by_market_and_code(market_id: MarketType, symbol: str) -> bool:
    if market_id == MarketType.ShangHai and starts_with(symbol, ["000", "880", "881"]):
        return True
    if market_id == MarketType.ShenZhen and starts_with(symbol, ["399"]):
        return True
    if market_id == MarketType.BeiJing and starts_with(symbol, ["899"]):
        return True
    return False

def assert_index_by_security_code(security_code: str) -> bool:
    market_id, _, code = detect_market(security_code)
    return assert_index_by_market_and_code(market_id, code)

def assert_block_by_security_code(security_code: str) -> Tuple[bool, str]:
    market_id, flag, code = detect_market(security_code)
    if market_id != MarketType.ShangHai or not starts_with(code, SECTOR_PREFIXES):
        return False, security_code
    return True, flag + code

def assert_etf_by_market_and_code(market_id: MarketType, symbol: str) -> bool:
    return market_id == MarketType.ShangHai and starts_with(symbol, ["510"])

def assert_stock_by_market_and_code(market_id: MarketType, symbol: str) -> bool:
    if market_id == MarketType.ShangHai and starts_with(symbol, ["60", "68", "510"]):
        return True
    if market_id == MarketType.ShenZhen and starts_with(symbol, ["00", "30"]):
        return True
    if market_id == MarketType.BeiJing and starts_with(symbol, ["40", "43", "83", "87", "88", "420", "820", "920"]):
        return True
    return False

def assert_stock_by_security_code(security_code: str) -> bool:
    market_id, _, code = detect_market(security_code)
    return assert_stock_by_market_and_code(market_id, code)

def correct_security_code(symbol: str) -> str:
    if not symbol:
        return ""
    _, m_flag, m_symbol = detect_market(symbol)
    return m_flag + m_symbol

def assert_code(security_code: str) -> TargetKind:
    market_id, _, code = detect_market(security_code)
    if market_id == MarketType.ShangHai:
        if starts_with(code, SECTOR_PREFIXES):
            return TargetKind.BLOCK
        if starts_with(code, ["000"]):
            return TargetKind.INDEX
        if starts_with(code, ["5"]):
            return TargetKind.ETF
    if market_id == MarketType.ShenZhen:
        if starts_with(code, ["399"]):
            return TargetKind.INDEX
        if starts_with(code, ["159"]):
            return TargetKind.ETF
    if market_id == MarketType.BeiJing and starts_with(code, ["899"]):
        return TargetKind.INDEX
    return TargetKind.STOCK

def check_index_and_stock(security_code: str) -> bool:
    if assert_index_by_security_code(security_code):
        return True
    if assert_stock_by_security_code(security_code):
        return True
    return False
