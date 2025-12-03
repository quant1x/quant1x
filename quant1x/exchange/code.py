from enum import Enum
from typing import Tuple, List

# Constants
MARKET_SHANGHAI = "sh"
MARKET_SHENZHEN = "sz"
MARKET_BEIJING = "bj"
MARKET_HONGKONG = "hk"
MARKET_USA = "us"

MARKET_FLAGS = ["sh", "sz", "SH", "SZ", "bj", "BJ", "hk", "HK", "us", "US"]

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
    """
    检查字符串是否以给定的任意前缀开头
    
    Args:
        s (str): 要检查的目标字符串
        prefixes (List[str]): 可能的前缀列表
    
    Returns:
        bool: 如果字符串以任一前缀开头则返回True，否则返回False
    """
    return any(s.startswith(prefix) for prefix in prefixes)

def ends_with(s: str, suffixes: List[str]) -> bool:
    """
    检查字符串是否以给定的任一后缀结尾
    
    Args:
        s (str): 要检查的字符串
        suffixes (List[str]): 可能的后缀列表
    
    Returns:
        bool: 如果字符串以任一后缀结尾则返回True，否则返回False
    """
    return any(s.endswith(suffix) for suffix in suffixes)

def get_security_code(market: MarketType, symbol: str) -> str:
    """
    根据市场类型和证券代码生成完整的证券代码字符串
    
    Args:
        market (MarketType): 市场类型枚举值
        symbol (str): 原始证券代码
    
    Returns:
        str: 根据市场规则生成的完整证券代码字符串
    """
    if market == MarketType.USA:
        return MARKET_USA + symbol
    elif market == MarketType.HongKong:
        return MARKET_HONGKONG + symbol[:5]
    elif market == MarketType.BeiJing:
        return MARKET_BEIJING + symbol[:6]
    elif market == MarketType.ShenZhen:
        return MARKET_SHENZHEN + symbol[:6]
    else:
        return MARKET_SHANGHAI + symbol[:6]

def get_market(symbol: str) -> str:
    """
    根据股票代码判断其所属的市场
    
    Args:
        symbol (str): 股票代码字符串，可能包含市场前缀或后缀标识
        
    Returns:
        str: 市场标识符的小写形式，如'sh'表示上海市场，'sz'表示深圳市场，'bj'表示北京市场，'hk'表示香港市场，'us'表示美国市场
    
    说明:
        1. 首先检查代码开头或结尾是否包含市场标识符
        2. 如果没有显式市场标识，则根据代码前缀判断所属市场
        3. 默认返回上海市场标识
    """
    code = symbol.strip()
    market = MARKET_SHANGHAI

    if starts_with(code, MARKET_FLAGS):
        market = code[:2].lower()
    elif ends_with(code, MARKET_FLAGS):
        market = code[-2:].lower()
    elif starts_with(code, SHANGHAI_MAIN_BOARD_PREFIXES):
        market = MARKET_SHANGHAI
    elif starts_with(code, SHENZHEN_MAIN_BOARD_PREFIXES):
        market = MARKET_SHENZHEN
    elif starts_with(code, SHANGHAI_SPECIAL_PREFIXES):
        market = MARKET_SHANGHAI
    elif starts_with(code, SECTOR_PREFIXES):
        market = MARKET_SHANGHAI
    elif starts_with(code, BEIJING_MAIN_BOARD_PREFIXES):
        market = MARKET_BEIJING
    elif code.isalpha():
        market = MARKET_USA
    
    return market

def get_market_id(symbol: str) -> MarketType:
    """
    根据股票代码符号获取对应的市场类型
    
    Args:
        symbol (str): 股票代码符号，例如 '600000' 或 '000001'
    
    Returns:
        MarketType: 返回对应的市场枚举值，包括:
            - MarketType.ShangHai: 上海市场
            - MarketType.ShenZhen: 深圳市场 
            - MarketType.BeiJing: 北京市场
            - MarketType.HongKong: 香港市场
            - MarketType.USA: 美国市场
            默认返回上海市场
    """
    market = get_market(symbol)
    if market == MARKET_SHANGHAI:
        return MarketType.ShangHai
    if market == MARKET_SHENZHEN:
        return MarketType.ShenZhen
    if market == MARKET_BEIJING:
        return MarketType.BeiJing
    if market == MARKET_HONGKONG:
        return MarketType.HongKong
    if market == MARKET_USA:
        return MarketType.USA
    return MarketType.ShangHai

def get_market_flag(market_id: MarketType) -> str:
    """
    根据市场类型ID获取对应的市场标志字符串
    
    Args:
        market_id (MarketType): 市场类型枚举值
    
    Returns:
        str: 对应市场的标志字符串，可能的返回值包括:
            - MARKET_SHENZHEN: 深圳市场
            - MARKET_BEIJING: 北京市场
            - MARKET_HONGKONG: 香港市场
            - MARKET_USA: 美国市场
            - MARKET_SHANGHAI: 上海市场(默认值)
    """
    if market_id == MarketType.ShenZhen:
        return MARKET_SHENZHEN
    elif market_id == MarketType.BeiJing:
        return MARKET_BEIJING
    elif market_id == MarketType.HongKong:
        return MARKET_HONGKONG
    elif market_id == MarketType.USA:
        return MARKET_USA
    else:
        return MARKET_SHANGHAI

def detect_market(symbol: str) -> Tuple[MarketType, str, str]:
    """
    根据股票代码识别所属市场类型并提取纯净代码
    
    Args:
        symbol (str): 原始股票代码字符串，可能包含市场标识前缀/后缀(如'sh', 'sz')或特殊前缀
    
    Returns:
        Tuple[MarketType, str, str]: 返回三元组，包含:
            - 市场类型枚举值(MarketType)
            - 市场代码字符串(如'sh', 'sz')
            - 去除市场标识后的纯净股票代码
    
    Note:
        支持以下识别方式:
        1. 市场标识前缀(如'sh600000')
        2. 市场标识后缀(如'600000.sh')
        3. 上海/深圳/北京主板特殊前缀
        4. 板块指数特殊前缀
    """
    pure_code = symbol.strip()
    market_code = MARKET_SHANGHAI

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
        market_code = MARKET_SHANGHAI
    elif starts_with(pure_code, SHENZHEN_MAIN_BOARD_PREFIXES):
        market_code = MARKET_SHENZHEN
    elif starts_with(pure_code, SHANGHAI_SPECIAL_PREFIXES):
        market_code = MARKET_SHANGHAI
    elif starts_with(pure_code, SECTOR_PREFIXES):
        market_code = MARKET_SHANGHAI
    elif starts_with(pure_code, BEIJING_MAIN_BOARD_PREFIXES):
        market_code = MARKET_BEIJING

    market_id = MarketType.ShangHai
    if market_code == MARKET_SHANGHAI:
        market_id = MarketType.ShangHai
    elif market_code == MARKET_SHENZHEN:
        market_id = MarketType.ShenZhen
    elif market_code == MARKET_BEIJING:
        market_id = MarketType.BeiJing
    elif market_code == MARKET_HONGKONG:
        market_id = MarketType.HongKong
    elif market_code == MARKET_USA:
        market_id = MarketType.USA
    
    return market_id, market_code, pure_code

def assert_index_by_market_and_code(market_id: MarketType, symbol: str) -> bool:
    """
    根据市场类型和证券代码验证是否为指数证券
    
    Args:
        market_id (MarketType): 市场类型枚举值（ShangHai/ShenZhen/BeiJing）
        symbol (str): 证券代码字符串
    
    Returns:
        bool: 如果代码符合该市场的索引规则返回True，否则返回False
    """
    if market_id == MarketType.ShangHai and starts_with(symbol, ["000", "880", "881"]):
        return True
    if market_id == MarketType.ShenZhen and starts_with(symbol, ["399"]):
        return True
    if market_id == MarketType.BeiJing and starts_with(symbol, ["899"]):
        return True
    return False

def assert_index_by_security_code(security_code: str) -> bool:
    """
    根据证券代码验证是否为指数证券
    
    Args:
        security_code (str): 证券代码字符串
    
    Returns:
        bool: 如果是指数证券返回True，否则返回False
    """
    market_id, _, code = detect_market(security_code)
    return assert_index_by_market_and_code(market_id, code)

def assert_block_by_security_code(security_code: str) -> Tuple[bool, str]:
    """
    根据证券代码验证是否为板块代码，并返回处理后的代码
    
    Args:
        security_code (str): 待验证的证券代码
    
    Returns:
        Tuple[bool, str]: 返回元组，第一个元素表示是否为板块代码，
                         第二个元素为处理后的代码(原代码或带前缀的代码)
    """
    market_id, flag, code = detect_market(security_code)
    if market_id != MarketType.ShangHai or not starts_with(code, SECTOR_PREFIXES):
        return False, security_code
    return True, flag + code

def assert_etf_by_market_and_code(market_id: MarketType, symbol: str) -> bool:
    """
    检查给定市场和代码是否匹配ETF的规则
    
    Args:
        market_id (MarketType): 市场类型枚举值
        symbol (str): 证券代码
    
    Returns:
        bool: 如果市场是上海市场且代码以'510'开头则返回True，否则返回False
    """
    return market_id == MarketType.ShangHai and starts_with(symbol, ["510"])

def assert_stock_by_market_and_code(market_id: MarketType, symbol: str) -> bool:
    """
    根据市场类型和股票代码验证股票是否属于指定市场
    
    Args:
        market_id (MarketType): 市场类型枚举值(ShangHai/ShenZhen/BeiJing)
        symbol (str): 股票代码字符串
    
    Returns:
        bool: 如果股票代码符合该市场的编码规则返回True，否则返回False
    """
    if market_id == MarketType.ShangHai and starts_with(symbol, ["60", "68", "510"]):
        return True
    if market_id == MarketType.ShenZhen and starts_with(symbol, ["00", "30"]):
        return True
    if market_id == MarketType.BeiJing and starts_with(symbol, ["40", "43", "83", "87", "88", "420", "820", "920"]):
        return True
    return False

def assert_stock_by_security_code(security_code: str) -> bool:
    """
    根据证券代码验证股票代码格式是否正确
    
    Args:
        security_code (str): 证券代码字符串，格式应符合市场检测要求
    
    Returns:
        bool: 如果该证券代码符合股票代码格式规则则返回True，否则返回False
    """
    market_id, _, code = detect_market(security_code)
    return assert_stock_by_market_and_code(market_id, code)

def correct_security_code(symbol: str) -> str:
    """
    根据给定的证券代码符号，修正并返回标准化的证券代码。
    
    Args:
        symbol (str): 输入的证券代码符号，可以是任意格式的证券代码表示
    
    Returns:
        str: 修正后的标准化证券代码，格式为"市场标志+证券代码"
        
    Note:
        如果输入为空字符串，将返回空字符串
    """
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
