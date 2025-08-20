#!/usr/bin/python
# -*- coding: UTF-8 -*-
import pandas as pd

# df=pd.DataFrame({'x':[1,2,3,4,5,6,7,8,9]})
# 逻辑表达式测试
df = pd.DataFrame({'a': [True, True, True, False, False, False, False],
                   'b': [True, True, False, False, True, False, True],
                   'c': [True, False, False, False, False, True, True]
                   })

df['a & b & c'] = df.eval('a & b & c')

print(df)
