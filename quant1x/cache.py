# -*- coding: UTF-8 -*-
# 缓存的数据
import json
import os
from functools import lru_cache
from typing import Dict, Any

import pandas as pd
from dateutil import parser
from pandas import DataFrame

from quant1x import exchange, config
import inflection


@lru_cache(maxsize=None)
def securities() -> pd.DataFrame:
    """
    证券列表
    """
    full_path = os.path.join(config.quant1x_config.meta_path, 'securities.csv')
    if not os.path.isfile(full_path):
        return pd.DataFrame()
    df = pd.read_csv(full_path)
    # 转换为小写
    df.columns = df.columns.str.lower()
    return df[['code', 'name']]


@lru_cache(maxsize=None)
def block_list():
    """
    板块列表
    """
    df = securities()
    return df[df['code'].astype(str).str.startswith(('sh880', 'sh881'))]


def stock_name(code: str) -> str:
    corrected_symbol = exchange.correct_security_code(code)
    df = securities()
    tmp = df[df['code'] == corrected_symbol]
    name = tmp['name'].iloc[0]
    return name


def klines(code: str, freq:str = 'D') -> pd.DataFrame | None:
    """
    获取缓存的日线数据
    """
    corrected_symbol = exchange.correct_security_code(code)
    suffix_length = 3  # 修正拼写并明确表示后缀长度
    freq_path = 'day' if freq == 'D' else freq
    symbol_directory = os.path.join(config.quant1x_config.data_path, freq_path, corrected_symbol[:-suffix_length])  # 更清晰表达目录用途
    file_extension = '.csv'
    filename = f"{corrected_symbol}{file_extension}"  # 使用f-string格式化
    full_path = os.path.join(symbol_directory, filename)

    if os.path.isfile(full_path):
        df = pd.read_csv(full_path)
        # 转换所有列名为 snake_case
        df.columns = [inflection.underscore(col) for col in df.columns]
        return df
    return None


def get_period_name(period: str = 'D') -> str:
    """
    根据周期标识返回中文名称

    Parameters:
    period (str): 周期标识 'W', 'M', 'Q', 'Y'

    Returns:
    str: 中文周期名称
    """
    period_names = {
        'W': '周',
        'M': '月',
        'Q': '季',
        'Y': '年',
        'D': '日'
    }
    period = period.upper()
    return period_names.get(period, period)


def convert_klines_trading(klines, period='D'):
    """
    基于实际交易日的K线转换函数

    Parameters:
    klines (pd.DataFrame): 日线数据
    period (str): 目标周期
        'W' - 周线
        'M' - 月线
        'Q' - 季度线
        'Y' - 年线

    Returns:
    pd.DataFrame: 转换后的K线数据，date字段表示实际交易日
    """
    if klines.empty:
        return klines.copy()

    df = klines.copy()
    df['date'] = pd.to_datetime(df['date'])
    df = df.sort_values('date').reset_index(drop=True)

    # 直接使用简化的周期标识
    period = period.upper()
    if period not in ['W', 'M', 'Q', 'Y']:
        return df

    # 根据周期分组
    groups = df['date'].dt.to_period(period)

    # 聚合数据，date字段保留实际的交易日
    result = df.groupby(groups).agg({
        'date': 'last',  # 实际最后一个交易日
        'open': 'first',
        'high': 'max',
        'low': 'min',
        'close': 'last',
        'volume': 'sum',
        'amount': 'sum'
    }).reset_index(drop=True)

    return result


# SectorFilename 板块缓存文件名
def sector_filename(date: str = '') -> str:
    """
    板块缓存文件名
    """
    name = 'blocks'
    cache_date = date.strip()
    if len(cache_date) == 0:
        cache_date = exchange.last_trade_date()
    filename = os.path.join(config.quant1x_config.meta_path, f'{name}.{cache_date}')
    return filename


@lru_cache(maxsize=None)
def get_sector_list() -> pd.DataFrame:
    """
    获取板块列表
    """
    sfn = sector_filename()
    df = pd.read_csv(sfn)
    df['code'] = 'sh' + df['code'].astype(str)
    return df


def get_sector_constituents(code: str) -> list[str]:
    """
    获取板块成分股列表
    """
    code = code.strip()
    security_code = exchange.correct_security_code(code)
    df = get_sector_list().copy()
    cs = df[df['code'] == security_code]['ConstituentStocks']
    list = []
    if cs.empty:
        return list
    cs1 = cs.iloc[0]
    ConstituentStocks = json.loads(cs1)
    list = []
    for sc in ConstituentStocks:
        sc = sc.strip()
        sc = exchange.correct_security_code(sc)
        list.append(sc)
    return list


def date_format(date: str, layout: str = '%Y-%m-%d') -> str:
    dt = parser.parse(date)  # 自动识别各种常见日期格式
    return dt.strftime(layout)


@lru_cache(maxsize=None)
def get_minutes_data(code: str, date: str) -> DataFrame | None:
    """获取分时数据"""
    code = code.strip()
    corrected_symbol = exchange.correct_security_code(code)
    file_extension = '.csv'
    filename = f"{corrected_symbol}{file_extension}"  # 使用f-string格式化
    cache_date = date.strip()
    if len(cache_date) == 0:
        cache_date = exchange.last_trade_date()
    # 获取年份
    cache_date = date_format(cache_date, layout='%Y%m%d')
    year = cache_date[:4]
    base_path = os.path.join(config.quant1x_config.data_path, 'minutes')
    full_path = os.path.join(base_path, year, cache_date, filename)

    if os.path.isfile(full_path):
        return pd.read_csv(full_path)
    return None


@lru_cache(maxsize=None)
def cache_f10(date: str = None) -> DataFrame:
    factor_name = 'f10'
    trade_date = date or exchange.last_trade_date()
    file_extension = exchange.fix_trade_date(trade_date)
    filename = f"{factor_name}.{file_extension}"
    year = trade_date[:4]
    base_path = os.path.join(config.quant1x_config.data_path, 'flash')
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


@lru_cache(maxsize=None)
def get_tick_transaction(code: str, date: str) -> DataFrame | None:
    """获取分时"""
    code = code.strip()
    corrected_symbol = exchange.correct_security_code(code)
    file_extension = '.csv'
    filename = f"{corrected_symbol}{file_extension}"  # 使用f-string格式化
    cache_date = date.strip()
    if len(cache_date) == 0:
        cache_date = exchange.last_trade_date()
    # 获取年份
    cache_date = date_format(cache_date, layout='%Y%m%d')
    year = cache_date[:4]
    base_path = os.path.join(config.quant1x_config.data_path, 'trans')
    full_path = os.path.join(base_path, year, cache_date, filename)

    if os.path.isfile(full_path):
        df = pd.read_csv(full_path)
        df.columns = [col.lower() for col in df.columns]
        return df
    return None


if __name__ == '__main__':
    print(config.get_quant1x_config_filename())
    print('data_path', config.quant1x_config.data_path)
    print('kline_path', config.quant1x_config.kline_path)
    code = '600600'
    df = klines(code)
    print(df)
    stock_name = stock_name(code)
    print(stock_name)
    security_list = securities()
    print(security_list)
    index_list = block_list()
    print(index_list)
    sfn = sector_filename()
    df = pd.read_csv(sfn)
    print(df)
    print(df['code'].dtype)
    df['code'] = 'sh' + df['code'].astype(str)
    s1 = df[df['code'] == 'sh881478']
    print(s1)

    l1 = get_sector_constituents('880675')
    print(l1)
    print(type(l1))

    df2 = get_minutes_data(code, date='2025-06-20')
    print(df2)
    df3 = get_tick_transaction(code, date='2025-06-20')
    print(df3)
    df4 = get_f10(code)
    print(df4)
