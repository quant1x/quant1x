import matplotlib.pyplot as plt

plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False
import matplotlib.dates as mdates
import pandas as pd
from base1x import cache, exchange

#code = '000701'
#code = '002292'
code = '300251'
#code = '002276'
code = '300940'
code = '300759'
code = '300107'
code = '300456'
# code = '000156'
# code = '601228'
# code = '000521'
# code = '002342'
code = 'sh000001'
# =====================================
# 数据获取与预处理
# =====================================
security_code = exchange.correct_security_code(code)
security_name = cache.stock_name(security_code)
data = cache.klines(security_code)
data = data[-55:]

# ==== 关键修正：提取并转换日期字段 ====
# 假设原始数据中的日期列名为 'datetime'
data['date'] = pd.to_datetime(data['datetime'])  # 替换为实际列名

target = data['close']
#target = data['amount'].rolling(5).sum()/data['volume'].rolling(5).sum()
volume_weighted_data = data['volume']#.ewm(span=5, adjust=False).mean()
print(volume_weighted_data)
# DIF:EMA(CLOSE,SHORT)-EMA(CLOSE,LONG);
# DEA:EMA(DIF,MID);
# MACD:(DIF-DEA)*2,COLORSTICK;

# # 短期
# pShort = 5
# # 长期
# pLong =20
# # 标准差周期
# pStdDev = 9

# 计算EMA: 5,20
pShort = 5
pLong = 20

# =====================================
# 成交量加权
# =====================================
volume_weighted_period = pShort
volume_weighted_sum = volume_weighted_data.rolling(window=volume_weighted_period).sum()
print(volume_weighted_sum)
target = (target * volume_weighted_data).rolling(window=volume_weighted_period).sum()/volume_weighted_data.rolling(window=volume_weighted_period).sum()
#target = (target*volume_weighted_data).rolling(window=volume_weighted_period).sum()/volume_weighted_sum

data_short = target.ewm(span=pShort, adjust=False).mean()
data_long = target.ewm(span=pLong, adjust=False).mean()


# # 计算macd的diff和dea
# pShort = 12
# pLong =26
# data_short = close.ewm(span=pShort, adjust=False).mean()
# data_long = close.ewm(span=pLong, adjust=False).mean()
#
# data['short'] = data_short-data_long
# data['long'] = data['short'].ewm(span=pStdDev, adjust=False).mean()


# =====================================
# 计算标准化发散度Z-Score
# =====================================
# 确定短期和长期的数据
data['short'] = data_short
data['long'] = data_long
# 计算差值
data['diff'] = data_short - data_long
# 标准化到均值
pZscore = pLong
diff_mean = data['diff'].rolling(window=pZscore).mean()
# 标准差
diff_std = data['diff'].rolling(window=pZscore).std()
# 标准化发散度(Z-Score)
data['Z-Score'] = (data['diff'] - diff_mean) / diff_std

# 可视化价格和EMA
plt.figure(figsize=(12, 6))
plt.plot(data['date'], data['close'], label='Price', alpha=0.3)  # 使用日期列
plt.plot(data['date'], data['short'], label='EMA_short', color='orange')
plt.plot(data['date'], data['long'], label='EMA_long', color='green')
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
    thresholds = -0.618
    if row['Z-Score'] < thresholds:
        # prev = data['Z-Score'][idx - 1]
        # curr = data['Z-Score'][idx]
        # next = data['Z-Score'][idx + 1]
        # condition = prev <thresholds and next>thresholds and prev < curr and curr<next
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
print(data[['date','short','long','diff','Z-Score']])