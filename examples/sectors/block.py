import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False
from matplotlib.dates import DateFormatter

# ========== 1. 生成模拟数据 ==========
np.random.seed(42)
n_stocks = 100
n_days = 30
dates = pd.date_range("2024-01-01", periods=n_days, freq="D")

# 生成100只个股价格数据
prices = np.cumprod(1 + np.random.normal(0.01, 0.03, (n_days, n_stocks)), axis=0) * 100
prices += np.linspace(0, 20, n_days).reshape(-1, 1)  # 添加板块趋势

# 模拟第20天部分个股暴涨（跟风事件）
prices[20:, 50:60] *= 1.5  # 第50-60号股票暴涨

# 转换为DataFrame
price_df = pd.DataFrame(prices, index=dates, columns=[f"Stock_{i:03d}" for i in range(n_stocks)])


# ========== 2. 计算分散度指标 ==========
def calculate_dispersion(prices):
    returns = prices.pct_change().dropna()

    # 指标1: 收益率标准差
    std_dev = returns.std(axis=1)

    # 指标2: 分位数差 (Q90-Q10)
    quantile_spread = returns.apply(lambda x: x.quantile(0.9) - x.quantile(0.1), axis=1)

    # 指标3: 涨停股数量（涨幅>9.5%）
    涨停_count = (returns > 0.095).sum(axis=1)

    return pd.DataFrame({
        "Std_Deviation": std_dev,
        "Quantile_Spread": quantile_spread,
        "Limit_Up_Count": 涨停_count
    }, index=returns.index)

dispersion_df = calculate_dispersion(price_df)

# ========== 3. 可视化 ==========
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(16, 12), sharex=True)

# ---- 子图1: 前5只个股走势 + 板块平均 ----
for i in range(5):
    ax1.plot(price_df.index, price_df.iloc[:, i], label=f"Stock_{i:03d}", alpha=0.7)
ax1.plot(price_df.mean(axis=1), label="板块平均", color="black", linewidth=2)
ax1.axvline(x=price_df.index[20], color="red", linestyle="--", label="跟风事件日")
ax1.set_title("个股与板块价格走势")
ax1.legend(loc="upper left")
ax1.grid(True)

# ---- 子图2: 分散度指标 ----
ax2.plot(dispersion_df.index, dispersion_df["Std_Deviation"], label="收益率标准差", color="blue")
ax2.plot(dispersion_df.index, dispersion_df["Quantile_Spread"], label="分位数差(Q90-Q10)", color="orange")
ax2.axvline(x=price_df.index[20], color="red", linestyle="--")
ax2.set_title("收益率分散度指标")
ax2.legend()
ax2.grid(True)

# ---- 子图3: HHI与涨停股数量 ----
# 计算成交量HHI
volume = np.abs(np.random.normal(1e6, 5e5, (n_days, n_stocks)))  # 模拟成交量
hhi = pd.Series(
    [((volume[i] / volume[i].sum()) ** 2).sum() * 10000 for i in range(n_days)],
    index=dates
)

ax3.plot(hhi.index, hhi, label="成交量HHI", color="green")
ax3.bar(dispersion_df.index, dispersion_df["Limit_Up_Count"], label="涨停股数量", alpha=0.3)
ax3.axvline(x=price_df.index[20], color="red", linestyle="--")
ax3.set_title("成交量集中度与涨停股数量")
ax3.legend()
ax3.grid(True)

# ========== 4. 统一设置X轴格式 ==========
date_format = DateFormatter("%m-%d")  # 显示月-日
for ax in [ax1, ax2, ax3]:
    ax.xaxis.set_major_formatter(date_format)
    plt.setp(ax.get_xticklabels(), rotation=45, ha='right')

# 调整子图间距
plt.tight_layout()
plt.subplots_adjust(hspace=0.3)  # 增加子图上下间距
plt.show()