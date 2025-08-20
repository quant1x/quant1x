import akshare as ak
import numpy as np


def calculate_concentration(code, days=250, num_bins=200, precision=0.01):
    """
    优化后的筹码集中度计算函数
    :param code: 股票代码
    :param days: 计算天数
    :param num_bins: 分箱数量
    :param precision: 价格精度（元）
    :return: 90%筹码集中度（百分比）
    """
    # 获取历史数据
    df = ak.stock_zh_a_daily(code, adjust="qfq").iloc[-days:]

    if df.empty:
        return 0.0

    # 计算全局价格范围（保留3%边界缓冲）
    price_min = df['low'].min() * 0.97
    price_max = df['high'].max() * 1.03

    # 生成价格分箱（考虑精度要求）
    bin_edges = np.round(np.linspace(price_min, price_max, num_bins + 1), 2)
    hist = np.zeros(num_bins)

    # 权重累计
    for _, row in df.iterrows():
        low, high, vol = row[['low', 'high', 'volume']]
        low = max(low, price_min)
        high = min(high, price_max)

        if low >= high:
            idx = np.searchsorted(bin_edges, low, side='right') - 1
            if 0 <= idx < num_bins:
                hist[idx] += vol
            continue

        # 计算有效分箱范围
        start_idx = np.searchsorted(bin_edges, low, side='right') - 1
        end_idx = np.searchsorted(bin_edges, high, side='right')

        for i in range(max(0, start_idx), min(end_idx, num_bins)):
            bin_low = bin_edges[i]
            bin_high = bin_edges[i + 1]

            # 计算重叠区间
            overlap_low = max(low, bin_low)
            overlap_high = min(high, bin_high)
            overlap = overlap_high - overlap_low

            if overlap > 0:
                ratio = overlap / (high - low)
                hist[i] += vol * ratio

    # 处理零值情况
    total = hist.sum()
    if total == 0:
        return 100.0

    # 计算累积分布
    cum = np.cumsum(hist) / total

    # 寻找分位点（使用线性插值提高精度）
    def find_quantile(target):
        if cum[-1] < target:
            return len(bin_edges) - 1
        idx = np.searchsorted(cum, target)
        if idx == 0:
            return bin_edges[0]
        # 线性插值
        weight = (target - cum[idx - 1]) / (cum[idx] - cum[idx - 1])
        return bin_edges[idx - 1] + weight * (bin_edges[idx] - bin_edges[idx - 1])

    q5 = find_quantile(0.05)
    q95 = find_quantile(0.95)

    # 计算有效价格范围（去除空值区域）
    non_zero = np.where(hist > 0)[0]
    if len(non_zero) == 0:
        return 100.0
    effective_min = bin_edges[non_zero[0]]
    effective_max = bin_edges[non_zero[-1] + 1]
    effective_range = effective_max - effective_min

    if effective_range == 0:
        return 100.0

    return ((q95 - q5) / effective_range) * 100


# 示例使用
if __name__ == "__main__":
    codes = ['sh601020', 'sh600633', 'sz300750']
    for code in codes:
        try:
            conc = calculate_concentration(code)
            print(f"{code} 90%筹码集中度：{conc:.2f}%")
        except Exception as e:
            print(f"{code} 计算失败：{str(e)}")