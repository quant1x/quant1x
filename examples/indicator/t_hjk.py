#!/usr/bin/python
# -*- coding: UTF-8 -*-

from labs import indicator
from quant1x.data import D

# 加载 个股
symbol = '000151'
data = D.dataset(symbol)
df = indicator.hjk(symbol, data)
print(df)
# todo:2022-12-22这天应该有的信号,没出现,可能是序列化版本存在bug
