import matplotlib.pyplot as plt
import numpy as np
import pyhht

from quant1x.data import D


def likaiHHT_savefig_imfs(filepath, xlabel, title, signal, imfs, time_samples=None, fignum=None):
    if time_samples is None:
        time_samples = np.arange(signal.shape[0])

    n_imfs = imfs.shape[0]

    plt.figure(num=fignum)
    axis_extent = max(np.max(np.abs(imfs[:-1, :]), axis=0))

    # Plot original signal
    ax = plt.subplot(n_imfs + 1, 1, 1)
    ax.plot(time_samples, signal)
    ax.axis([time_samples[0], time_samples[-1], signal.min(), signal.max()])
    ax.tick_params(which='both', left=True, bottom=False, labelleft=True, labelbottom=False)
    ax.grid(False)
    ax.set_ylabel('signal')
    ax.set_title(title)

    # Plot the IMFs
    for i in range(n_imfs - 1):
        print(i + 2)
        ax = plt.subplot(n_imfs + 1, 1, i + 2)
        ax.plot(time_samples, imfs[i, :])
        ax.axis([time_samples[0], time_samples[-1], -axis_extent, axis_extent])
        ax.tick_params(which='both', left=True, bottom=False, labelleft=True, labelbottom=False)
        ax.grid(False)
        ax.set_ylabel('imf' + str(i + 1))

    # Plot the residue
    ax = plt.subplot(n_imfs + 1, 1, n_imfs + 1)
    ax.plot(time_samples, imfs[-1, :], 'r')
    ax.axis('auto')
    # ax.tick_params(which='both', left=False, bottom=False, labelleft=False,labelbottom=False)
    ax.grid(False)
    ax.set_ylabel('res.')
    ax.set_xlabel(xlabel)
    plt.savefig(filepath)
    return


def imfs_max_freq(imfs, sample_rate, fft_size):
    # 计算每一个imfs频谱中最高的那个频率
    n_imfs = imfs.shape[0]
    max_freq = []
    for i in range(n_imfs - 1):
        xs = imfs[i, :][:fft_size]
        xf = np.fft.rfft(xs) / fft_size
        freqs = np.linspace(0, sample_rate / 2, fft_size // 2 + 1)
        xfp = 20 * np.log10(np.clip(np.abs(xf), 1e-20, 1e100))
        max_freq.append(freqs[np.argmax(xfp)])
    return max_freq


code = "002297"
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
prices = data['close'].values
print(prices)

trading_day_num = len(prices)
t = np.linspace(0, trading_day_num, trading_day_num)
np_close = np.array(prices)
decomposer = pyhht.EMD(np_close)
imfs = decomposer.decompose()
# plot_imfs(np_close,imfs,t)
likaiHHT_savefig_imfs('./' + code + '.png', 't/day', code, np_close, imfs, t)
ls = imfs_max_freq(imfs, 1, 1000)  # 算每一段曲线的频率
print(ls)
