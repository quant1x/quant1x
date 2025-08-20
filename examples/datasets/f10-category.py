from mootdx.quotes import Quotes
# 标准市场
client = Quotes.factory(market='std', multithread=True, heartbeat=True)
df1 = client.F10C("600105")
print(df1)
df2 = client.F10("600105", "公司概况")
print(df2)