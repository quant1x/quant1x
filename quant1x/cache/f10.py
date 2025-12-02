# -*- coding: UTF-8 -*-
import os
from functools import lru_cache
from typing import Dict, Any
import pandas as pd
from pandas import DataFrame
from .. import exchange, config

@lru_cache(maxsize=None)
def cache_f10(date: str = None) -> DataFrame:
    """
    从闪存数据中读取F10因子数据
    
    Args:
        date (str, optional): 交易日期，格式为'YYYYMMDD'。如果未提供，则使用交易所最后一个交易日
    
    Returns:
        DataFrame: 包含F10因子数据的Pandas DataFrame
    
    Raises:
        FileNotFoundError: 如果指定日期的数据文件不存在
    """
    factor_name = 'f10'
    trade_date = date or exchange.last_trade_date()
    file_extension = exchange.fix_trade_date(trade_date)
    filename = f"{factor_name}.{file_extension}"
    year = trade_date[:4]
    base_path = os.path.join(config.data_path, 'flash')
    return pd.read_csv(os.path.join(base_path, year, filename))

def get_f10(code: str, date: str = None) -> Dict[str, Any]:
    """
    获取f10数据
    Args:
        code: 证券代码
        date: 日期

    Returns:
        返回f10数据
    """
    security_code = exchange.correct_security_code(code)
    df = cache_f10(date)
    result_df = df[df['Code'] == security_code]
    if result_df.empty:
        return {}

    # 取第一行，转为字典
    return result_df.iloc[0].to_dict()
