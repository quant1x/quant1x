import matplotlib.pyplot as plt
import numpy as np

from quant1x.data import D


def gen_coefficient_matrix(X, Y):
    N = len(X)
    m = 3
    A = []
    # 计算每一个方程的系数
    for i in range(m):
        a = []
        # 计算当前方程中的每一个系数
        for j in range(m):
            a.append(sum(X ** (i + j)))
        A.append(a)
    return A


# 计算方程组的右端向量b
def gen_right_vector(X, Y):
    N = len(X)
    m = 3
    b = []
    for i in range(m):
        b.append(sum(X ** i * Y))
    return b


# （2）自己设定组数
# X = np.array([0, 1.34, 2.25, 4.67, 7.2, 9.6, 12.79, 15.61])
# Y = np.array([0, 12.5, 25,     50, 100, 200,   400, 800])
code = '002528'
df = D.dataset(code)
df = df[0:-1]
print(df)
print('------------------------------------------------------------')
CLOSE = df['low']
length = 5
Y = np.arange(1, length + 1, 1)
X = CLOSE[-length:]

A = gen_coefficient_matrix(X, Y)
b = gen_right_vector(X, Y)

print('A =', A)
print('b =', b)
a0, a1, a2 = np.linalg.solve(A, b)
print(a0, a1, a2)

# 绘制拟合曲线
_X = np.arange(0, length + 1, 1.0)
_Y = np.array([a0 + a1 * x + a2 * x ** 2 for x in _X])

# 画图
plt.figure(figsize=(10, 6))
plt.plot(X, Y, 'o', markersize=10, label='Hou等(2017)')
plt.plot(_X, _Y, 'b', linewidth=2, label="式(1) ")
plt.legend(fontsize=16, frameon=False)
plt.show()
