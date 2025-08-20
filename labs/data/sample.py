#!/usr/bin/python
# -*- coding: UTF-8 -*-
import os

from quant1x import data
from quant1x.base.formula import *
from quant1x.data import D


def sample(code: str, end_time: str = None):
    """
    样本采集

    Notes
    -----
    .. math::
        ${Z}=\\frac{X-\\bar{X}}{s}=\\frac{x}{s}

    :param code: 证券代码
    :param end_time: 指定采样的结束时间, 默认全部数据
    :return:
    """
    data = D.dataset(code)
    data.set_index('date', inplace=True)
    if end_time != None:
        data = data[:end_time]
    df = data.copy()
    # print(df)
    # df = df[['date', 'close']]
    N = 10
    # matlab计算的z(0.9)=1.2816, z(0.95)=1.6449，z(0.975) = 1.96
    W = 1.2816
    CLOSE = df['close']
    mid = MA(CLOSE, N)
    bl = CLOSE / REF(CLOSE, N)
    variance = STD(CLOSE, N)  # 计算方差
    # 计算Z值, 即Width
    # W = (CLOSE - mid)/ variance
    up = mid + W * variance
    lower = mid - W * variance
    df['w'] = W
    df['bl'] = bl
    df['ma'] = mid
    df['v'] = variance
    df['up'] = up
    df['lower'] = lower
    df['result'] = (CLOSE > lower) & (CLOSE < up)
    # print(df)
    # df = df[['date', 'bl', 'result']]
    # df = df[['bl', 'result']]

    date = df.index[-1]
    date = date.strftime('%Y-%m-%d')
    bl = df['bl'][-1]
    result = df['result'][-1]
    return date, bl, result


date, bl, result = sample('002570', '2022-12-15')
if (bl > 1.05) and result:
    print("OK")
else:
    print("error")

# 是否需要无效数据做对比
need_invalid = True


def cb(symbol: str, name: str):
    date, bl, result = sample(symbol)
    df = pd.Series({'date': date, 'code': symbol, 'name': name, 'bl': bl, 'result': result})
    if need_invalid:
        # 全部输出
        return df
    elif bl >= 1.05 and result:
        return df
    else:
        return None


action = '样本数据采样'
df = D.apply(func_name=action, func=cb)
df = df[['date', 'code', 'name', 'bl', 'result']]
df.dropna(subset=['date'], inplace=True)

df.to_csv(os.path.expanduser(data.quant1x_data + '/' + action + '.csv'), index=False, encoding="UTF-8", quoting=1)
print(df)
# df = pd.read_csv(os.path.expanduser(data.quant1x_data + '/' + action + '.csv'))
# df = df[['date','code','name']]
# df.dropna(inplace=True)
# print(df)
