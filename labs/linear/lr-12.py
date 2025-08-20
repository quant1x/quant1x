import math
import random
from collections import defaultdict


class Trade:
    def __init__(self, price, volume):
        self.price = price
        self.volume = volume


def generate_trades(n):
    """生成模拟交易数据（正态分布）"""
    random.seed()
    trades = []
    for _ in range(n):
        # 生成正态分布价格（均值50，标准差5）
        price = random.gauss(50, 5)
        # 生成随机成交量（1-1000）
        volume = random.randint(1, 1000)
        trades.append(Trade(price, volume))
    return trades


def calculate_chip_distribution(trades, step):
    """计算筹码分布（按价格区间统计）"""
    distribution = defaultdict(int)
    for trade in trades:
        # 计算价格所属区间下限
        lower = math.floor(trade.price / step) * step
        distribution[lower] += trade.volume
    return distribution


def find_max_volume(distribution):
    """查找最大成交量"""
    return max(distribution.values()) if distribution else 0


def display_chip_chart(distribution, max_volume, step):
    """显示筹码峰图表"""
    print("筹码分布图（价格区间 vs 相对成交量）:")
    print("-" * 60)

    # 获取排序后的价格区间
    sorted_prices = sorted(distribution.keys())

    for price in sorted_prices:
        volume = distribution[price]
        # 计算归一化的星号数量（最大宽度50字符）
        stars = (volume * 50) // max_volume
        if stars == 0 and volume > 0:  # 处理非零小值
            stars = 1

        # 格式化输出
        print(f"{price:5.1f}~{price + step:<5.1f} | {volume:<6} | {'█' * stars}")


if __name__ == "__main__":
    # 生成模拟交易数据
    trades = generate_trades(1000)

    # 设置价格区间步长
    step = 1.0

    # 计算筹码分布
    chip_distribution = calculate_chip_distribution(trades, step)

    # 获取最大成交量用于归一化
    max_volume = find_max_volume(chip_distribution)

    # 显示筹码峰图表
    display_chip_chart(chip_distribution, max_volume, step)
