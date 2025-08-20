#!/usr/bin/python
# -*- coding: UTF-8 -*-
"""
读取 宽表信息
"""

import pandas

from quant1x.data import D

# 列名与数据对其显示
pandas.set_option('display.unicode.ambiguous_as_wide', True)
pandas.set_option('display.unicode.east_asian_width', True)
# 显示所有列
pandas.set_option('display.max_columns', None)

code = "300359"
df = D.stock_hist(symbol=code)
df.to_csv('wb' + code + '.csv', index=False)
