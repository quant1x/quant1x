# from matplotlib.font_manager import *
#
# _rebuild()

import random

from matplotlib import font_manager  # 使用font-manager来管理字体样式，从而解决中文问题
from matplotlib import pyplot as plt

from quant1x.util import FONT_SimHei

# for font in font_manager.fontManager.ttflist:
#     print(font.name, '-', font.fname)

my_font = font_manager.FontProperties(fname=FONT_SimHei)  # 使用微软雅黑字体

x = range(0, 120)  # 假设时间轴为10点到12点的每分钟为刻度
y = [random.randint(20, 35) for i in range(120)]  # 生成120个数值

plt.figure(figsize=(20, 8), dpi=80)  # 通过初始化figure对象传入figsize指定图像的大小，

plt.plot(x, y)

# 通过xticks方法调整x轴的刻度
_x = list(x)  # 调整时间轴刻度的间隔
# _xtick_labels是自定义的刻度
_xtick_labels = ["10点{}分".format(i) for i in range(60)]  # 10点的范围
_xtick_labels += ["11点{}分".format(i) for i in range(60)]  # 11点的范围

# _x 与 _xtick_labels的数量应该一致，否则数据显示不全
plt.xticks(_x[::5], _xtick_labels[::5], rotation=45,
           fontproperties=my_font)  # rotation 旋转的度数,设置fontproperties属性为找到的字体样式对象

plt.xlabel("时间(s)", fontproperties=my_font)
plt.ylabel("温度(C°)", fontproperties=my_font)
plt.title("xxx市10点~12点每分钟的气温变换", fontproperties=my_font)

plt.show()
