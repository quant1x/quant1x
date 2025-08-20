import akshare as ak
import mplfinance as mpf  # Please install mplfinance as follows: pip install mplfinance
import pandas as pd

df = ak.stock_zh_a_hist(symbol="000002", period="daily", adjust="qfq")
print(df)
df = df[["日期", "开盘", "收盘", "最高", "最低", "成交量", "成交额"]]
df.columns = ['Date', 'Open', 'Close', 'High', 'Low', 'Volume', 'Amount']  # 更正排序
# df = df[['date',"open", "close", "high", "low", "volume","amount"]]
# df.index.name = "date"
# print(df['date'])
# df = df["2020-04-01" : "2020-04-29"]
df['Date'] = pd.to_datetime(df['Date'])
df.set_index('Date', inplace=True)
df = df["2020-04-01": "2020-04-29"]
# df = df[:5]
print(df)
mpf.plot(df, type='candle', mav=(3, 6, 9), volume=True, show_nontrading=False)
# mpf.plot(df)
