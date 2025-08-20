import matplotlib.pyplot as plt
import numpy as np
from base1x import cache
from scipy.signal import argrelextrema

observer_num = 100
observer_window = 5
code = '300076'
df = cache.klines(code)
#print(df)
# # 生成示例价格数据（正弦波加噪声）
# np.random.seed(42)
# x = np.linspace(0, 100, 200)
# y = np.sin(x * 0.2) * 10 + np.random.normal(0, 1.5, len(x)) + 25
length = len(df)
if length >= observer_num:
    length = observer_num
df = df[-length:]
y = df['close'].values
x = df['close'].index

# 检测局部低点和高点
def find_extrema(data, comparator, order=observer_window):
    return argrelextrema(data, comparator, order=order)[0]


# 查找低点（支撑点）和高点（阻力点）
low_indices = find_extrema(y, np.less, order=observer_window)
high_indices = find_extrema(y, np.greater, order=observer_window)

x_lows = x[low_indices]
y_lows = y[low_indices]
x_highs = x[high_indices]
y_highs = y[high_indices]


# 二次多项式拟合函数
def quadratic_fit(x_points, y_points):
    coefficients = np.polyfit(x_points, y_points, 2)
    a, b, c = coefficients
    return lambda x: a * x ** 2 + b * x + c


# 生成支撑线和阻力线
support_fit = quadratic_fit(x_lows, y_lows)
resistance_fit = quadratic_fit(x_highs, y_highs)

# 生成拟合曲线数据
x_fit = np.linspace(x.min(), x.max(), 200)
support_line = support_fit(x_fit)
resistance_line = resistance_fit(x_fit)

# 可视化结果
plt.figure(figsize=(14, 7))
plt.plot(x, y, label='Price', alpha=0.5)
plt.scatter(x_lows, y_lows, color='green', label='Support Points', zorder=5)
plt.scatter(x_highs, y_highs, color='red', label='Resistance Points', zorder=5)
plt.plot(x_fit, support_line, color='green', linestyle='--', label='Support Line')
plt.plot(x_fit, resistance_line, color='red', linestyle='--', label='Resistance Line')
plt.title('Curved Support/Resistance Lines using Quadratic Polynomial Fit')
plt.xlabel('Time')
plt.ylabel('Price')
plt.legend()
plt.show()
