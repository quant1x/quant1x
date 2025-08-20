#!/usr/bin/python
# -*- coding: UTF-8 -*-

import pandas as pd

from quant1x.data import D

code = '002528'
name = '英飞拓'
# code = '000638'
# name = '万方发展'
# code = '002056'
# name = '横店东磁'
code = '600602'
df = D.dataset(code)
print(df)
# y = sim_data()
y = df['high']
x = df['date']
langth = 89
y = pd.Series(y[-langth:]).reset_index(drop=True)
print('y =', y)
x = pd.Series(x[-langth:]).reset_index(drop=True)
print('x =', x)

from quant1x.findpeaks import findpeaks

# Initialized
fp = findpeaks(method='topology')
# Peak detection
results = fp.fit(y)
print('results =', results['df'])
# Plot
fp.plot(prefix=code)
# Plot
fp.plot_persistence(prefix=code)
df = pd.DataFrame(results['df'])
df.to_csv(code + '.csv', index=False)
