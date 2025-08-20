#!/usr/bin/python
# -*- coding: UTF-8 -*-

import matplotlib.pyplot as plt
# 曲线拟合——最小二乘法(Ordinary Least Square，OLS)
# https://blog.csdn.net/llittleSun/article/details/115045660
import numpy as np
import numpy.linalg as lg

from quant1x.data import D

code = '002528'
name = '英飞拓'
df = D.dataset(code)
df = df[0:-1]
print(df)
print('------------------------------------------------------------')
field = 'low'
CLOSE = df[field]
length = 5
t = np.arange(1, length + 1, 1)
print('t=', t)
y = CLOSE[-length:]
print('y=', y)
plt.figure()
plt.plot(t, y, 'k*')
# y=at^2+bt+c

A = np.c_[t ** 2, t, np.ones(t.shape)]
print('------------------------------------------------------------')
print('A.T=', A.T)
print('A=', A)
w0 = A.T.dot(A)
print('w0=', w0)
w1 = lg.inv(w0)
print('w1=', w1)
w2 = w1.dot(A.T)
print('w2=', w2)
w = w2.dot(y)
print('w=', w)
# w = lg.inv(A.T.dot(A)).dot(A.T).dot(y)
# print('w=', w)
print('------------------------------------------------------------')
t = np.arange(1, length + 2, 1)
print('t=', t)
wx = w[0] * t ** 2 + w[1] * t + w[2]
print('wx=', wx)
print('------------------------------------------------------------')
# plt.plot(t, wx, 'o', markersize=10, label='Hou等(2017)')
# t = np.array(df['date'][-length:])
# #t[3]='2023-02-22'
# print(t)
# plt.xlabel('Date')
# plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%m/%d/%Y'))
plt.plot(t, wx)
plt.title(name + ' ' + field)
plt.show()
print('------------------------------------------------------------')
