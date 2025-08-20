import matplotlib.pyplot as plt
import numpy as np
from scipy.signal import find_peaks

x = np.array([0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1])
from quant1x.data import D

code = "600703"
# 读取股票数据
df = D.dataset(code=code)
data = df[-89:]
print(data)
# 取出股票价格列
x = data['close'].values

N = x.shape[0]
print(N)
f = np.fft.fftn(x)  # 全部变换，f[0]为全部取值累加结果
f = f[:N // 2]
print('f =', f)

peaks, p = find_peaks(abs(f), height=0.1)
print('peaks =', peaks)

# print(f)
# plt.plot(abs(x), '.')
plt.plot(abs(f), '.')
print(abs(f))
print(f[6])
real = f[13].real
im = f[13].imag
theta = np.arctan(im / (real + 1e-6))
pos = np.angle(f[13]) / 2 / np.pi * 5
print(np.angle(f[13]))
print(theta, pos)
plt.show()
