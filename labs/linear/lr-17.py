import matplotlib.pyplot as plt

plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False
import matplotlib.dates as mdates
import pandas as pd
from base1x import cache, exchange

code = '000701'
#code = '600581'
#code = '600714'
#code = 'sh000001'
#code = '600580'
#code = '600126'
#code = '002332'
#code = '603556'
#code = '600985'
#code = '600583'
#code = '600203'
#code = '002243'
#code = '002741'
code = '600156'
#code = '002292'
code = '300251'
# code = '002815'
# code = '002276'
security_code = exchange.correct_security_code(code)
security_name = cache.stock_name(security_code)
data = cache.klines(security_code)
data = data[-250:]

# ==== 关键修正：提取并转换日期字段 ====
# 假设原始数据中的日期列名为 'datetime'
data['date'] = pd.to_datetime(data['datetime'])  # 替换为实际列名

close = data['close']
#close = data['amount'].rolling(5).sum()/data['volume'].rolling(5).sum()

# 短期
pShort = 5
# 中期
pLong =20
# 计算EMA
data['EMA5'] = close.ewm(span=pShort, adjust=False).mean()
data['EMA20'] = close.ewm(span=pLong, adjust=False).mean()

# 计算发散度
data['EMA_Diff'] = data['EMA5'] - data['EMA20']
data['Z-Score'] = (data['EMA_Diff'] - data['EMA_Diff'].rolling(pLong).mean()) / data['EMA_Diff'].rolling(pLong).std()

# 可视化价格和EMA
plt.figure(figsize=(12, 6))
plt.plot(data['date'], data['close'], label='Price', alpha=0.3)  # 使用日期列
plt.plot(data['date'], data['EMA5'], label='EMA5', color='orange')
plt.plot(data['date'], data['EMA20'], label='EMA20', color='green')
plt.legend()

# 可视化Z-Score
# 创建画布和主Y轴（左侧）
fig, ax1 = plt.subplots(figsize=(14, 6))

# 绘制close价格（左侧Y轴）
ax1.plot(data['date'], data['close'], label='Close Price', color='blue', alpha=0.7, linewidth=1)
ax1.set_xlabel('Date')
ax1.set_ylabel('Price', color='blue')
ax1.tick_params(axis='y', labelcolor='blue')

# 创建次Y轴（右侧）用于Z-Score
ax2 = ax1.twinx()
ax2.plot(data['date'], data['Z-Score'], label='Z-Score Divergence', color='purple', linewidth=1)
ax2.set_ylabel('Z-Score', color='purple')
ax2.tick_params(axis='y', labelcolor='purple')

# 设置标题
plt.title(f'{security_code}xxx')
plt.title(f'Z-Score发散度指标 - {security_name}({security_code})',
          fontsize=14, fontweight='bold', pad=15)
# 设置日期格式
plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d'))
locator = mdates.AutoDateLocator()
data_length = len(data)
if data_length <= 34:
    locator = mdates.DayLocator(interval=1)
plt.gca().xaxis.set_major_locator(locator)
plt.gcf().autofmt_xdate(rotation=60)

# 添加阈值线
thresholds = { -2: {'color': 'green', 'ls': '--', 'lw': 1, 'label': '-2σ'},
              -1: {'color': 'blue', 'ls': '--', 'lw': 1, 'label': '-1σ'},
               0: {'color': 'black', 'ls': '-', 'lw': 1.2, 'label': 'Mean'},
               1: {'color': 'orange', 'ls': '--', 'lw': 1, 'label': '+1σ'},
               2: {'color': 'red', 'ls': '--', 'lw': 1, 'label': '+2σ'}}
for value, style in thresholds.items():
    plt.axhline(value, **style)

# 标注 -2 的点
times = 0
for idx,row in data.iterrows():
    score = row['Z-Score']
    date = row['date']
    if row['Z-Score'] < -1.618:
        prev = data['Z-Score'][idx - 1]
        curr = data['Z-Score'][idx]
        next = data['Z-Score'][idx + 1]
        condition = prev < -2 and next>-2 and prev < curr and curr<next
        condition = True
        if condition:
            # 在数据点上方添加星号（调整 y 坐标避免重叠）
            plt.text(
                x=row['date'],  # x 坐标为日期索引
                y=row['Z-Score'] - 0.5*times,  # y 坐标为 close 值 + 偏移量
                s=row['date'].strftime('%Y-%m-%d'),  # 标注文本
                rotation=45,
                ha='center',
                va='bottom',  # 水平/垂直对齐
                color='red',
                fontsize=13,
                fontweight='bold'
            )
            times= times + 1
    else:
        times = 0

plt.legend()
plt.show()
print(data)