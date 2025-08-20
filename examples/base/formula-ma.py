import numpy as np

from quant1x.base.formula import *
from quant1x.formula import CONST
import pandas as pd
from quant1x import base

e = [1, 2, 3, 4, 5, 6, 7, 8, 9]
df = pd.DataFrame({'x': e})
x = df['x']
t1 = V1_MA(x, 2)
print(t1)
t2 = MA(x, 2)
print(t2)