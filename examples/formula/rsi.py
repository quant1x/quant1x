#!/usr/bin/python
# -*- coding: UTF-8 -*-

from quant1x import formula
from quant1x.data import D

data = D.dataset('000002')
print(data)
df = data
print(df)

CLOSE = df['close']

rsi1 = formula.RSI(CLOSE, 6)
rsi2 = formula.RSI(CLOSE, 12)
jc = formula.CROSS(rsi1, rsi2)
df['jc'] = jc
print(df)


a = 1
b = 2
c = 0.1
d = formula.IFF(a<b, 2, 1)
print(d)

