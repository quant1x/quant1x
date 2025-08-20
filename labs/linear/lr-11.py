from pylab import mpl
# 设置显示中文字体
mpl.rcParams["font.sans-serif"] = ["SimHei"]
mpl.rcParams['axes.unicode_minus'] = False
import numpy as np
import matplotlib.pyplot as plt

# 示例数据
t = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
P = np.array([100, 105, 108, 110, 112, 115, 117, 114, 110, 106])

# 二次多项式拟合
coefficients = np.polyfit(t, P, 2)
a, b, c = coefficients

# 计算顶点
t_vertex = -b / (2 * a)
P_vertex = c - (b**2) / (4 * a)

# 绘图
t_fit = np.linspace(1, 10, 100)
P_fit = a * t_fit**2 + b * t_fit + c

plt.scatter(t, P, label='实际股价')
plt.plot(t_fit, P_fit, 'r', label=f'拟合抛物线: P(t)={a:.2f}t²+{b:.2f}t+{c:.2f}')
plt.scatter(t_vertex, P_vertex, color='green', label=f'顶点(t={t_vertex:.1f}, P={P_vertex:.1f})')
plt.xlabel('交易日')
plt.ylabel('价格')
plt.legend()
plt.show()
