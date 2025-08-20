#!/usr/bin/python
# -*- coding: UTF-8 -*-

from quant1x import indicator
from quant1x.data import D

# 显示全部列
#pd.options.display.max_columns = None
#pd.set_option('display.max_columns', None)
#pd.set_option('max_colwidth',200)

# 加载 深圳个股-万科A
data = D.dataset('000002')
df = indicator.ma1x(data)
print(df)






