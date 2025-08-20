#!/usr/bin/python
# -*- coding: UTF-8 -*-

import numpy as np
import pandas as pd


def AMPD(data):
    """
    实现AMPD算法
    :param data: 1-D numpy.ndarray
    :return: 波峰所在索引值的列表
    """
    p_data = np.zeros_like(data, dtype=np.int32)
    count = data.shape[0]
    print('count =', count)
    print('count // 2 + 1 =', count // 2 + 1)
    arr_rowsum = []
    for k in range(1, count // 2 + 1):
        row_sum = 0
        for i in range(k, count - k):
            if data[i] > data[i - k] and data[i] > data[i + k]:
                row_sum -= 1
        arr_rowsum.append(row_sum)
    min_index = np.argmin(arr_rowsum)
    max_window_length = min_index
    for k in range(1, max_window_length + 1):
        for i in range(k, count - k):
            if data[i] > data[i - k] and data[i] > data[i + k]:
                p_data[i] += 1
    result = np.where(p_data == max_window_length)
    print('result =', result)
    return result[0]


def sim_data():
    N = 1000
    x = np.linspace(0, 200, N)
    y = 2 * np.cos(2 * np.pi * 300 * x) \
        + 5 * np.sin(2 * np.pi * 100 * x) \
        + 4 * np.random.randn(N)
    return y


from quant1x.data import D

code = '002528'
name = '英飞拓'
# code = '000638'
# name = '万方发展'
# code = '002056'
# name = '横店东磁'
df = D.dataset(code)

# y = sim_data()
y = df['high']
x = df['date']
langth = 89
y = pd.Series(y[-langth:]).reset_index(drop=True)
print('y =', y)
x = pd.Series(x[-langth:]).reset_index(drop=True)
print('x =', x)

# plt.plot(range(len(y)), y)
# px = AMPD(y)
# plt.scatter(px, y[px], color="red")
# plt.show()


from findpeaks import findpeaks

# Initialized
fp = findpeaks(method='topology')
# Peak detection
results = fp.fit(y)
print('results =', results['df'])
# Plot
fp.plot(text=name)
# Plot
fp.plot_persistence()
df = results['df']
df.to_csv('002528.csv', index=False)

# from pyampd import ampd
# plt.plot(range(len(y)), y)
# results = ampd.find_peaks(y)
# print('results =', results)
# plt.scatter(results, y[results], color="red")
# plt.show()

# from scipy.signal import find_peaks
# #peaks, _ = find_peaks(y, height=0)
# peaks, _ = find_peaks(y)
# plt.plot(y)
# plt.plot(peaks, y[peaks], "x")
# #plt.plot(np.zeros_like(y), "--", color="gray")
# plt.show()
