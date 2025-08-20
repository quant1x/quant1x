#!/usr/bin/python
# -*- coding: UTF-8 -*-

from quant1x.data import D

D.update_history()
df = D.dataset('603126')
print(df)
