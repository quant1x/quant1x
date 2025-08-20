import matplotlib.dates as mdates
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.patches import Rectangle


def generate_price_data(days=200):
    np.random.seed(42)
    t = np.arange(days)
    trend = 0.03 * t
    cycle = 8 * np.sin(0.08 * t)
    noise = np.random.normal(0, 1.5, days)

    price = []
    in_box = False
    box_level = None
    for i in range(len(t)):
        if i > 50 and not in_box and np.random.rand() < 0.02:
            in_box = True
            box_level = trend[i] + cycle[i]
            box_width = 5 + np.random.rand() * 3
        elif in_box and np.random.rand() < 0.05:
            in_box = False

        if in_box:
            p = box_level + np.random.uniform(-box_width, box_width)
            price.append(p)
        else:
            p = trend[i] + cycle[i] + noise[i]
            price.append(p)

    dates = pd.date_range('2023-01-01', periods=days)
    return pd.DataFrame({
        'Close': price,
        'High': np.array(price) + np.abs(np.random.normal(1, 0.5, days)),
        'Low': np.array(price) - np.abs(np.random.normal(1, 0.5, days))
    }, index=dates)


def detect_box(df, window=30, touch_threshold=3, tolerance=0.01):
    """
    window: 滚动窗口大小
    touch_threshold: 触碰次数要求
    tolerance: 触碰容差（价格距离边界的百分比）
    """
    df = df.copy()

    # 计算初始支撑阻力
    df['Resistance_raw'] = df['High'].rolling(window).max()
    df['Support_raw'] = df['Low'].rolling(window).min()

    # 初始化有效标记列
    df['Valid_Resistance'] = np.nan
    df['Valid_Support'] = np.nan

    # 遍历每个窗口检查触碰次数
    for i in range(window, len(df)):
        window_data = df.iloc[i - window:i]
        resistance = window_data['Resistance_raw'].iloc[-1]
        support = window_data['Support_raw'].iloc[-1]

        # 计算触碰次数
        resist_touches = np.sum(
            (window_data['High'] >= resistance * (1 - tolerance))
        )
        support_touches = np.sum(
            (window_data['Low'] <= support * (1 + tolerance))
        )

        # 标记有效边界
        df.iloc[i, df.columns.get_loc('Valid_Resistance')] = resistance if resist_touches >= touch_threshold else np.nan
        df.iloc[i, df.columns.get_loc('Valid_Support')] = support if support_touches >= touch_threshold else np.nan

    # 前向填充有效边界
    df['Resistance'] = df['Valid_Resistance'].ffill()
    df['Support'] = df['Valid_Support'].ffill()

    # 标记箱体区域
    df['InBox'] = (
            (df['Low'] >= df['Support'] * (1 - tolerance)) &
            (df['High'] <= df['Resistance'] * (1 + tolerance))
    )

    return df


def plot_box_strategy(df):
    plt.figure(figsize=(14, 8))
    ax = plt.gca()

    # 绘制价格线
    plt.plot(df['Close'], label='Close Price', color='#2c3e50', lw=1, zorder=1)

    # 绘制有效支撑阻力线
    plt.plot(df['Resistance'], color='#e74c3c', lw=1.5, label='Resistance')
    plt.plot(df['Support'], color='#27ae60', lw=1.5, label='Support')

    # 绘制箱体区域
    in_box = False
    start_idx = None
    for idx, row in df.iterrows():
        if row['InBox'] and not in_box:
            start_idx = idx
            in_box = True
        elif not row['InBox'] and in_box:
            end_idx = idx
            ax.add_patch(Rectangle(
                (mdates.date2num(start_idx), row['Support']),
                mdates.date2num(end_idx) - mdates.date2num(start_idx),
                row['Resistance'] - row['Support'],
                color='#3498db', alpha=0.2
            ))
            in_box = False

    # 标注触碰次数
    resist_points = df[df['High'] >= df['Resistance'] * 0.99]
    support_points = df[df['Low'] <= df['Support'] * 1.01]
    plt.scatter(resist_points.index, resist_points['High'],
                color='#e67e22', s=30, label='Resistance Touches')
    plt.scatter(support_points.index, support_points['Low'],
                color='#2ecc71', s=30, label='Support Touches')

    # 格式设置
    ax.xaxis.set_major_locator(mdates.MonthLocator())
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
    plt.xticks(rotation=45)
    plt.title('Box Strategy with Minimum 3 Touches', fontsize=14)
    plt.legend()
    plt.grid(alpha=0.2)
    plt.tight_layout()
    plt.show()


# 执行流程
df = generate_price_data(days=200)
df = detect_box(df, window=10, touch_threshold=3)
plot_box_strategy(df)