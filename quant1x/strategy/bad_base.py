#!/usr/bin/python
# -*- coding: UTF-8 -*-
"""
线性回归
基于线性回归的股票预测(scikit-learn) https://blog.csdn.net/qq_42433311/article/details/121382417
"""

# 线性回归算法一般用于解决"使用已知样本对未知公式参数的估计"类问题
# 获取数据
#     股票数据特征:开盘价(open)、最高价(high)、最低价(low)、收盘价(close)、交易额(volume)
#     及调整后的开盘价(open)、最高价(high)、最低价(low)、收盘价(close)、交易额(volume)
# 数据预处理
#     除权后的数据更能反映数据特征，选择调整后的数据为主要使用的数据特征
#     两个数据特征：HL_PCT(股票最高价与最低价变化百分比)、PCT_change(股票收盘价与最低价的变化百分比)
#     自变量为: close, HL_PCT, PCT_change, volume
#     因变量为: close

import math

import akshare as ak
import numpy as np
import pandas as pandas
from sklearn import model_selection
from sklearn import preprocessing
from sklearn.linear_model import LinearRegression

from quant1x.data import D


def predict(symbol: str, n: int = 0, percent: int = 1):
    """
    预测此后N个交易日的价格趋势

    ----
    :param symbol: 证券代码
    :param n: 预测几天
    :param percent: 预测后起的百分之percent的数据
    :return:
    """

    df = D.dataset(symbol)
    # 选择列, 是为了改变表头
    #df = df[["日期", "开盘", "收盘", "最高", "最低", "成交量", "成交额"]]
    # 变更表头
    #df.columns = ['date', 'open', 'close', 'high', 'low', 'volume', 'amount']
    # 更正排序
    #df['date'] = pandas.to_datetime(df['date'])
    df.set_index('date', inplace=True)

    # 定义预测列变量，存放研究对象的标签名
    forecast_col = 'close'
    # 默认预测1天
    forecast_out = 1
    # 如果预测天数为0, 则使用percent为所有数据量长度的百分比
    if n == 0 and percent > 0:
        forecast_out = int(math.ceil((percent / 100) * len(df)))
    elif n > 0 and percent ==0:
        forecast_out = n
    current_close = df['close'][-1]
    current_date = df.index[-1].strftime("%Y-%m-%d")


    # 只用到df中的下面几个字段
    df = df[['open', 'high', 'low', 'close', 'volume']]
    # 构造两个新列
    df['HL_PCT'] = (df['high'] - df['close']) / df['close'] * 100.0
    df['PCT_change'] = (df['close'] - df['open']) / df['open'] * 100.0
    # 真正用到的特征
    #df = df[['close', 'HL_PCT', 'PCT_change', 'volume']]
    df = df[['close']]
    # 处理空值，这里设置为-99999
    df.fillna(-99999, inplace=True)
    # label代表预测结果，通过让Adj. Close列的数据往前移动1%行来表示
    df['label'] = df[forecast_col].shift(-forecast_out)
    # 生成在模型中使用的数据X,y,以及预测时用到的数据X_lately
    # TODO: 会产生告警: FutureWarning: In a future version of pandas all arguments of DataFrame.drop except for the argument 'labels' will be keyword-only.
    # X = np.array(df.drop(['label'], 1))
    X = np.array(df.drop(labels=['label'], axis=1))
    train_inf = np.isinf(X)
    X[train_inf] = 0.00
    #X = preprocessing.scale(X)
    # 上面生成的label列时留下的最后1%行的数据，这些行并没有label 数据，用作预测时用到的输入数据
    X_lately = X[-(forecast_out+0):]
    X = X[:-forecast_out]
    # 抛弃label列中为空的那些行
    df.dropna(inplace=True)
    # 缺失的值填充
    df.fillna(0.00)
    # 无穷数值处理
    # print(np.isfinite(df).all())
    # False:不包含
    # True:包含
    #print(np.isinf(df).all())

    # while True:
    #     train_inf = np.isinf(df)
    #     df[train_inf] = 0

    y = np.array(df['label'])

    # 先把X，y数据分成两部份，训练和测试
    X_train, X_test, y_train, y_test = model_selection.train_test_split(X, y, test_size=0.2)
    # 生成线性回归对象
    clf = LinearRegression(n_jobs=-1)
    # 开始训练
    clf.fit(X_train, y_train)
    # 用测试数据评估准确性
    accuracy = clf.score(X_test, y_test)
    # 进行预测
    foreca_set = clf.predict(X_lately)
    print(foreca_set, accuracy)
    return current_date, current_close, foreca_set[-1], accuracy
