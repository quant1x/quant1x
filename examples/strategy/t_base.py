#!/usr/bin/python
# -*- coding: UTF-8 -*-
import os

import pandas

from quant1x import data
from quant1x.strategy import linear_regression as base2
from quant1x.strategy import bad_base as base1

d, x, y, z = base1.predict(symbol="002528", n=1)
print(d, x, y, z)

# from quant1x.data import D
#
#
# def cb(symbol: str, name: str):
#     d, x, y, z = base.predict(symbol=symbol, n=1)
#     if pandas.isna(x) or pandas.isna(y) or pandas.isna(x):
#         return None
#     return pandas.Series({'date': d, 'code': symbol, 'name': name, 'close': x, 'predict': y, 'accuracy': z * 100})
#
#
# df = D.apply(func_name='预测下一个交易日收盘价', func=cb)
#
# df.to_csv(os.path.expanduser(data.quant1x_data + '/线性回归测试结果.csv'), index=False, encoding="UTF-8", quoting=1)
# print(df)
