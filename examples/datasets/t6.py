import pandas

# 列名与数据对其显示
pandas.set_option('display.unicode.ambiguous_as_wide', True)
pandas.set_option('display.unicode.east_asian_width', True)
# 显示所有列
pandas.set_option('display.max_columns', None)

from mootdx.quotes import Quotes

code = "600018"
# 标准市场
client = Quotes.factory(market='std', multithread=True, heartbeat=True)
data = client.xdxr(symbol=code)
data.to_csv(code + '-xdxr.csv', index=False)

# xdxr = D.xdxr('600600')
# xdxr['date'] = pd.to_datetime(xdxr['date'])
# xdxr.set_index('date', inplace=True, drop=True)
# print(xdxr)
# print(df[:'1994-06-27'])
# s = None
# try:
#     s1 = xdxr.loc['1994-06-27']
#     s = s1
# except KeyError:
#     pass
# print(s)
