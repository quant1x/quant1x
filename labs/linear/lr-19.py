import matplotlib.dates as mdates
import numpy as np
import pandas as pd
from base1x import cache, exchange
from matplotlib.gridspec import GridSpec

from quant1x.chart import pyplot as plt

# =====================================
# 数据获取与预处理
# =====================================
code = '600156'
security_code = exchange.correct_security_code(code)
security_name = cache.stock_name(security_code)
data = cache.klines(security_code)
df = data[['close', 'volume']].dropna()

# 确保索引是DatetimeIndex
df.index = pd.to_datetime(df.index)

# 计算历史波动率指标
def calculate_volatility(series, window=30):
    log_ret = np.log(series).diff()
    sma_vol = log_ret.rolling(window).std() * np.sqrt(252)  # 年化波动率
    ewma_vol = log_ret.ewm(span=window).std() * np.sqrt(252)
    return sma_vol, ewma_vol

sma_vol, ewma_vol = calculate_volatility(df['close'])

# 生成模拟市场情绪数据（示例）
np.random.seed(42)
df['sentiment'] = np.random.normal(50, 15, len(df)).clip(0,100)
df['volume_z'] = (df['volume'] - df['volume'].rolling(30).mean()) / df['volume'].rolling(30).std()
df['fund_flow'] = np.random.choice([-1, 0, 1], len(df), p=[0.2,0.6,0.2]).cumsum()

# 创建可视化画布
plt.figure(figsize=(16, 12), dpi=100)
gs = GridSpec(4, 1, height_ratios=[3, 2, 2, 2])

# 子图1：价格与波动率
ax1 = plt.subplot(gs[0])
ax1.plot(df['close'], label='Close Price', color='#1f77b4')
ax1.set_title(f'{security_name} Price & Volatility', fontsize=12)
ax1.legend(loc='upper left')

ax1v = ax1.twinx()
ax1v.plot(sma_vol, label='30D Vol', color='#ff7f0e', alpha=0.7)
ax1v.plot(ewma_vol, label='EWMA Vol', color='#2ca02c', linestyle='--')
ax1v.axhline(y=60, color='r', linestyle=':', linewidth=1)
ax1v.legend(loc='upper right')

# 子图2：动态波动率增强模型（修正语法错误）
ax2 = plt.subplot(gs[1])
vol_threshold = ewma_vol.quantile(0.8)
vol_regime = pd.Series(np.where(ewma_vol > vol_threshold, 1, 0), index=df.index)
vol_signal = vol_regime.rolling(5).mean()

colors = np.where(vol_signal > 0.5, 'red', 'green')
ax2.bar(df.index, height=1, width=1, color=colors, alpha=0.3)
ax2.plot(ewma_vol, color='#17becf', label='Dynamic Volatility')
ax2.set_title('Volatility Regime Detection (80th percentile threshold)')
ax2.legend()

# 子图3：市场情绪指标
ax3 = plt.subplot(gs[2])
ax3.plot(df['sentiment'], color='#9467bd', label='Sentiment Score')
ax3.fill_between(df.index, 80, 100, color='red', alpha=0.1)
ax3.fill_between(df.index, 0, 20, color='green', alpha=0.1)
ax3.set_ylim(0, 100)
ax3.set_title('Synthetic Market Sentiment')

# 子图4：成交量异常检测（修正列名）
ax4 = plt.subplot(gs[3])
ax4.bar(df.index, df['volume'], color=np.where(df['volume_z']>3, 'orange', '#7f7f7f'))
ax4.set_title('Volume Anomaly Detection (Z-Score >3)')
ax4.set_yscale('log')

# 标记预警信号（修正列名引用）
warning_condition = (ewma_vol > 60) & (df['sentiment'] > 80)
warning_dates = df[warning_condition].index

for date in warning_dates:
    ax1.annotate('⚠',
                (mdates.date2num(date), df['close'].loc[date]),
                textcoords="offset points",
                xytext=(0,10),
                ha='center',
                color='red',
                fontsize=14,
                arrowprops=dict(arrowstyle="->", color='red'))

plt.tight_layout()
plt.show()