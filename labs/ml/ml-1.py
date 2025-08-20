import warnings

import matplotlib.pyplot as plt
import numpy as np
from base1x import cache, exchange  # 自定义模块
from skopt import gp_minimize
from skopt.callbacks import DeltaYStopper
from skopt.optimizer import optimizer as skopt_optimizer
from skopt.space import Integer
from skopt.utils import use_named_args

plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False

# 禁用特定警告（可选）
warnings.filterwarnings(
    "ignore",
    category=UserWarning,
    module=skopt_optimizer.__name__
)

# 1. 下载历史数据
def fetch_data(code="sh000001", start="2010-01-01", end="2023-01-01"):
    security_code = exchange.correct_security_code(code)
    security_name = cache.stock_name(security_code)
    data = cache.klines(security_code)
    data['Return'] = data['close'].pct_change().fillna(0)  # 填充NaN
    return data


# 2. 定义MACD策略
def macd_strategy(params, data):
    fast, slow, signal = params
    ema_fast = data['close'].ewm(span=fast, adjust=False).mean()
    ema_slow = data['close'].ewm(span=slow, adjust=False).mean()
    macd_line = ema_fast - ema_slow
    signal_line = macd_line.ewm(span=signal, adjust=False).mean()
    buy_signal = (macd_line > signal_line) & (macd_line.shift() <= signal_line.shift())
    sell_signal = (macd_line < signal_line) & (macd_line.shift() >= signal_line.shift())
    strategy_returns = np.where(buy_signal, data['Return'],
                                np.where(sell_signal, -data['Return'], 0))
    return strategy_returns


# 3. 目标函数：最大化夏普比率
@use_named_args(dimensions=[
    Integer(5, 25, name='fast'),  # 扩大参数范围
    Integer(26, 60, name='slow'),  # 避免与fast重叠
    Integer(3, 20, name='signal')  # 扩大signal范围
])
def objective(**params):
    if params['fast'] >= params['slow']:
        return np.inf
    returns = macd_strategy([params['fast'], params['slow'], params['signal']], data)
    mean_return = np.mean(returns)
    std_return = np.std(returns)
    if std_return == 0:
        return np.inf
    sharpe_ratio = (mean_return / std_return) * np.sqrt(252)
    print(f"评估参数: fast={params['fast']}, slow={params['slow']}, signal={params['signal']} → 夏普比率: {sharpe_ratio:.2f}")
    return -sharpe_ratio


# 4. 主程序
if __name__ == "__main__":
    code = '300251'  # 光线传媒
    data = fetch_data(code=code)  # 明确传递code参数
    if data is None:
        exit()
    stopper = DeltaYStopper(delta=0.01, n_best=10)  # 连续10次改进<1%时停止
    # 贝叶斯优化
    result = gp_minimize(
        func=objective,
        dimensions=[
            Integer(5, 25, name='fast'),
            Integer(26, 60, name='slow'),
            Integer(3, 20, name='signal')
        ],
        n_calls=50,  # 增加迭代次数
        n_initial_points=20,  # 增加初始采样点
        random_state=42,
        n_jobs=-1, # 全核并行
        callback=stopper,
    )

    best_params = result.x
    print(f"最优参数: fast={best_params[0]}, slow={best_params[1]}, signal={best_params[2]}")

    # 回测最优参数
    optimal_returns = macd_strategy(best_params, data)
    # 对比默认参数（12,26,9）
    default_returns = macd_strategy([12, 26, 9], data)

    # 计算累计收益
    data['Optimal_Strategy'] = (1 + optimal_returns).cumprod()
    data['Default_Strategy'] = (1 + default_returns).cumprod()
    data['Buy_Hold'] = (1 + data['Return']).cumprod()

    # 可视化结果
    plt.figure(figsize=(12, 6))
    plt.plot(data['Optimal_Strategy'], label=f'Optimal MACD ({best_params[0]},{best_params[1]},{best_params[2]})')
    plt.plot(data['Default_Strategy'], label='Default MACD (12,26,9)', alpha=0.5)
    plt.plot(data['Buy_Hold'], label='Buy & Hold', linestyle='--')
    plt.title('Strategy Cumulative Returns')
    plt.legend()
    plt.show()