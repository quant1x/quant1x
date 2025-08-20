#!/usr/bin/python
# -*- coding: UTF-8 -*-


import matplotlib.pyplot as plt
import numpy as np

Fs = 150.0  # sampling rate采样率
Ts = 1.0 / Fs  # sampling interval 采样区间
t = np.arange(0, 1, Ts)  # time vector,这里Ts也是步长

ff = 25;  # frequency of the signal
y = np.sin(2 * np.pi * ff * t)

n = len(y)  # length of the signal
k = np.arange(n)
T = n / Fs
frq = k / T  # two sides frequency range
frq1 = frq[range(int(n / 2))]  # one side frequency range

YY = np.fft.fft(y)  # 未归一化
Y = np.fft.fft(y) / n  # fft computing and normalization 归一化
Y1 = Y[range(int(n / 2))]

fig, ax = plt.subplots(4, 1)

ax[0].plot(t, y)
ax[0].set_xlabel('Time')
ax[0].set_ylabel('Amplitude')

ax[1].plot(frq, abs(YY), 'r')  # plotting the spectrum
ax[1].set_xlabel('Freq (Hz)')
ax[1].set_ylabel('|Y(freq)|')

ax[2].plot(frq, abs(Y), 'g')  # plotting the spectrum
ax[2].set_xlabel('Freq (Hz)')
ax[2].set_ylabel('|Y(freq)|')

ax[3].plot(frq1, abs(Y1), 'b')  # plotting the spectrum
ax[3].set_xlabel('Freq (Hz)')
ax[3].set_ylabel('|Y(freq)|')

plt.show()
