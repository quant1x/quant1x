# plotlib_path = matplotlib.matplotlib_fname() #输出matplotlib包所在的配置文件的路径
# print(plotlib_path)
import matplotlib.pyplot as plt
import numpy as np
from base1x import cache, exchange
from scipy.signal import argrelextrema

plt.rcParams["font.sans-serif"]=["SimHei"] #设置字体
plt.rcParams["axes.unicode_minus"]=False #该语句解决图像中的“-”负号的乱码问题

observer_num = 89
observer_window = 3
code = '600714'
#code = '601020'
#code = '600633'
# code = "002195"
# code = '600580'
# code = '000810'
# code = '300543'
# code = '000801'
# code = '600588'
# code = '601177'
# code = '300433'
#code = '600188'
#code = '605398'
#code = '600732'
#code ='000034'
security_code = exchange.correct_security_code(code)
df = cache.klines(security_code)
print(df)
length = len(df)
if length >= observer_num:
    length = observer_num
df = df[-length:-2]
y = df['close'].values
x = df['close'].index.values  # 确保x是NumPy数组

# 检测局部低点和高点
def find_extrema(data, comparator, order=observer_window):
    return argrelextrema(data, comparator, order=order)[0]

low_indices = find_extrema(y, np.less, order=observer_window)
high_indices = find_extrema(y, np.greater, order=observer_window)

x_lows = x[low_indices]
y_lows = y[low_indices]
x_highs = x[high_indices]
y_highs = y[high_indices]

# 返回二次多项式系数和拟合函数
def quadratic_fit(x_points, y_points):
    coefficients = np.polyfit(x_points, y_points, 2)
    a, b, c = coefficients
    fit_func = lambda x_val: a * np.asarray(x_val)**2 + b * np.asarray(x_val) + c
    return coefficients, fit_func

# 拟合支撑线和压力线
support_coeffs, support_fit = quadratic_fit(x_lows, y_lows)
resistance_coeffs, resistance_fit = quadratic_fit(x_highs, y_highs)

# 计算交点
a1, b1, c1 = support_coeffs
a2, b2, c2 = resistance_coeffs
A, B, C = a1 - a2, b1 - b2, c1 - c2

intersections = []
threshold = 1e-10  # 处理数值稳定性

if abs(A) < threshold:  # 退化为一次方程
    if abs(B) < threshold:
        if abs(C) < threshold:
            print("支撑线和压力线重合")
        else:
            print("无交点")
    else:
        x_sol = -C / B
        if np.isreal(x_sol):
            x_sol = float(x_sol)
            y_sol = support_fit(x_sol)
            intersections.append((x_sol, y_sol))
else:  # 二次方程求解
    discriminant = B**2 - 4*A*C
    if discriminant >= 0:
        sqrt_d = np.sqrt(discriminant)
        x1 = (-B + sqrt_d) / (2*A)
        x2 = (-B - sqrt_d) / (2*A)
        for x_sol in [x1, x2]:
            if np.isreal(x_sol):
                x_sol = float(x_sol)
                y_sol = support_fit(x_sol)
                intersections.append((x_sol, y_sol))

# 去重并验证
intersections = list(set([(round(x,6), round(y,6)) for x, y in intersections]))
valid_intersections = []
for x_pt, y_pt in intersections:
    y_support = support_fit(x_pt)
    y_resistance = resistance_fit(x_pt)
    if np.isclose(y_support, y_resistance, atol=1e-6):
        valid_intersections.append((x_pt, y_pt))

# 输出结果
print("支撑线和压力线的交点坐标：")
current_x_min, current_x_max = x.min(), x.max()
for pt in valid_intersections:
    x_pt, y_pt = pt
    status = "当前数据范围内" if current_x_min <= x_pt <= current_x_max else "数据范围外"
    print(f"x={x_pt:.2f}, y={y_pt:.2f} ({status})")

# 扩展绘图范围以包含交点
x_values = [pt[0] for pt in valid_intersections] if valid_intersections else []
x_fit_min = min(x.min(), min(x_values)) if x_values else x.min()
x_fit_max = max(x.max(), max(x_values)) if x_values else x.max()
x_fit = np.linspace(x_fit_min, x_fit_max, 200)
support_line = support_fit(x_fit)
resistance_line = resistance_fit(x_fit)

# 绘图
plt.figure(figsize=(14,7))
plt.plot(x, y, label='价格', alpha=0.5)
plt.scatter(x_lows, y_lows, color='green', label='支撑点', zorder=5)
plt.scatter(x_highs, y_highs, color='red', label='压力点', zorder=5)
plt.plot(x_fit, support_line, 'g--', label='支撑线')
plt.plot(x_fit, resistance_line, 'r--', label='压力线')

if valid_intersections:
    x_pts = [pt[0] for pt in valid_intersections]
    y_pts = [pt[1] for pt in valid_intersections]
    plt.scatter(x_pts, y_pts, color='purple', marker='X', s=100, label='交点')

#plt.title('带交点的支撑/压力曲线')
plt.title(f'{security_code} - 支撑/压力曲线')
plt.legend()
plt.show()