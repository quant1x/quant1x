import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.dates import DateFormatter

plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False
from quant1x import cache, exchange

# ========== 1. 数据准备 ==========
sector_code = '880952'
sector_name = cache.stock_name(sector_code)
symbol_list = cache.get_sector_constituents(sector_code)

price_dfs = []
volume_dfs = []

for symbol in symbol_list:
    df = cache.klines(symbol)
    if df is None:
        continue
    df = df[-89:]  # 取最近34个交易日
    df['date'] = pd.to_datetime(df['date'])

    # 价格数据
    df_price = df[['date', 'close']].copy()
    df_price.rename(columns={'close': symbol}, inplace=True)
    price_dfs.append(df_price)

    # 成交量数据
    df_volume = df[['date', 'volume']].copy()
    df_volume.rename(columns={'volume': symbol}, inplace=True)
    volume_dfs.append(df_volume)


def merge_data(df_list):
    merged = pd.DataFrame()
    for df in df_list:
        merged = pd.merge(merged, df, on='date', how='outer') if not merged.empty else df
    merged.set_index('date', inplace=True)
    merged.sort_index(inplace=True)
    return merged.fillna(0)


price_df = merge_data(price_dfs)
volume_df = merge_data(volume_dfs)

# ===== 数据清洗 =====
# 过滤总成交量为零或价格全零的日期
valid_dates = (volume_df.sum(axis=1) > 0) & (price_df.sum(axis=1) > 0)
price_df = price_df[valid_dates]
volume_df = volume_df[valid_dates]

# 处理价格零值：替换0为NaN后前向填充，最后填充剩余NaN为0
price_df = price_df.replace(0, np.nan).ffill().fillna(0)

# ========== 2. 计算指标 ==========
# --- 收益率分散度 ---
# 计算收益率并过滤无穷大值和NaN
returns = price_df.pct_change().replace([np.inf, -np.inf], np.nan).dropna(how='all', axis=1)
returns = returns.dropna()

dispersion_df = pd.DataFrame({
    'Std_Deviation': returns.std(axis=1),
    'Quantile_Spread': returns.apply(
        lambda x: x.quantile(0.9, interpolation='nearest') - x.quantile(0.1, interpolation='nearest')
        if not x.dropna().empty else 0,
        axis=1
    ),
    'Limit_Up_Count': (returns > 0.095).sum(axis=1)
}, index=returns.index)


# --- HHI指数 ---
def calculate_hhi(df):
    total = df.sum(axis=1)
    total[total <= 0] = 1e-6  # 避免除零错误
    market_shares = df.div(total, axis=0)
    return (market_shares ** 2).sum(axis=1) * 10000


volume_df['HHI'] = calculate_hhi(volume_df.iloc[:, :-4])  # 排除可能的无效列
volume_df['HHI_MA20'] = volume_df['HHI'].rolling(20, min_periods=1).mean()
volume_df['HHI_STD20'] = volume_df['HHI'].rolling(20, min_periods=1).std()
volume_df['Upper_Band'] = volume_df['HHI_MA20'] + 2 * volume_df['HHI_STD20']

# --- 板块成交量风险分析 ---
volume_total = volume_df.iloc[:, :-4].sum(axis=1)
volume_ma20 = volume_total.rolling(20, min_periods=1).mean()
volume_std20 = volume_total.rolling(20, min_periods=1).std().replace(0, 1e-6)  # 避免除零
volume_zscore = (volume_total - volume_ma20) / volume_std20

# ========== 3. 可视化 ==========
fig, axes = plt.subplots(4, 1, figsize=(12, 10))
ax1, ax2, ax3, ax4 = axes

# ---- 子图1: 最后一个交易日涨幅前5个股与板块平均 ----
last_day_returns = returns.iloc[-1]  # 最后一个交易日的涨幅
top_5_symbols = last_day_returns.sort_values(ascending=False).head(5).index

for symbol in top_5_symbols:
    stock_name = cache.stock_name(symbol) if hasattr(cache, 'stock_name') else symbol
    ax1.plot(price_df.index, price_df[symbol], label=f'{symbol}({stock_name})', alpha=0.7)
ax1.plot(price_df.mean(axis=1), label='板块平均', color='black', linewidth=2)
ax1.set_title(f'{sector_name}({sector_code}) - 最后一个交易日涨幅前5个股与板块价格走势')
ax1.legend(loc='upper left')
ax1.grid(True)

# ---- 子图2: 分散度指标 ----
ax2.plot(dispersion_df.index, dispersion_df['Std_Deviation'], label='收益率标准差', color='blue')
ax2.plot(dispersion_df.index, dispersion_df['Quantile_Spread'], label='分位数差(Q90-Q10)', color='orange')
ax2.set_title('收益率分散度指标')
ax2.legend()
ax2.grid(True)

# ---- 子图3: HHI与涨停股数量 ----
ax3.plot(volume_df.index, volume_df['HHI'], label='成交量HHI', color='green')
ax3.bar(dispersion_df.index, dispersion_df['Limit_Up_Count'], label='涨停股数量', alpha=0.3)
ax3.plot(volume_df.index, volume_df['Upper_Band'], label='风险阈值', linestyle='--', color='red')
ax3.set_title('成交量集中度与涨停股数量')
ax3.legend()
ax3.grid(True)

# ---- 子图4: 板块成交量风险分析 ----
ax4.plot(volume_total.index, volume_total, label='板块总成交量', color='purple')
ax4.plot(volume_ma20.index, volume_ma20, label='20日移动平均', linestyle='--', color='orange')
ax4.fill_between(
    volume_total.index,
    volume_ma20 - 2 * volume_std20,
    volume_ma20 + 2 * volume_std20,
    color='gray', alpha=0.2, label='±2σ波动带'
)
outliers = volume_zscore[volume_zscore > 2]
ax4.scatter(outliers.index, volume_total[outliers.index], color='red', s=50, label='异常放量(Z>2)')
ax4.set_title('板块成交量风险分析')
ax4.legend()
ax4.grid(True)

# ========== 4. 统一格式设置 ==========
date_format = DateFormatter("%Y-%m-%d")
for ax in axes:
    ax.xaxis.set_major_formatter(date_format)
    plt.setp(ax.get_xticklabels(), rotation=0, ha='center')

plt.tight_layout()
plt.subplots_adjust(hspace=0.4)
plt.show()