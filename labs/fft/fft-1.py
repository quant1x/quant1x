#!/usr/bin/python
# -*- coding: UTF-8 -*-

import matplotlib.pyplot as plt
import numpy as np

from quant1x.data import D

code = "600703"
# 读取股票数据
df = D.dataset(code=code)
data = df[-89:]
# data = df[:89]
# data = df
print(data)

# 对数据按时间排序
# data = data.sort_values(by='date')

# 取出股票价格列
# prices = data['close'].values
prices = data['close'].values
print(prices)
# 对股票价格进行傅里叶变换
fft = np.fft.fft(prices)
print('fft =', fft)
# 绘制频域图
freqs = np.fft.fftfreq(prices.size)
plt.plot(freqs, np.abs(fft))
plt.xlabel('Frequency')
plt.ylabel('Amplitude')
plt.show()

# 对频域信号进行反变换
ifft = np.fft.ifft(fft)
print(ifft)

# 绘制原始股票价格和反变换后的股票价格
plt.plot(prices, label='Original')
plt.plot(ifft.real, label='Reconstructed')
plt.legend()
plt.show()
