#!/usr/bin/python
# -*- coding: UTF-8 -*-
import numpy as np
import pandas as pd
# 获取通达信获取股票列表

from mootdx.quotes import Quotes

# 标准市场
client = Quotes.factory(market='std', multithread=True, heartbeat=True)
# markets = {"sh":1, "sz": 0, "bj":2}
markets = {"sh": 1, "sz": 0}
df = pd.DataFrame()
for key, val in markets.items():
    df1 = client.stocks(val)
    dl = len(df1)
    mk = pd.Series(np.repeat(key, dl))
    df1.insert(0, 'market', mk)
    df = pd.concat([df, df1], ignore_index=True)

client.close()
df.to_csv('market.csv', index=False)
