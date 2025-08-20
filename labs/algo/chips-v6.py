import matplotlib.pyplot as plt
plt.rcParams["font.sans-serif"]=["SimHei"] #设置字体
plt.rcParams["axes.unicode_minus"]=False #该语句解决图像中的“-”负号的乱码问题

from base1x import cache, exchange

code = '000701'
security_code = exchange.correct_security_code(code)
security_name = cache.stock_name(security_code)
print(f'加载{security_name}({security_code})数据:')
df = cache.klines(security_code)
df = df[-30:]
print(df[-1:])

import pandas as pd


def build_transition_matrix(prices, bins=20):
    price_bins = pd.cut(prices, bins=bins)
    matrix = pd.crosstab(price_bins, price_bins.shift(-1), normalize='index')
    return matrix

# 应用示例
matrix = build_transition_matrix(df['close'], bins=30)
print(matrix)
plt.imshow(matrix, cmap='hot', aspect='auto')
plt.colorbar(label='转移概率')
plt.show()