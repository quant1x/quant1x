#!/usr/bin/python
# -*- coding: UTF-8 -*-

"""
财务数据
"""
import os
import time

import akshare as ak
import numpy as np

from quant1x.data import D, quant1x_data_cn


def F10(symbol: str, name: str):
    df = ak.stock_financial_analysis_indicator(symbol=symbol)
    df.replace('--', np.nan, inplace=True)
    df.to_csv(os.path.expanduser(quant1x_data_cn + '/' + symbol + '-f10.csv'), index=False)
    time.sleep(1)


action = '财务数据'
df = D.apply(func_name=action, func=F10)
