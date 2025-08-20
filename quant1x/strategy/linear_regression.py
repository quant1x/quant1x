#!/usr/bin/python
# -*- coding: UTF-8 -*-

"""
python机器学习-线性回归(LinearRegression)算法
https://blog.csdn.net/Arwen_H/article/details/82181288
"""

# 导入numpy库
import numpy as np
# 导入pandas库
import pandas as pd
# 导入机器学习库中的线性回归模块
from sklearn.linear_model import LinearRegression

from quant1x.data import D
from quant1x.formula import *


def predict(symbol: str, n: int = 0):
    """
    基础的线性回归
    :param code:
    :return:
    """

    df = D.dataset(symbol)
    df.set_index('date', inplace=True)
    #df['x'] = np.array(df.index)
    #df['x'] = df['open']
    print(df)
    data_length = len(df)
    # 创建一组N行2列的数据, serial_number为数据顺序序列号, close为收盘价
    data = pd.DataFrame({'serial_number': df['open']/REF(df['close'],1),
                         'close': df['close']})
    data.fillna(0.00, inplace=True)
    from matplotlib import pyplot as plt

    # 这里是将数据转化为一个1维矩阵
    data_train = np.array(data['serial_number']).reshape(data['serial_number'].shape[0], 1)
    data_test = data['close']

    # 创建线性回归模型, 拟合面积与价格并通过面积预测价格, 参数默认
    regr = LinearRegression()
    # 拟合数据, serial_number将房屋面积作为x,close价格作为y; 也可以理解用面积去预测价格
    regr.fit(data_train, data_test)
    t = data_length
    #print(df['x'])
    #t = df['x'][-1]
    t = df['open']/REF(df['close'],1)
    t = t[-1]
    a = regr.predict([[t]])
    # 查看预测结果
    print(a)
    # 查看拟合准确率情况,这里的检验是 R^2 , 趋近于1模型拟合越好
    print(regr.score(data_train, data_test))

    # 预测的结果：268.5平的房子价格为8833.54,  R^2 =0.967
    # 我们来画个图看一下数据最后是什么样的

    plt.scatter(data['serial_number'], data['close'])  # 画散点图看实际面积和价格的分布情况
    plt.plot(data['serial_number'],
             regr.predict(np.array(data['serial_number']).reshape(data['serial_number'].shape[0], 1)),
             color='red')  # 画拟合面积与价格的线型图
    plt.show()
