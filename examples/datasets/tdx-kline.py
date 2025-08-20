from mootdx.quotes import Quotes
# 标准市场
client = Quotes.factory(market='std', multithread=True, heartbeat=True)
# k 线数据
df1=client.bars(symbol='000966', frequency=9, offset=10)
print(df1)