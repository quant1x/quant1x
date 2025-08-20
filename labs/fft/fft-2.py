#!/usr/bin/python
# -*- coding: UTF-8 -*-

import matplotlib.pyplot as plt
import numpy as np
from scipy.fftpack import fft

Fs = 10000  # 采样频率
f1 = 390  # 信号频率1
f2 = 2000  # 信号频率2
t = np.linspace(0, 1, Fs)  # 生成 1s 的实践序列
noise1 = np.random.random(Fs)  # 0-1 之间的随机噪声
noise2 = np.random.normal(1, 10, Fs)
# 产生的是一个10e3的高斯噪声点数组集合（均值为：1，标准差：10）
y = 2 * np.sin(2 * np.pi * f1 * t) + 5 * np.sin(2 * np.pi * f2 * t) + noise2


def FFT(Fs, data):
    L = len(data)  # 信号长度
    # N = np.power(2, np.ceil(np.log2(L)))  # 下一个最近二次幂
    N = int(np.power(2, np.ceil(np.log2(L))))
    FFT_y1 = np.abs(fft(data, N)) / L * 2  # N点FFT 变化,但处于信号长度
    Fre = np.arange(int(N / 2)) * Fs / N  # 频率坐标
    FFT_y1 = FFT_y1[range(int(N / 2))]  # 取一半
    return Fre, FFT_y1


Fre, FFT_y1 = FFT(Fs, y)
plt.figure()
plt.plot(Fre, FFT_y1)
plt.grid()
plt.show()
