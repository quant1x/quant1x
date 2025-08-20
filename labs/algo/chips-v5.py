import pandas as pd

# 原始价格序列（示例）
prices = pd.Series([100, 102, 101, 105, 103, 107, 108, 105, 104, 102])

# 步骤 1：分箱
price_bins = pd.cut(prices, bins=3)

# 步骤 2：构建状态对
X = price_bins[:-1]          # 当前状态
Y = price_bins[1:]           # 下一状态（已对齐）

# 步骤 3：统计频数
ct = pd.crosstab(X, Y)

# 步骤 4：归一化
transition_matrix = ct.div(ct.sum(axis=1), axis=0)

print("频数矩阵：\n", ct)
print("转移概率矩阵：\n", transition_matrix)
