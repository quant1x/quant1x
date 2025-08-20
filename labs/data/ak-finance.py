#!/usr/bin/python
# -*- coding: UTF-8 -*-

"""
财务数据
"""

import akshare as ak
import numpy as np

code = "600600"
data = ak.stock_financial_analysis_indicator(symbol=code)
print(data)
df = data
df.replace('--', np.nan, inplace=True)
df.to_csv(code + '-f10.csv', index=False)
