import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.dates import DateFormatter

plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False
from quant1x import cache, exchange

# ========== 1. 数据准备 ==========
sector_code = '881319'
sector_name = cache.stock_name(sector_code)
symbol_list = cache.get_sector_constituents(sector_code)

# 存储价格和成交量数据
price_dfs = []
volume_dfs = []

for symbol in symbol_list:
    df = cache.klines(symbol)
    if df is None:
        continue
    df = df[-55:]  # 取最近55个交易日
    df['date'] = pd.to_datetime(df['date'])

    # 价格数据
    df_price = df[['date', 'close']].copy()
    df_price.rename(columns={'close': symbol}, inplace=True)
    price_dfs.append(df_price)

    # 成交量数据
    df_volume = df[['date', 'volume']].copy()
    df_volume.rename(columns={'volume': symbol}, inplace=True)
    volume_dfs.append(df_volume)


# 合并数据
def merge_data(df_list):
    merged = pd.DataFrame()
    for df in df_list:
        merged = pd.merge(merged, df, on='date', how='outer') if not merged.empty else df
    merged.set_index('date', inplace=True)
    merged.sort_index(inplace=True)
    return merged.fillna(0)


price_df = merge_data(price_dfs)
volume_df = merge_data(volume_dfs)

# ========== 2. 计算指标 ==========
# --- 指标1: 收益率分散度 ---
returns = price_df.pct_change().dropna()
dispersion_df = pd.DataFrame({
    'Std_Deviation': returns.std(axis=1),
    'Quantile_Spread': returns.apply(lambda x: x.quantile(0.9) - x.quantile(0.1), axis=1),
    'Limit_Up_Count': (returns > 0.095).sum(axis=1)  # 涨停股数量
}, index=returns.index)


# --- 指标2: HHI指数 ---
def calculate_hhi(df):
    total = df.sum(axis=1)
    return df.div(total, axis=0).pow(2).sum(axis=1) * 10000


volume_df['HHI'] = calculate_hhi(volume_df)
volume_df['HHI_MA20'] = volume_df['HHI'].rolling(20).mean()
volume_df['HHI_STD20'] = volume_df['HHI'].rolling(20).std()
volume_df['Upper_Band'] = volume_df['HHI_MA20'] + 2 * volume_df['HHI_STD20']

# --- 指标3: 板块成交量风险分析 ---
# 计算板块总成交量
volume_total = volume_df.iloc[:, :-4].sum(axis=1)  # 排除HHI相关列
volume_ma20 = volume_total.rolling(20).mean()
volume_std20 = volume_total.rolling(20).std()
volume_zscore = (volume_total - volume_ma20) / volume_std20  # Z-Score

# ========== 3. 可视化 ==========
fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(16, 9), sharex=True)

# ---- 子图1: 前5只个股价格走势 ----
top_5_symbols = volume_df.iloc[:, :-4].mean().sort_values(ascending=False).index[:5]
for symbol in top_5_symbols:
    ax1.plot(price_df.index, price_df[symbol], label=f'{symbol}({cache.stock_name(symbol)})', alpha=0.7)
ax1.plot(price_df.mean(axis=1), label='板块平均', color='black', linewidth=2)
ax1.set_title(f'{sector_name}({sector_code}) - 个股与板块价格走势')
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
# 标记Z-Score > 2的异常放量日
outliers = volume_zscore[volume_zscore > 2]
ax4.scatter(outliers.index, volume_total[outliers.index], color='red', s=50, label='异常放量(Z>2)')
ax4.set_title('板块成交量风险分析')
ax4.legend()
ax4.grid(True)

# ========== 4. 统一格式设置 ==========
date_format = DateFormatter("%m-%d")
for ax in [ax1, ax2, ax3, ax4]:
    ax.xaxis.set_major_formatter(date_format)
    plt.setp(ax.get_xticklabels(), rotation=45, ha='right')

plt.tight_layout()
plt.subplots_adjust(hspace=0.3, wspace=0.2)  # 调整子图间距
plt.show()