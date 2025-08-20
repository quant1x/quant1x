import akshare as ak
import numpy as np


def calculate_concentration(code, days=250):
    df = ak.stock_zh_a_daily(code, adjust="qfq")
    df = df[-days:]

    prices = df['close'].values
    volumes = df['volume'].values

    # 计算筹码分布直方图
    hist, bin_edges = np.histogram(prices, bins=100, weights=volumes, density=False)

    # 计算累积分布
    cum_hist = np.cumsum(hist) / np.sum(hist)

    # 安全获取阈值索引
    idx_start = np.argmax(cum_hist >= 0.05)
    idx_end = np.argmax(cum_hist >= 0.95)

    # 处理无满足条件的索引（如所有数据在左侧）
    if cum_hist[idx_start] < 0.05:
        idx_start = 0
    if cum_hist[idx_end] < 0.95:
        idx_end = len(bin_edges) - 2  # 使用最后一个区间

    # 计算集中度
    price_start = bin_edges[idx_start]
    price_end = bin_edges[idx_end]
    total_range = bin_edges[-1] - bin_edges[0]

    return ((price_end - price_start) / total_range) * 100 if total_range != 0 else 100.0


# 示例：计算贵州茅台筹码集中度
code = 'sh601020'
code = 'sh600633'
concentration = calculate_concentration(code)
print(f"90%筹码集中度：{concentration:.2f}%")
# a1 = ak.stock_zh_a_daily(symbol="sh600580", adjust="qfq")
# print(a1)