import akshare as ak
import numpy as np


def calculate_concentration(code, days=250, num_bins=200, precision=0.01,
                            lower_quantile=0.05, upper_quantile=0.95,
                            return_interval=False):
    """
    增强版筹码集中度计算函数
    :param code: 股票代码
    :param days: 计算天数
    :param num_bins: 价格分箱数量
    :param precision: 价格精度(元)
    :param lower_quantile: 下分位数(默认5%)
    :param upper_quantile: 上分位数(默认95%)
    :param return_interval: 是否返回价格区间
    :return: 集中度百分比 或 价格区间元组
    """
    # 数据获取与校验
    df = ak.stock_zh_a_daily(code, adjust="qfq").iloc[-days:]
    if df.empty or len(df) < days // 2:  # 至少需要50%数据
        return (0.0, 0.0) if return_interval else 0.0

    # 动态价格范围计算（保留5%缓冲）
    price_min = max(df['low'].min() * 0.95, df['close'].min() * 0.97)
    price_max = min(df['high'].max() * 1.05, df['close'].max() * 1.03)

    # 生成价格分箱（确保精度）
    bin_edges = np.round(np.linspace(price_min, price_max, num_bins + 1),
                         decimals=abs(int(np.log10(precision))))
    hist = np.zeros(num_bins)

    # 精确成交量分布计算
    for _, row in df.iterrows():
        low, high, vol = row[['low', 'high', 'volume']]
        low = max(low, price_min)
        high = min(high, price_max)

        # 单日价格波动处理
        if low >= high:
            idx = np.searchsorted(bin_edges, low, side='right') - 1
            if 0 <= idx < num_bins:
                hist[idx] += vol
            continue

        # 分箱定位与权重计算
        start_idx = np.clip(np.searchsorted(bin_edges, low, side='right') - 1, 0, num_bins)
        end_idx = np.clip(np.searchsorted(bin_edges, high, side='right'), 0, num_bins)

        # 向量化计算重叠比例
        bin_lows = bin_edges[start_idx:end_idx]
        bin_highs = bin_edges[start_idx + 1:end_idx + 1]
        overlaps = np.minimum(bin_highs, high) - np.maximum(bin_lows, low)
        ratios = overlaps / (high - low)
        hist[start_idx:end_idx] += vol * ratios

    # 有效价格区间计算
    valid_bins = np.where(hist > 0)[0]
    if len(valid_bins) == 0:
        return (0.0, 0.0) if return_interval else 0.0

    # 累积分布计算
    cum_hist = np.cumsum(hist) / hist.sum()

    # 分位数定位函数（带边界保护）
    def find_quantile(target):
        target = np.clip(target, 0.0, 1.0)
        if cum_hist[-1] < target:
            return bin_edges[-1]
        idx = np.searchsorted(cum_hist, target)
        if idx == 0:
            return bin_edges[0]
        # 线性插值
        weight = (target - cum_hist[idx - 1]) / (cum_hist[idx] - cum_hist[idx - 1] + 1e-9)
        return bin_edges[idx - 1] + weight * (bin_edges[idx] - bin_edges[idx - 1])

    # 计算目标分位数
    q_lower = find_quantile(lower_quantile)
    q_upper = find_quantile(upper_quantile)

    # 返回价格区间或集中度
    if return_interval:
        return (round(q_lower, 2), round(q_upper, 2))
    else:
        effective_min = bin_edges[valid_bins[0]]
        effective_max = bin_edges[valid_bins[-1] + 1]
        effective_range = effective_max - effective_min
        if effective_range == 0:
            return 100.0
        return ((q_upper - q_lower) / effective_range) * 100


def get_10_percent_interval(code, days=250):
    """获取最密集10%筹码的价格区间"""
    return calculate_concentration(code, days=days,
                                   lower_quantile=0.45,
                                   upper_quantile=0.55,
                                   return_interval=True)


# 使用示例
if __name__ == "__main__":
    stock_codes = ['sh601318', 'sz000858', 'sh600519','sz300076']

    for code in stock_codes:
        try:
            # 计算常规90%集中度
            conc_90 = calculate_concentration(code)
            # 获取最密集10%区间
            low, high = get_10_percent_interval(code)
            price_range = high - low

            print(f"{code} 分析结果：")
            print(f"90%筹码集中度：{conc_90:.1f}%")
            print(f"最密集10%价格区间：{low:.2f}-{high:.2f} (跨度：{price_range:.2f}元)")
            print("━" * 40)

        except Exception as e:
            print(f"{code} 分析失败：{str(e)}")