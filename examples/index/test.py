import matplotlib.pyplot as plt

from quant1x.data import D
from quant1x.formula import *

df = D.dataset('000002')
# print(df)

# -------有数据了，下面开始正题 -------------

CLOSE = df.close.values
OPEN = df.open.values  # 基础数据定义，只要传入的是序列都可以
HIGH = df.high.values
LOW = df.low.values  # 例如 CLOSE=list(df.close) 都是一样

up, mid, down = TAQ(HIGH, LOW, 20)  # 获取唐安奇交易通道数据，大道至简，能穿越牛熊

# 设置中文显示字体
# mpl.rcParams["font.sans-serif"] = ["simhei"]
# 设置正常显示符号
# mpl.rcParams["axes.unicode_minus"] = False

from matplotlib.font_manager import FontProperties
from quant1x.util import FONT_SimHei
font = FontProperties(fname=FONT_SimHei)

plt.title('测试', fontproperties=font)
plt.figure(figsize=(15, 8))
plt.plot(CLOSE, label='沪深300指数')
plt.plot(up, label='唐安奇-上轨')
plt.plot(mid, label='唐安奇-中轨')
plt.plot(down, label='唐安奇-下轨')
# plt.legend()
plt.show()
