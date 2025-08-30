#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os

from quant1x import data
from quant1x.formula.formula import *


def turn_percentage(x):
    return '%.2f%%' % (x * 100)


action = '样本数据采样'
df = pd.read_csv(os.path.expanduser(data.quant1x_data + '/' + action + '.csv'))
df1 = df.loc[(df['bl'] >= 1.05) & (df['result'] == True)]
# 覆盖率
coverage = len(df1) * 100 / len(df)
print("涨幅5%%股价落在90%%概率, 在自选股中覆盖率: %.2f%%" % coverage)

df2 = df.loc[(df['bl'] >= 1.05)]
coverage = len(df2) * 100 / len(df)
print("超过5%%涨幅, 在自选股中占比: %.2f%%" % coverage)

df3 = df.loc[(df['result'] == True)]
coverage = len(df3) * 100 / len(df)
print("股价在90%%置信区间, 在自选股中占比: %.2f%%" % coverage)
