# -*- coding: UTF-8 -*-
from typing import List
from quant1x.exchange.security import get_security_info
from quant1x.cache.sector import get_sector_list

# A股指数列表
A_SHARE_INDEX_LIST = [
    "sh000001",  # 上证综合指数
    "sh000002",  # 上证A股指数
    "sh000300",  # 沪深300指数
    "sh000688",  # 科创50指数
    "sh000905",  # 中证500指数
    "sz399001",  # 深证成份指数
    "sz399006",  # 创业板指
    "sz399107",  # 深证A指
    "bj899050",  # 北证50指数
    "sh880005",  # 通达信板块-涨跌家数
    "sh510050",  # 上证50ETF
    "sh510300",  # 沪深300ETF
    "sh510900",  # H股ETF
]

def is_need_ignore(code: str) -> bool:
    """
    证券代码是否需要忽略, 这是一个不参与数据和策略处理的开关
    """
    p = get_security_info(code)
    if not p:
        # 没找到直接忽略
        return True

    # 需要检查的关键字列表
    ignored_keywords = ["ST", "退", "摘牌"]

    # 转换名称为大写
    upper_name = p.name.upper()

    # 检查是否存在任意关键字
    return any(keyword in upper_name for keyword in ignored_keywords)

def get_stock_code_list() -> List[str]:
    """
    获取证券代码列表, 过滤退市、摘牌和ST标记的个股
    """
    all_codes = []
    
    # 上海证券交易所 (sh600000-sh609999)
    for i in range(600000, 610000):
        fc = f"sh{i:06d}"
        if not is_need_ignore(fc):
            all_codes.append(fc)

    # 科创板 (sh688000-sh689999)
    for i in range(688000, 690000):
        fc = f"sh{i:06d}"
        if not is_need_ignore(fc):
            all_codes.append(fc)

    # 深圳主板 (sz000000-sz000999)
    for i in range(0, 1000):
        fc = f"sz{i:06d}"
        if not is_need_ignore(fc):
            all_codes.append(fc)

    # 中小板 (sz001000-sz009999)
    for i in range(1000, 10000):
        fc = f"sz{i:06d}"
        if not is_need_ignore(fc):
            all_codes.append(fc)

    # 创业板 (sz300000-sz300999)
    for i in range(300000, 310000):
        fc = f"sz{i:06d}"
        if not is_need_ignore(fc):
            all_codes.append(fc)

    # 北交所 (bj920000-bj920999)
    for i in range(920000, 921000):
        fc = f"bj{i:06d}"
        if not is_need_ignore(fc):
            all_codes.append(fc)

    return all_codes

def get_code_list() -> List[str]:
    """
    加载全部指数、板块和个股的代码
    """
    code_list = []
    # 1. 指数
    code_list.extend(A_SHARE_INDEX_LIST)
    
    # 2. 板块
    sectors = get_sector_list()
    if not sectors.empty and 'code' in sectors.columns:
        code_list.extend(sectors['code'].astype(str).tolist())

    # 3. 个股, 包括场内开放式ETF基金
    stock_code_list = get_stock_code_list()
    code_list.extend(stock_code_list)
    
    return code_list
