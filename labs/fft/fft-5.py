import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from quant1x.data import D


# 函数功能：将频域数据转换成时序数据
# bins为频域数据，n设置使用前多少个频域数据，loop设置生成数据的长度

def fft_combine(bins, n, loops=1):
    length = int(len(bins) * loops)
    data = np.zeros(length)
    index = loops * np.arange(0, length, 1.0) / length * (2 * np.pi)
    for k, p in enumerate(bins[:n]):
        if k != 0: p *= 2  # 除去直流成分之外, 其余的系数都 * 2
        data += np.real(p) * np.cos(k * index)  # 余弦成分的系数为实数部分
        data -= np.imag(p) * np.sin(k * index)  # 正弦成分的系数为负的虚数部分
    return index, data


code = "002564"
# 读取股票数据
df = D.dataset(code=code)
df = df[-200:]
ts = df['close']
# ts = np.ndarray(ts)
ts = ts.values
ts = pd.Series(ts)
# 平稳化
ts_log = np.log(ts)
ts_diff = ts_log.diff(1)
ts_diff = ts_diff.dropna()
fy = np.fft.fft(ts_diff)
print(fy[:10])  # 显示前10个频域数据
conv1 = np.real(np.fft.ifft(fy))  # 逆变换
index, conv2 = fft_combine(fy / len(ts_diff), int(len(fy) / 2 - 1), 1.3)  # 只关心一半数据
plt.plot(ts_diff)
plt.plot(conv1 - 0.5)  # 为看清楚，将显示区域下拉0.5
plt.plot(conv2 - 1)
plt.show()
