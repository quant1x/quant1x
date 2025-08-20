from labs import indicator
from quant1x.data import D


data = D.dataset('601318')
data.set_index('date', inplace=True)
# df = data[:'2023-01-04']
# df = data[:'2022-10-11']
df = data
df = indicator.cdtd(df)
print(df)
