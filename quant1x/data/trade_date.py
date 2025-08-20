#!/usr/bin/python
# -*- coding: UTF-8 -*-
from datetime import datetime

import pandas as pd
from dateutil.parser import *

import quant1x.data

# 交易日历
__calendar_filename = quant1x.data.quant1x_info + '/calendar.csv'
__calendar_df = pd.read_csv(__calendar_filename)

# 日期格式
__fmt_date = '%Y-%m-%d'


def today() -> str:
    """
    当前日期
    :return:
    """
    return datetime.today().strftime(__fmt_date)


def correct_date(time: str) -> str:
    """
    矫正日期, 统一格式
    :param time:
    :return:
    """
    t = parse(time)
    return t.strftime(__fmt_date)


def trade_range(start: str, end: str) -> list:
    """
    返回一个交易日期范围
    :param start 开始日期
    :param end 结束日期
    :return:
    """
    start = correct_date(start)
    end = correct_date(end)
    print(start, end)
    dates = __calendar_df['trade_date'].array
    trades = []
    for date in dates:
        if start <= date <= end:
            trades.append(date)
    return trades


dates = trade_range('2023-01-01', '2023-03-23')
print(dates)
