#!/usr/bin/python
# -*- coding: UTF-8 -*-

from quant1x.data import D

data = D.finance('600068')
print(data)
start = data.iloc[0, 6]
print(start)
