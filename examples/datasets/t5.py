import pandas

from quant1x.data import D

# 列名与数据对其显示
pandas.set_option('display.unicode.ambiguous_as_wide', True)
pandas.set_option('display.unicode.east_asian_width', True)
# 显示所有列
pandas.set_option('display.max_columns', None)

df = D.stock_hist(symbol="603126")
# # 处理流通股本
# if len(df) > 0:
#     dr = None
#     for idx, row in df.iterrows():
#         # 遍历除权信息
#         dt = row['交易日(Date)']
#         try:
#             info = xdxr.loc[dt]
#             if not dr is None:
#                 dr = info
#                 continue
#             dr = info
#         except KeyError:
#             pass
#         row['流通股(ShsFloat)'] = dr['panhouliutong'] * 10000
#         row['流通值(FloatCAP)'] = row['当日收盘价(Close)'] * row['流通股(ShsFloat)']
#         row['总股本(SharedOutstanding)'] = dr['houzongguben'] * 10000
#         row['总市值(MarketCAP)'] = row['当日收盘价(Close)'] * row['总股本(SharedOutstanding)']
#         df.iloc[idx] = row
# print(df)
df.to_csv('t603126.csv', index=False)
