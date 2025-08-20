import pandas as pd

# 假设有3个板块的日线数据
data = {
    'date': pd.date_range('2023-01-01', periods=5),
    'tech': [
        {'close': 100, 'volume': 1e6, 'amount': 5e8},
        {'close': 102, 'volume': 1.2e6, 'amount': 6e8},
        {'close': 105, 'volume': 1.5e6, 'amount': 7e8},
        {'close': 103, 'volume': 1.3e6, 'amount': 6.5e8},
        {'close': 108, 'volume': 2e6, 'amount': 9e8}
    ],
    'finance': [
        {'close': 50, 'volume': 2e6, 'amount': 1e8},
        {'close': 49, 'volume': 1.8e6, 'amount': 9e7},
        {'close': 48, 'volume': 1.5e6, 'amount': 7.5e7},
        {'close': 47, 'volume': 1.2e6, 'amount': 6e7},
        {'close': 46, 'volume': 1e6, 'amount': 5e7}
    ],
    'energy': [
        {'close': 80, 'volume': 5e5, 'amount': 4e7},
        {'close': 82, 'volume': 6e5, 'amount': 5e7},
        {'close': 85, 'volume': 7e5, 'amount': 6e7},
        {'close': 83, 'volume': 6.5e5, 'amount': 5.5e7},
        {'close': 88, 'volume': 8e5, 'amount': 7e7}
    ]
}

# 转换为DataFrame
df = pd.DataFrame(data).set_index('date')


def analyze_fund_flow(df, window=3):
    """分析资金流向
    Args:
        df: 包含各板块日线数据的DataFrame
        window: 分析窗口大小（天数）
    Returns:
        资金转移方向报告
    """
    # 初始化资金流强度DataFrame并设置日期索引
    flow_strength = pd.DataFrame(index=df.index)

    for sector in df.columns:
        # 将嵌套字典转换为DataFrame并设置相同索引
        sector_df = pd.json_normalize(df[sector]).set_index(df.index)
        # 计算资金流指标：价格变化率 * 成交金额
        sector_df['flow'] = sector_df['close'].pct_change() * sector_df['amount']
        flow_strength[sector] = sector_df['flow']

    # 计算窗口内平均强度（使用min_periods=1确保初始窗口计算）
    window_flow = flow_strength.rolling(window, min_periods=1).mean()

    # 生成资金转移信号
    report = []
    for date in window_flow.dropna(how='all').index:  # 过滤全NaN行
        daily_flow = window_flow.loc[date]

        # 找出最强流入和流出板块（排除NaN值）
        valid_flows = daily_flow.dropna()
        if len(valid_flows) < 2:
            continue  # 确保至少有两个有效板块

        inflow_sector = valid_flows.idxmax()
        outflow_sector = valid_flows.idxmin()

        # 计算转移强度比率（处理除零风险）
        strength_ratio = valid_flows[inflow_sector] / abs(valid_flows[outflow_sector])

        if strength_ratio > 1.5:  # 强度阈值
            report.append({
                'date': date.strftime('%Y-%m-%d'),  # 正确格式化日期
                'from': outflow_sector,
                'to': inflow_sector,
                'strength': round(strength_ratio, 2)
            })

    return pd.DataFrame(report)


# 执行分析
flow_report = analyze_fund_flow(df)
print("资金转移方向报告：")
print(flow_report)
