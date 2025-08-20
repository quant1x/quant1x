import matplotlib.pyplot as plt
import numpy as np

a = [1, 1, 1, 1]
print(a)

from quant1x.data import D

code = "600703"
# 读取股票数据
df = D.dataset(code=code)
data = df[-89:]
print(data)
# 取出股票价格列
prices = data['close'].values

a = prices
plt.plot(a)
plt.grid(True)
plt.xlim(0, 15)
plt.show()

dft_a = np.fft.fft(a, 4 * len(a))
print(dft_a)
plt.plot(dft_a)
plt.grid(True)
plt.xlim(0, 15)
plt.show()
