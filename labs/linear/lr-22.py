import matplotlib.pyplot as plt
import numpy as np

plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False
from matplotlib.gridspec import GridSpec
from base1x import cache, exchange

# 改进版筹码集中度计算
def chips_concentration(close, volume, lookback=60, density_band=0.2):
    if len(close) < lookback:
        return np.nan

    try:
        # 增强型价格区间生成
        price_min = close.min() * 0.95
        price_max = close.max() * 1.05

        # 处理全等价格或极值异常
        if price_min >= price_max or np.isnan(price_min) or np.isnan(price_max):
            avg_price = close.mean()
            price_std = close.std()
            price_min = avg_price - max(price_std, 0.1 * avg_price)
            price_max = avg_price + max(price_std, 0.1 * avg_price)

        price_bins = np.linspace(price_min, price_max, 100)
    except:
        return np.nan

    # 时间衰减权重（保持原逻辑）
    decay = 0.5 ** (1 / (lookback / 4))
    weights = np.array([decay ** i for i in range(lookback)][::-1])

    # 成交量分布矩阵（保持原逻辑）
    volume_dist = np.zeros(100)
    for i in range(lookback):
        bin_idx = np.abs(price_bins - close.iloc[i]).argmin()
        volume_dist[bin_idx] += volume.iloc[i] * weights[i]
    # 成交量分布矩阵
    volume_dist = np.zeros(100)
    for i in range(lookback):
        bin_idx = np.abs(price_bins - close.iloc[i]).argmin()
        volume_dist[bin_idx] += volume.iloc[i] * weights[i]

    # 动态核心价格带（覆盖70%筹码）
    sorted_indices = np.argsort(volume_dist)[::-1]
    cumulative = 0
    core_prices = []
    total_volume = volume_dist.sum()

    for idx in sorted_indices:
        cumulative += volume_dist[idx]
        core_prices.append(price_bins[idx])
        if cumulative >= 0.7 * total_volume:
            break

    # 计算集中度指标
    if len(core_prices) == 0:
        return 0.0
    price_range = max(core_prices) - min(core_prices)
    avg_price = np.dot(price_bins, volume_dist) / total_volume
    concentration = price_range / (density_band * avg_price)

    return np.clip(concentration, 0, 1)


# 生成样本数据
# code = '000701'
# code = '002292'
code = '300251'
# code = '002276'
code = '300940'
code = '300759'
code = '300107'
code = '300456'
#code = '000156'
#code = '601228'
code = '002342'
code = 'sh000001'
# =====================================
# 数据获取与预处理
# =====================================
security_code = exchange.correct_security_code(code)
security_name = cache.stock_name(security_code)
print(f'加载{security_name}({security_code})数据:')
df = cache.klines(security_code)

# 计算筹码集中度（滚动窗口）
lookback = 60
df['Concentration'] = [chips_concentration(df['close'].iloc[i - lookback:i],
                                           df['volume'].iloc[i - lookback:i])
                       if i >= lookback else np.nan
                       for i in range(len(df))]

# 可视化
plt.figure(figsize=(14, 10))
gs = GridSpec(4, 1, height_ratios=[3, 1, 1, 1])

ax1 = plt.subplot(gs[0])
ax1.plot(df['close'], label='Price', color='#1f77b4')
ax1.set_title('Price Trend with Concentration Zones')
ax1.fill_between(df.index, df['close'] * 0.8, df['close'] * 1.2,
                 where=(df['Concentration'] > 0.7),
                 color='red', alpha=0.2, label='High Concentration')
ax1.legend()

ax2 = plt.subplot(gs[1], sharex=ax1)
ax2.bar(df.index, df['volume'], color='#2ca02c', alpha=0.6)
ax2.set_title('Trading Volume')

ax3 = plt.subplot(gs[2], sharex=ax1)
ax3.plot(df['Concentration'], color='#d62728', label='Concentration')
ax3.axhline(0.7, linestyle='--', color='grey', alpha=0.7)
ax3.set_title('Chip Concentration Index')
ax3.set_ylim(0, 1)

ax4 = plt.subplot(gs[3])
ax4.hist(df['close'].iloc[-lookback:], bins=30, orientation='horizontal',
         color='#9467bd', alpha=0.7)
ax4.set_title(f'Recent Price Distribution - {security_name}({security_code})')

plt.tight_layout()
plt.show()

print(df[['date','close','Concentration']][df['Concentration'] > 0.7])