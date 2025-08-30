from quant1x.formula.formula import *
import pandas as pd

e = [1, 2, 3, 4, 5, 6, 7, 8, 9]
df = pd.DataFrame({'x': e})
x = df['x']
t1 = V1_MA(x, 2)
print(t1)
t2 = MA(x, 2)
print(t2)
