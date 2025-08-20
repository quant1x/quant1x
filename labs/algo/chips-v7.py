import pandas as pd

# import matplotlib.pyplot as plt
# plt.rcParams["font.sans-serif"]=["SimHei"] #设置字体
# plt.rcParams["axes.unicode_minus"]=False #该语句解决图像中的“-”负号的乱码问题
from quant1x.chart import pyplot as plt


def build_transition_matrix(prices, bins=20):
    # 分箱处理，确保包含所有区间
    price_bins = pd.cut(prices, bins=bins)
    categories = price_bins.cat.categories

    # 获取下一时间步的状态，并处理NaN
    next_bins = price_bins.shift(-1).iloc[:-1]  # 去掉最后一个NaN
    price_bins = price_bins.iloc[:-1]  # 对齐长度

    # 转换为Categorical以确保所有区间存在
    price_bins = pd.Categorical(price_bins, categories=categories)
    next_bins = pd.Categorical(next_bins, categories=categories)

    # 计算交叉表，填充0并平滑处理
    ct = pd.crosstab(price_bins, next_bins, dropna=False).fillna(0)
    ct += 1e-10  # 避免除以零

    # 归一化行概率
    matrix = ct.div(ct.sum(axis=1), axis=0)
    return matrix



from base1x import cache, exchange

code = '000701'
security_code = exchange.correct_security_code(code)
df = cache.klines(security_code)
print(df)
df = df[-30:-1]
print(df)

# 应用示例
matrix = build_transition_matrix(df['close'], bins=30)

# 可视化优化
plt.figure(figsize=(10, 8))
plt.imshow(matrix, cmap='hot', aspect='auto')
plt.colorbar(label='转移概率')

# 设置坐标轴标签
plt.xticks(range(len(matrix.columns)), matrix.columns, rotation=90)
plt.yticks(range(len(matrix.index)), matrix.index)
plt.xlabel('下一状态')
plt.ylabel('当前状态')
plt.title('价格状态转移矩阵热图')
plt.tight_layout()
plt.show()


def find_top_transitions(transition_matrix, top_n=5, exclude_self=True):
    # 将矩阵转换为长格式
    stacked = transition_matrix.stack().reset_index()
    stacked.columns = ['current_state', 'next_state', 'probability']

    # 排除自转移（可选）
    if exclude_self:
        stacked = stacked[stacked['current_state'] != stacked['next_state']]

    # 按概率排序并取前N个
    top_transitions = stacked.sort_values(by='probability', ascending=False).head(top_n)

    # 格式化价格区间为字符串
    top_transitions['current_state'] = top_transitions['current_state'].astype(str)
    top_transitions['next_state'] = top_transitions['next_state'].astype(str)

    return top_transitions


# 应用示例
top_transitions = find_top_transitions(matrix, top_n=5)
print("最容易迁移的价格区间：")
print(top_transitions)

import seaborn as sns

plt.figure(figsize=(10, 8))
sns.heatmap(matrix, annot=False, cmap='hot', linewidths=0.5)
plt.title('价格状态转移矩阵（高亮Top转移）')

# 高亮最大值（示例）
max_prob = top_transitions['probability'].values[0]
max_mask = (matrix == max_prob)
sns.heatmap(matrix, mask=~max_mask, annot=True, fmt=".2f", cmap='Reds', cbar=False)
plt.show()