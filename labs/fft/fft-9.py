# 功能：把函数进行傅里叶变换，变换到频域，以期获得函数的周期
# 输入：时间序列，获取频率点数值n（可选），频率对应幅度的下限值fmin（可选）
# 输入序列的X轴需要归一化为1
# 输出： n个序列的下标以及对应的幅度值
# 创建时间： 2021-1-26

import math

import matplotlib.pyplot as plt
import scipy.signal as signal
from scipy.fftpack import fft

from quant1x.formula.formula import *


def fftTransfer(timeseries, n=10, fmin=0.2):
    yf = abs(fft(timeseries))  # 取绝对值
    yfnormlize = yf / len(timeseries)  # 归一化处理
    yfhalf = yfnormlize[range(int(len(timeseries) / 2))]  # 由于对称性，只取一半区间
    yfhalf = yfhalf * 2  # y 归一化

    xf = np.arange(len(timeseries))  # 频率
    xhalf = xf[range(int(len(timeseries) / 2))]  # 取一半区间

    plt.subplot(211)
    x = np.arange(len(timeseries))  # x轴
    plt.plot(x, timeseries)
    plt.title('Original wave')

    plt.subplot(212)
    plt.plot(xhalf, yfhalf, 'r')
    plt.title('FFT of Mixed wave(half side frequency range)', fontsize=10, color='#7A378B')  # 注意这里的颜色可以查询颜色代码表

    fwbest = yfhalf[signal.argrelextrema(yfhalf, np.greater)]
    xwbest = signal.argrelextrema(yfhalf, np.greater)
    plt.plot(xwbest[0][:n], fwbest[:n], 'o', c='yellow')
    plt.show(block=False)
    plt.show()

    xorder = np.argsort(-fwbest)  # 对获取到的极值进行降序排序，也就是频率越接近，越排前
    print('xorder =', xorder)
    print(type(xorder))
    xworder = list()
    xworder.append(xwbest[x] for x in xorder)  # 返回频率从大到小的极值顺序
    fworder = list()
    fworder.append(fwbest[x] for x in xorder)  # 返回幅度

    if len(fwbest) <= n:
        fwbest = fwbest[fwbest >= fmin].copy()
        return len(timeseries) / xwbest[0][:len(fwbest)], fwbest  # 转化为周期输出
    else:
        fwbest = fwbest[fwbest >= fmin].copy()
        print(len(fwbest))
        print(xwbest)
        return len(timeseries) / xwbest[0][:len(fwbest)], fwbest  # 只返回前n个数   #转化为周期输出


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
prices = data['close']
print(prices)
prices = MA(prices, 5)
prices = pd.Series(prices).fillna(0.00)
print(prices)

prices = prices.values
# xtime = np.arange(0, 1000, 1)
# xtime = prices
xtime = np.fft.fft(prices)
xnorm = xtime / len(xtime)
queshi = xnorm + 1
plt.plot(xtime, queshi)
zouqi = [np.sin(x * 5 * math.pi) for x in xnorm]
plt.title(label=code)
plt.plot(xtime, zouqi)
plt.show()
__signal = zouqi
y = __signal
y = pd.Series(y).astype('float')
df_price = y
x, y = fftTransfer(np.array(df_price), n=5, fmin=0.015)  # 快速傅里叶变换
print('x = ', x)  # 周期
print('y = ', y)  # 周期对应的增幅，也就是权重

# xtime = np.arange(0, 1000, 1)
# xnorm = xtime / len(xtime)
# queshi = xnorm + 1
# zouqi = [0.5 * np.sin(x * 20 * math.pi + 2 * math.pow(2 * x, 4) + 5 * np.cos(3 * x)) for x in xnorm]
# # zouqi = [sin(x*20*math.pi) for x in xnorm]
# plt.plot(xtime, zouqi)
# noize = 0.02 * np.random.normal(size=xtime.size)
# # plt.plot(xtime,noize)
# # signal = queshi+zouqi+noize
# __signal = zouqi
# testx = xtime
# y = __signal
# plt.plot(testx, y)
# plt.show()
# y = pd.Series(y).astype('float')
# x, y = fftTransfer(np.array(y), n=5, fmin=0.015)  # 快速傅里叶变换
# print('x = ', x)  # 周期
# print('y = ', y)  # 周期对应的增幅，也就是权重
