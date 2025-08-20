from quant1x import indicator
from quant1x.data import D

# 加载 深圳个股-跨境通
code = '002640'
data = D.dataset(code)
data.set_index('date', inplace=True)
df = data[:'2023-01-20']
df = indicator.p2fb(code, df)
print(df)
