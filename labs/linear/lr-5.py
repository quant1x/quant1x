import matplotlib
import matplotlib.pyplot as plt
import numpy as np

print(matplotlib.matplotlib_fname())


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
X = np.array([0, 1.34, 2.25, 4.67, 7.2, 9.6, 12.79, 15.61])
Y = np.array([0, 12.5, 25, 50, 100, 200, 400, 800])


# 定义直线拟合函数
def linear_regression(x, y):
    N = len(x)
    sumx = sum(x)
    sumy = sum(y)
    sumx2 = sum(x ** 2)
    sumxy = sum(x * y)

    A = np.mat([[N, sumx], [sumx, sumx2]])
    b = np.array([sumy, sumxy])

    return np.linalg.solve(A, b)


a10, a11 = linear_regression(X, Y)

# 生成拟合直线的绘制点
_X1 = np.arange(0, 20, 0.01)
_Y1 = np.array([a10 + a11 * x for x in _X1])

# 画图
plt.figure(figsize=(6, 6))
plt.plot(_X1, _Y1, 'b', linewidth=2)
plt.legend(bbox_to_anchor=(1, 0), loc="lower left")
plt.title("y = {} + {}x".format(a10, a11))  # 标题
plt.show()

A = gen_coefficient_matrix(X, Y)
b = gen_right_vector(X, Y)

print('A =', A)
print('b =', b)
a0, a1, a2 = np.linalg.solve(A, b)
print(a0, a1, a2)

# 绘制拟合曲线
_X = np.arange(0, 20, 1)
_Y = np.array([a0 + a1 * x + a2 * x ** 2 for x in _X])

# 画图
plt.figure(figsize=(10, 6))
plt.plot(X, Y, 'o', markersize=10, label='Hou等(2017)')

# plt.plot(_X, _Y, 'b', linewidth=2,label="$I$ = {:.2f} + {:.2e}$n$ +{:.2e}$n^2$ ".format(a0, a1, a2))    #{:.2f}等用于保留小数

plt.plot(_X, _Y, 'b', linewidth=2, label="式(1) ")
# plt.gca().invert_yaxis()
plt.legend(fontsize=16, frameon=False)
# plt.legend(bbox_to_anchor=(0.5,-0.35),loc=10,ncol=2,frameon=False) #图框
# plt.title("AI = {} + {}n + {}$n^2$ ".format(a0, a1, a2))   #标题
plt.show()
