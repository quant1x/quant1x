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

import datetime
import math

import akshare as ak
import numpy as np
import pandas as pandas
from sklearn import model_selection
from sklearn.linear_model import LinearRegression

from quant1x.data import D

symbol = "002528"
name = "英飞拓"
#df = ak.stock_zh_a_hist(symbol=symbol, period="daily", adjust="qfq")
df = D.dataset(symbol)
df['x1'] = np.array(df.index)
#print(x1)
# # 选择列, 是为了改变表头
# df = df[["日期", "开盘", "收盘", "最高", "最低", "成交量", "成交额"]]
# # 变更表头
# df.columns = ['date', 'open', 'close', 'high', 'low', 'volume', 'amount']
# # 更正排序
# df['date'] = pandas.to_datetime(df['date'])
df.set_index('date', inplace=True)
print(df)

# 定义预测列变量，存放研究对象的标签名
forecast_col = 'open'
# 定义预测天数，这里设置为所有数据量长度的1%
forecast_out = int(math.ceil(0.01 * len(df)))
# 强制预测1天
forecast_out = 1
# 只用到df中的下面几个字段
df = df[['x1','open', 'high', 'low', 'close']]
# 构造两个新列
#df['HL_PCT'] = (df['high'] - df['close']) / df['close'] * 100.0
#df['PCT_change'] = (df['close'] - df['open']) / df['open'] * 100.0
# 真正用到的特征
#df = df[['close', 'HL_PCT', 'PCT_change']]
df = df[['open','x1']]
# 处理空值，这里设置为-99999
df.fillna(-99999, inplace=True)
# label代表预测结果，通过让close列的数据往前移动1%行来表示
df['label'] = df[forecast_col].shift(-forecast_out)
# 生成在模型中使用的数据X,y,以及预测时用到的数据X_lately
# TODO: 会产生告警: FutureWarning: In a future version of pandas all arguments of DataFrame.drop except for the argument 'labels' will be keyword-only.
# X = np.array(df.drop(['label'], 1))
X = np.array(df.drop(labels=['label'], axis=1))
#X = preprocessing.scale(X)
# 上面生成的label列时留下的最后1%行的数据，这些行并没有label 数据，用作预测时用到的输入数据
X_lately = X[-(forecast_out):]
X = X[:-forecast_out]
# 抛弃label列中为空的那些行
df.dropna(inplace=True)
y = np.array(df['label'])

# 先把X，y数据分成两部份，训练和测试
np.set_printoptions(suppress=True)
print('----------< X >----------')
print(X)
print('----------< y >----------')
print(y)
X_train, X_test, y_train, y_test = model_selection.train_test_split(X, y, test_size=0.2)
# 生成线性回归对象
clf = LinearRegression(n_jobs=-1)
# 开始训练
clf.fit(X_train, y_train)
# 用测试数据评估准确性
accuracy = clf.score(X_test, y_test)
# 进行预测
print(X_lately)
#X_lately = np.append(X_lately,[[12.54]], axis=0)
#X_lately = np.roll(X_lately, 1)
foreca_set = clf.predict(X_lately)
print(foreca_set, accuracy)
print(clf.coef_)
print(clf.intercept_)

import matplotlib.pyplot as plt
from matplotlib import style
import matplotlib.dates as mdates

# 在df中新建Forecast列，用于存放预测结果的数据
df['Forecast'] = np.nan
# 取df最后一行的时间索引
last_date = df.iloc[-1].name
print(df[-1:])
# 遍历预测结果，用它向df中追加行
for i in foreca_set:
    next_date = last_date + datetime.timedelta(days=1)
    while D.holiday(next_date):
        next_date = next_date + datetime.timedelta(days=1)
    # [np.nan  for _ in range(len(df.columns)-1)]生成不包含Forecast字段的列表
    # 而[i]是只包含Forecast字段的列表
    # 拼在一起组成新行，按日期追加到df下面
    t1 = [np.nan for _ in range(len(df.columns) - 1)] + [i]
    df.loc[next_date] = t1
    last_date = next_date

print(df)
print(clf.coef_)
df = df[-60:]
# 修改matplotlib样式
style.use('ggplot')
# 绘图
df['open'].plot()
df['Forecast'].plot()
plt.title(f"{name}({symbol})".format(name=name, symbol=symbol))
plt.legend(loc=4)
plt.xlabel('Date')
# 配置横坐标
plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%m/%d/%Y'))
# plt.gca().xaxis.set_major_locator(mdates.DayLocator())
# plt.xticks(rotation=20)
# 减小x轴刻度的间隔
from matplotlib.pyplot import MultipleLocator

# 把x轴的刻度间隔设置为0.005，并存在变量里
x_major_locator = MultipleLocator(10)
# 把x轴的主刻度设置为1的倍数
plt.gca().xaxis.set_major_locator(x_major_locator)
plt.ylabel('Price')
plt.show()
