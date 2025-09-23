import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False

# 跟风指数量化计算
#
# ========== 1. 模拟数据生成 ==========
np.random.seed(42)
n_days = 60
n_stocks = 100
dates = pd.date_range("2024-01-01", periods=n_days)

# 生成个股数据
data = {
    'volume': np.random.lognormal(10, 1, (n_days, n_stocks)),  # 成交量
    'close': np.cumprod(1 + np.random.normal(0.01, 0.03, (n_days, n_stocks)), axis=0) * 100,  # 价格
    '融资买入额': np.random.exponential(1e6, (n_days, n_stocks)),  # 模拟融资数据
    '换手率': np.random.uniform(0.1, 20, (n_days, n_stocks))  # 换手率(%)
}

# 构建DataFrame
volume = pd.DataFrame(data['volume'], index=dates, columns=[f'S{i}' for i in range(n_stocks)])
close = pd.DataFrame(data['close'], index=dates, columns=volume.columns)
margin_buy = pd.DataFrame(data['融资买入额'], index=dates, columns=volume.columns)
turnover = pd.DataFrame(data['换手率'], index=dates, columns=volume.columns)


# ========== 2. 分项指标计算 ==========
class FollowIndexCalculator:
    def __init__(self, window=20):
        self.window = window  # 滚动窗口大小

    def calc_margin_ratio(self, margin_buy, volume):
        """融资买入占比"""
        total_margin = margin_buy.sum(axis=1)
        total_volume = volume.sum(axis=1)
        return (total_margin / total_volume).fillna(0) * 100

    def calc_small_trade_ratio(self, volume):
        """小额交易占比（模拟）"""
        small_trade = volume.apply(lambda x: x * np.random.uniform(0.3, 0.8))  # 假设小额占30-80%
        return (small_trade.sum(axis=1) / volume.sum(axis=1)) * 100

    def calc_hhi(self, volume):
        """成交量HHI指数"""
        total = volume.sum(axis=1)
        total[total == 0] = 1e-6
        market_share = volume.div(total, axis=0)
        return (market_share ** 2).sum(axis=1) * 10000

    def calc_turnover_percentile(self, turnover):
        """换手率分位数指标"""
        q10 = turnover.rolling(self.window).quantile(0.1)
        q90 = turnover.rolling(self.window).quantile(0.9)
        return ((turnover - q10) / (q90 - q10)).mean(axis=1) * 100  # 板块平均

    def composite_index(self):
        """综合跟风指数（返回 Pandas Series）"""
        # 计算各分项指标（确保返回 Series）
        margin_ratio = self.calc_margin_ratio(margin_buy, volume)
        small_ratio = self.calc_small_trade_ratio(volume)
        hhi = self.calc_hhi(volume)
        turnover_q = self.calc_turnover_percentile(turnover)

        # 标准化并限制范围（使用 Pandas 计算 Z-Score）
        def standardize(s):
            return (s - s.mean()) / s.std()

        margin_ratio = standardize(margin_ratio).clip(-3, 3)
        small_ratio = standardize(small_ratio).clip(-3, 3)
        hhi = standardize(hhi).clip(-3, 3)
        turnover_q = standardize(turnover_q).clip(-3, 3)

        # 权重分配
        weights = {'margin': 0.3, 'small': 0.25, 'hhi': 0.25, 'turnover': 0.2}
        composite = (
                margin_ratio * weights['margin'] +
                small_ratio * weights['small'] +
                hhi * weights['hhi'] +
                turnover_q * weights['turnover']
        )

        # 归一化到0-100区间，并返回 Series
        return (composite * 10 + 50).to_frame(name='跟风指数')


# ========== 3. 计算与可视化 ==========
calculator = FollowIndexCalculator(window=20)
follow_index = calculator.composite_index()

plt.figure(figsize=(12, 6))
follow_index.plot(label='跟风指数', color='darkred', lw=1.5)
plt.axhline(70, color='red', linestyle='--', label='高风险阈值')
plt.axhline(30, color='green', linestyle='--', label='低风险阈值')
plt.title('板块跟风指数走势')
plt.ylabel('指数值')
plt.legend()
plt.grid(True)
plt.show()