#!/usr/bin/python
# -*- coding: UTF-8 -*-

from labs import indicator
from quant1x.data import D

# 加载 上海个股-香飘飘
data = D.dataset('600520')
df = indicator.f89k(data)
print(df)