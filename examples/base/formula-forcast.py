import numpy as np

from quant1x.formula.formula import FORCAST
import pandas as pd
from quant1x import base

e = [1, 2, 3, 4, 5, 6, 7, 8, 9]

df = pd.DataFrame({'x': e})
# x = df['x'].values
x = df['x']
print(x)
N = 3
t = pd.Series(x).rolling(N)
print(t)
t = t.apply(lambda x: np.polyval(np.polyfit(range(N), x, deg=1), N - 1), raw=True).values
print(t)
N = [3,3,3,3,3,3,3,3,3]
t1 = FORCAST(x, N)
print(t1)

t2 = base.Series(e, name='x').rolling_apply(N, lambda x, N: np.polyval(np.polyfit(range(N), x, deg=1), N - 1), raw=True)
print(t2)
