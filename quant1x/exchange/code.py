from enum import Enum
from typing import Tuple


class MarketType(Enum):
    SHENZHEN = 0  # 深圳
    SHANGHAI = 1  # 上海
    BEIJING = 2  # 北京
    HONGKONG = 21  # 香港
    USA = 22  # 美国


MARKET_FLAGS = ["sh", "sz", "SH", "SZ", "bj", "BJ", "hk", "HK", "us", "US"]


def starts_with(s: str, prefixes) -> bool:
    return any(s.startswith(prefix) for prefix in prefixes)


def ends_with(s: str, suffixes) -> bool:
    return any(s.endswith(suffix) for suffix in suffixes)


def get_market_id(symbol: str) -> MarketType:
    symbol = (symbol or "").strip()
    if starts_with(symbol, ["sh", "SH"]):
        return MarketType.SHANGHAI
    if starts_with(symbol, ["sz", "SZ"]):
        return MarketType.SHENZHEN
    if starts_with(symbol, ["bj", "BJ"]):
        return MarketType.BEIJING
    if starts_with(symbol, ["hk", "HK"]):
        return MarketType.HONGKONG
    if starts_with(symbol, ["us", "US"]):
        return MarketType.USA
    if starts_with(symbol, ["50", "51", "60", "68", "90", "110", "113", "132", "204"]):
        return MarketType.SHANGHAI
    if starts_with(symbol, ["00", "12", "13", "18", "15", "16", "20", "30", "39", "115", "1318"]):
        return MarketType.SHENZHEN
    if starts_with(symbol, ["5", "6", "9", "7"]):
        return MarketType.SHANGHAI
    if starts_with(symbol, ["88"]):
        return MarketType.SHANGHAI
    if starts_with(symbol, ["4", "8"]):
        return MarketType.BEIJING
    return MarketType.SHANGHAI


def detect_market(symbol: str) -> Tuple[MarketType, str, str]:
    code = (symbol or "").strip()
    market = "sh"
    if starts_with(code, MARKET_FLAGS):
        market = code[:2].lower()
        code = code[2:].lstrip('.')
    elif ends_with(code, MARKET_FLAGS):
        market = code[-2:].lower()
        code = code[:-3].rstrip('.')
    elif starts_with(code, ["50", "51", "60", "68", "90", "110", "113", "132", "204"]):
        market = "sh"
    elif starts_with(code, ["00", "12", "13", "18", "15", "16", "20", "30", "39", "115", "1318"]):
        market = "sz"
    elif starts_with(code, ["5", "6", "9", "7"]):
        market = "sh"
    elif starts_with(code, ["88"]):
        market = "sh"
    elif starts_with(code, ["4", "8"]):
        market = "bj"
    market_id = get_market_id(market)
    return market_id, market, code


def correct_security_code(security_code: str) -> str:
    if not security_code:
        return ""
    _, market, code = detect_market(security_code)
    return f"{market}{code}"
