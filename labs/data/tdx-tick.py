import pandas
from mootdx.quotes import Quotes

# 标准市场
client = Quotes.factory(market='std', multithread=True, heartbeat=True)
# df = client.transactions(symbol='600600', date=20200101)

code = "002528"
offset = 1800
start = 0
df = pandas.DataFrame()
while True:
    # 历史数据
    data = client.transactions(symbol=code, date=20230303, start=start, offset=1800)
    # 当前数据
    data = client.transaction(symbol=code, start=start, offset=1800)
    if len(data) > 0:
        df = pandas.concat([data, df], ignore_index=True)
        start += offset
    if len(data) < offset:
        break

print(df)
df.to_csv(code + '-tick.csv', index=False)
