#!/usr/bin/python
# -*- coding: UTF-8 -*-

import pandas as pd

from quant1x.data import D
from quant1x.formula import *

df = D.dataset('002528')
print(df)

e1 = RSI(df['close'],6)
e2 = pd.Series(df['close']).ewm(span=14, adjust=False).mean().values
print(e1)
