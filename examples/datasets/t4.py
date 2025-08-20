import pandas

from quant1x.data import D

# # A+H股所有代码
# df1 = ak.stock_zh_ah_name()
# print(df1)
# df1.to_csv('ah_name.csv', index=False)
# df2 = ak.stock_zh_a_spot_em()
# print(df2)
# df2.to_csv('a_all.csv', index=False)

# 交易日(Date)
# 股票代码(Symbol)
# 股票名称(Name)
# 当日收盘价(Close)
# 当日最高价(High)
# 当日最低价(Low)
# 平均价(AvgPrice)
# 当日开盘价(Open)
# 昨日收盘价(PrevClose)
# 涨跌额(Change)
# 涨跌幅(PctChg)
# 振幅(Amplitude)
# 换手率(TurnoverRatio)
# 成交量(Volume)
# 成交额(Turnover)
# 总市值(MarketCAP)
# 总股本(SharedOutstanding)
# 流通值(FloatCAP)
# 流通股(ShsFloat)
# 市盈率TTM(PETTM)
# 市盈率静(PEStatic)
# 市净率(PB)
# 委比(BidAskPct)
# 量比(VolumePct)
# 净流入量(NetInflowVolume)
# 净流入额(NetInflowAmount)

# 列名与数据对其显示
pandas.set_option('display.unicode.ambiguous_as_wide', True)
pandas.set_option('display.unicode.east_asian_width', True)
# 显示所有列
pandas.set_option('display.max_columns', None)
code = "600018"
df = D.dataset(code="600018")
print(df)
df.to_csv('t' + code + '.csv', index=False)
