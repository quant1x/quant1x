from matplotlib import pyplot as plt
from matplotlib.font_manager import FontProperties

from quant1x.data import D
from quant1x.index.formula import *


# 画图
# 取000002万科A的数据

def bug_sell(zhuli, san, C, x):
    # 买入分解线
    bug = (zhuli - san)
    x_b = []
    y_b = []
    x_s = []
    y_s = []
    i = 0
    while i < len(bug):
        if bug[i] <= 0:
            bug[i] = -1
        elif bug[i] > 0:
            bug[i] = 1

        if i > 3:
            if bug[i] == 1 and (bug[i - 1] + bug[i - 2] + bug[i - 3]) == -3:
                x_b.append(x[i])
                y_b.append(C[i])
            elif bug[i] == -1 and (bug[i - 1] + bug[i - 2] + bug[i - 3]) == 3:
                x_s.append(x[i])
                y_s.append(C[i])

        i += 1
    return x_b, y_b, x_s, y_s


def san_zhu(h):
    # 定义参数

    CLOSE = h['close']
    HIGH = h['high']
    LOW = h['low']
    OPEN = h['open']
    M = 60
    N = 30

    # 散户线
    san = 100 * (HHV(HIGH, M) - CLOSE) / (HHV(HIGH, M) - LLV(LOW, M))
    # 主力线
    RSV = (CLOSE - LLV(LOW, N)) / (HHV(HIGH, N) - LLV(LOW, N)) * 100;
    K = SMA(RSV, 5, 1)
    D = SMA(K, 3, 1)
    J = 3 * K - 2 * D
    zhuli = EMA(J, 6)
    return san, zhuli


h = D.dataset('000002')
# h = pd.Series(df['close']).ewm(span=14, adjust=False).mean().values
C = h['close'] * 10
x = list(range(len(C)))

san, zhuli = san_zhu(h)
x_b, y_b, x_s, y_s = bug_sell(zhuli, san, C, x)

import matplotlib

# matplotlib.rc("font", family='simhei')
# matplotlib.rc("font", family='Wingdings 3')
matplotlib.use('TKAgg')
# plt.style.use('seaborn')

# plt.rcParams['font.sans-serif'] = [u'SimHei']
plt.rcParams['axes.unicode_minus'] = False

from quant1x.util import FONT_SimHei

font = FontProperties(fname=FONT_SimHei)

plt.title('测试', fontproperties=font)

plt.figure(figsize=(20, 8))
plt.plot(x, san, label='散户线')
plt.plot(x, zhuli, label='主力线')
plt.plot(x, C, h['close'] * 10, color='black', label='股票收盘价走势')
plt.scatter(x_b, y_b, s=100, color='red', label='买点')
plt.scatter(x_s, y_s, s=100, color='green', label='卖点')
plt.legend()
plt.show()
