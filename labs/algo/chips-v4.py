import numpy as np
import pandas as pd


def tongdaxin_concentration(df, days=60):
    """模拟通达信筹码集中度计算"""
    df = df[-days:].copy()

    # 生成累积分布
    all_prices = []
    all_weights = []
    for _, row in df.iterrows():
        prices = np.linspace(row['low'], row['high'], 100)
        weights = np.full(100, row['volume'] / 100)
        all_prices.extend(prices)
        all_weights.extend(weights)

    sorted_idx = np.argsort(all_prices)
    cum_weights = np.cumsum(np.array(all_weights)[sorted_idx])
    total = cum_weights[-1]

    # 分位点计算
    def quantile(p):
        if total == 0:
            return np.nan
        target = total * p
        idx = np.searchsorted(cum_weights, target)
        if idx == 0:
            return all_prices[sorted_idx[0]]
        if idx >= len(cum_weights):
            return all_prices[sorted_idx[-1]]

        w1 = cum_weights[idx - 1]
        w2 = cum_weights[idx]
        alpha = (target - w1) / (w2 - w1)
        return all_prices[sorted_idx[idx - 1]] * (1 - alpha) + all_prices[sorted_idx[idx]] * alpha

    q95 = quantile(0.95)
    q05 = quantile(0.05)
    price_max = all_prices[sorted_idx[-1]]
    price_min = all_prices[sorted_idx[0]]

    if (price_max - price_min) == 0:
        return 100.0
    return (q95 - q05) / (price_max - price_min) * 100

# 识别筹码单峰形态
def is_single_peak(df):
    conc = tongdaxin_concentration(df)
    return (conc < 30) and (df['close'].pct_change().std() < 0.02)

# 与主力资金流向协同分析
def smart_money_signal(df):
    conc = tongdaxin_concentration(df)
    money_flow = (df['close'] - df['open']) / df['open'] * df['volume']
    return (conc < 25) and (money_flow.mean() > 1e8)

# code = 'sh300076'
# df = ak.stock_zh_a_daily(code, adjust="qfq")
df = pd.read_csv('test.csv')
print(df)
# 计算双周期集中度差
conc_short = tongdaxin_concentration(df, 20)
conc_long = tongdaxin_concentration(df, 60)
divergence = conc_short - conc_long  # 捕捉筹码集中趋势
print(divergence)