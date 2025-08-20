import matplotlib.pyplot as plt
import numpy as np
import numpy.fft as fft

from quant1x.data import D

plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
plt.rcParams['axes.unicode_minus'] = False  # 用来正常显示符号

Fs = 100  # 采样频率
T = 1 / Fs  # 采样周期
L = 1000  # 信号长度
# t = [i * T for i in range(L)]
# t = np.array(t)
code = "600703"
# 读取股票数据
df = D.dataset(code=code)
data = df[-L:]
print(data)
t = data['close'].values
# S = np.fft.fft(t)

S = 0.2 + 0.7 * np.cos(2 * np.pi * 50 * t + 20 / 180 * np.pi) + 0.2 * np.cos(2 * np.pi * 100 * t + 70 / 180 * np.pi);

complex_array = fft.fft(S)
print(complex_array.shape)  # (1000,)
print(complex_array.dtype)  # complex128
print(complex_array[1])  # (-2.360174309695419e-14+2.3825789764340993e-13j)

#################################
plt.subplot(311)
plt.grid(linestyle=':')
plt.plot(1000 * t[1:51], S[1:51], label='S')  # y是1000个相加后的正弦序列
plt.xlabel("t（毫秒）")
plt.ylabel("S(t)幅值")
plt.title("叠加信号图")
plt.legend()

###################################
plt.subplot(312)
S_ifft = fft.ifft(complex_array)
# S_new是ifft变换后的序列
plt.plot(1000 * t[1:51], S_ifft[1:51], label='S_ifft', color='orangered')
plt.xlabel("t（毫秒）")
plt.ylabel("S_ifft(t)幅值")
plt.title("ifft变换图")
plt.grid(linestyle=':')
plt.legend()

###################################
# 得到分解波的频率序列
freqs = fft.fftfreq(t.size, t[1] - t[0])
# 复数的模为信号的振幅（能量大小）
pows = np.abs(complex_array)

plt.subplot(313)
plt.title('FFT变换,频谱图')
plt.xlabel('Frequency 频率')
plt.ylabel('Power 功率')
plt.tick_params(labelsize=10)
plt.grid(linestyle=':')
plt.plot(freqs[freqs > 0], pows[freqs > 0], c='orangered', label='Frequency')
plt.legend()
plt.tight_layout()
plt.show()
