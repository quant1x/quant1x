import matplotlib.pyplot as plt
import numpy as np

fs = 100  # frequency: 100  Hz
Fs = 1000  # sampling frequency: 1000 Hz

dt = 1 / Fs  # sampling period
N = 2048

T = N * dt  # span
print('T =', T)

t = np.linspace(0, T, N, endpoint=False)  # time
data = np.cos(2 * np.pi * fs * t) + np.random.normal(scale=0.2, size=len(t))

# data = np.random.randint(6, 10, 300)  # 生成随机数
X = np.fft.fft(data)  # Discrete Fourier Transform by fft
X = np.abs(X)
plt.plot(data[:100])
plt.show()
plt.plot(X)
plt.show()
print(1)

mean_X = np.mean(X)
distance = (X - mean_X) ** 2
mean_distance = np.mean(distance)
frequency = [i for i in range(len(distance)) if distance[i] > 0.8 * mean_distance]

length = len(X)
if len(frequency) > 2:
    if frequency[0] == 0:
        period = length // frequency[1]  # length个点中，完成了frequency[1]个周期
        if period >= 0.5 * length:
            print("none")
        print('1 =', period)
    else:
        period = length // frequency[1]
        if period >= 0.5 * length:
            print("none")
        print('2 =', period)
else:
    print("none")
