import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from quant1x.data import D

code = "600703"
# 读取股票数据
df = D.dataset(code=code)
data = df[-89:]
print(data)
# 取出股票价格列
x = data['close'].values
# x = np.array([0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1])
x = np.fft.fft(x)
N = x.shape[0]
print(N)
f = np.fft.fftn(x)  # 全部变换，f[0]为全部取值累加结果
print('f =', f)

# peaks, p = find_peaks(f, height=0.1)
# print('peaks =', peaks)
from findpeaks import findpeaks

# Initialized
fp = findpeaks(method='topology')
# Peak detection
results = fp.fit(f)
print('results =', results['df'])
# Plot
fp.plot(text=code)
# Plot
fp.plot_persistence()
df = pd.DataFrame(results['df'])
df.to_csv(code + '.csv', index=False)

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
