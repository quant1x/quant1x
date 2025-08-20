import numpy as np
import pandas as pd
import seaborn as sns
from astropy.stats import bayesian_blocks
from base1x import cache, exchange

from quant1x.chart import pyplot as plt

# =====================================
# 数据获取与预处理
# =====================================
code = '600156'
security_code = exchange.correct_security_code(code)
security_name = cache.stock_name(security_code)
data = cache.klines(security_code)
df = data[-500:]  # 扩大数据量至500个交易日

# 数据验证
print("数据摘要：")
print(f"股票代码：{security_code}({security_name})")
# print(f"数据周期：{df.index.min()} 至 {df.index.max()}")
# print("\n价格统计：")
# print(df['close'].describe())
# print("\n成交量统计：")
# print(df['volume'].describe())

# 数据清洗
df = df[df['close'] > 0]  # 过滤异常价格
df = df[df['volume'] > 0]  # 过滤异常成交量

prices = df['close']
volumes = df['volume']


# =====================================
# 1. 动态价格分箱（增强参数配置）
# =====================================
def volume_weighted_binning(prices, volumes, p0=0.02, min_bins=10):
    valid_mask = prices.notna() & volumes.notna()
    prices_clean = prices[valid_mask]
    volumes_clean = volumes[valid_mask]

    # 检查数据波动性
    if prices_clean.std() < 0.1 * prices_clean.mean():
        print("警告：价格波动过小！强制使用等宽分箱。")
        edges = np.linspace(prices_clean.min(), prices_clean.max(), min_bins + 1)
    else:
        t = np.arange(len(prices_clean))
        weights = volumes_clean / volumes_clean.max()
        sigma = 1 / (weights + 1e-10)

        edges = bayesian_blocks(
            t=t,
            x=prices_clean.values,
            sigma=sigma,
            fitness='measures',
            p0=p0  # 降低p0值以增加分箱灵敏度
        )

        # 强制最小分箱数
        if len(edges) < min_bins + 1:
            edges = np.linspace(prices_clean.min(), prices_clean.max(), min_bins + 1)

    # 生成分箱并严格格式化
    bins = pd.cut(prices, bins=edges)
    formatted_bins = bins.apply(lambda x: f"{x.left:.2f}-{x.right:.2f}" if pd.notna(x) else np.nan)
    return formatted_bins.dropna()


# =====================================
# 2. 构建联合转移矩阵（优化分箱策略）
# =====================================
def build_joint_transition_matrix(prices, volumes, volume_bins=8):
    price_cat = volume_weighted_binning(prices, volumes)

    # 成交量分箱（改用等宽分箱增强稳定性）
    volume_k = volumes / 1000  # 转换为千单位
    volume_edges = np.linspace(volume_k.min(), volume_k.max(), volume_bins + 1)
    volume_labels = [f"{v:.0f}K-{volume_edges[i + 1]:.0f}K" for i, v in enumerate(volume_edges[:-1])]
    volume_cat = pd.cut(volume_k, bins=volume_edges, labels=volume_labels)

    # 数据清洗：双重验证
    valid_mask = price_cat.notna() & volume_cat.notna()
    price_cat = price_cat[valid_mask]
    volume_cat = volume_cat[valid_mask]

    # 构建状态对（当前→下一）
    current_state = list(zip(price_cat[:-1], volume_cat[:-1]))
    next_state = list(zip(price_cat.shift(-1).iloc[:-1], volume_cat.shift(-1).iloc[:-1]))

    # 收集所有有效状态并排序
    states = []
    for s in current_state + next_state:
        if all(isinstance(item, str) and '-' in item for item in s):
            states.append(s)
    states = sorted(list(set(states)),
                    key=lambda x: (float(x[0].split('-')[0]),
                                   float(x[1].split('K-')[0].replace('K', ''))))

    # 创建MultiIndex并指定层级名称
    index = pd.MultiIndex.from_tuples(
        states,
        names=['Price Interval', 'Volume Interval']
    )

    # 初始化转移矩阵
    matrix = np.zeros((len(states), len(states)))

    # 填充转移矩阵
    for cur, nxt in zip(current_state, next_state):
        if cur in index and nxt in index:
            i = index.get_loc(cur)
            j = index.get_loc(nxt)
            matrix[i, j] += 1

    # 安全归一化
    row_sums = matrix.sum(axis=1, keepdims=True)
    matrix = np.divide(matrix, row_sums, out=np.zeros_like(matrix), where=row_sums != 0)
    return pd.DataFrame(matrix, index=index, columns=index)


# =====================================
# 3. 查找最可能转移区间（增强过滤）
# =====================================
def find_top_transitions(joint_matrix, top_n=5, exclude_self=True):
    transitions = []

    for from_state in joint_matrix.index:
        for to_state in joint_matrix.columns:
            # 格式验证
            if not (isinstance(from_state[0], str) and isinstance(to_state[0], str)):
                continue
            if not (isinstance(from_state[1], str) and isinstance(to_state[1], str)):
                continue

            prob = joint_matrix.loc[from_state, to_state]

            # 有效性过滤
            if prob < 1e-8 or (exclude_self and from_state == to_state):
                continue

            transitions.append({
                'from_price': from_state[0],
                'from_volume': from_state[1],
                'to_price': to_state[0],
                'to_volume': to_state[1],
                'probability': prob
            })

    # 转换为DataFrame并排序
    df_trans = pd.DataFrame(transitions)
    return df_trans.sort_values('probability', ascending=False).head(top_n)


# =====================================
# 主程序执行
# =====================================
if __name__ == "__main__":
    # 构建联合转移矩阵
    joint_matrix = build_joint_transition_matrix(
        df['close'],
        df['volume'],
        volume_bins=8  # 增加成交量分箱数量
    )

    # 查找最可能转移区间
    top_transitions = find_top_transitions(joint_matrix, top_n=5)

    # 优化结果输出格式
    print("===" * 30)
    print("Top 5 最可能转移区间：")
    if not top_transitions.empty:
        print(top_transitions[[
            'from_price', 'from_volume',
            'to_price', 'to_volume',
            'probability'
        ]].to_string(index=False))
    else:
        print("未找到有效转移路径")
    print("===" * 30)

    # 可视化
    plt.figure(figsize=(14, 12))
    sns.heatmap(joint_matrix, cmap='viridis', annot=False, fmt=".2f", vmin=0, vmax=1)
    plt.title('价格-成交量联合转移概率矩阵', fontsize=14)

    # 高亮转移路径
    for _, row in top_transitions.iterrows():
        from_state = (row['from_price'], row['from_volume'])
        to_state = (row['to_price'], row['to_volume'])

        if from_state in joint_matrix.index and to_state in joint_matrix.columns:
            from_idx = joint_matrix.index.get_loc(from_state)
            to_idx = joint_matrix.columns.get_loc(to_state)
            plt.plot(to_idx + 0.5, from_idx + 0.5, 'r*', markersize=15)

    plt.xlabel('下一状态', fontsize=12)
    plt.ylabel('当前状态', fontsize=12)
    plt.xticks(rotation=45, ha='right')
    plt.yticks(rotation=0)
    plt.tight_layout()
    plt.show()