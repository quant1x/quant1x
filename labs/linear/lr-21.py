import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from base1x import cache, exchange


# 生成模拟日线数据
def generate_daily_data(periods=300):
    dates = pd.date_range(start='2020-01-01', periods=periods, freq='D')
    data = {
        'open': np.random.normal(100, 5, periods),
        'high': np.random.normal(105, 5, periods),
        'low': np.random.normal(95, 5, periods),
        'close': np.random.normal(100, 5, periods),
        'volume': np.random.randint(1e6, 1e7, periods),
        'buy_vol': np.random.randint(3e5, 8e5, periods),
        'sell_vol': np.random.randint(3e5, 8e5, periods)
    }
    return pd.DataFrame(data, index=dates)


class DailyBBICalculator:
    def __init__(self):
        self.weights = {'vpi': 0.3, 'pressure': 0.25, 'obv': 0.3, 'fgi': 0.15}

    def compute_bbi(self, df):
        # 1. 计算VPI
        df['ma_volume'] = df['volume'].rolling(20, min_periods=1).mean()
        df['vpi'] = (df['close'] - df['open']) / df['open'] * (df['volume'] / (df['ma_volume'] + 1e-6))

        # 2. 计算买卖压力指数
        df['pressure'] = df['volume'] / (df['volume'] + 1e-6)
        df['pressure'] = (df['pressure'] - df['pressure'].rolling(20, min_periods=1).mean()) / (
                    df['pressure'].rolling(20, min_periods=1).std() + 1e-6)

        # 3. 计算OBV动量
        df['obv'] = (np.sign(df['close'].diff()) * df['volume']).cumsum()
        df['obv_momentum'] = (df['obv'] - df['obv'].rolling(5, min_periods=1).mean()) / (
                    df['obv'].rolling(5, min_periods=1).std() + 1e-6)

        # 4. 计算FGI
        # RSI计算
        delta = df['close'].diff()
        gain = delta.where(delta > 0, 0)
        loss = -delta.where(delta < 0, 0)
        avg_gain = gain.rolling(14, min_periods=1).mean()
        avg_loss = loss.rolling(14, min_periods=1).mean()
        rs = avg_gain / avg_loss
        df['rsi'] = 100 - (100 / (1 + rs))
        df['rsi_score'] = np.where(df['rsi'] > 70, 1, np.where(df['rsi'] < 30, -1, 0))

        # ATR计算
        df['tr'] = np.maximum(
            df['high'] - df['low'],
            np.abs(df['high'] - df['close'].shift()),
            np.abs(df['low'] - df['close'].shift())
        )
        df['atr'] = df['tr'].rolling(14, min_periods=1).mean()
        df['atr_rank'] = (df['atr'] / df['atr'].rolling(20, min_periods=1).mean()).apply(lambda x: max(x - 1, -0.5))

        # 换手率
        df['turnover'] = df['volume'] / 1e8  # 假设流通股1亿股
        df['turnover_rank'] = df['turnover'].expanding().apply(lambda x: (x <= x.iloc[-1]).sum() / len(x))

        # 综合FGI
        df['fgi'] = 0.4 * df['rsi_score'].fillna(0) + 0.3 * df['atr_rank'].fillna(0) + 0.3 * df['turnover_rank'].fillna(
            0)

        # 标准化处理
        for col in ['vpi', 'pressure', 'obv_momentum', 'fgi']:
            df[col] = (df[col] - df[col].mean(skipna=True)) / (df[col].std(skipna=True) + 1e-6)

        # 计算BBI
        df['BBI'] = (
                self.weights['vpi'] * df['vpi'] +
                self.weights['pressure'] * df['pressure'] +
                self.weights['obv'] * df['obv_momentum'] +
                self.weights['fgi'] * df['fgi']
        ).fillna(0)

        return df


# 主程序
if __name__ == '__main__':
    # code = '000701'
    # code = '002292'
    code = '300251'
    # code = '002276'
    code = '300940'
    code = '300759'
    code = '300107'
    code = '300456'
    code = '000156'
    code = '601228'
    # =====================================
    # 数据获取与预处理
    # =====================================
    security_code = exchange.correct_security_code(code)
    security_name = cache.stock_name(security_code)
    print(f'加载{security_name}({security_code})数据:')
    df = cache.klines(security_code)
    calculator = DailyBBICalculator()
    df = calculator.compute_bbi(df)

    # 生成交易信号（确保在BBI计算之后）
    df['signal'] = np.where(df['BBI'] > 0.5, 1,
                            np.where(df['BBI'] < -0.5, -1, 0))

    # 可视化
    plt.figure(figsize=(14, 7))
    plt.plot(df.index[200:], df['BBI'][200:], label='Daily BBI')
    plt.axhline(0.5, color='g', linestyle='--')
    plt.axhline(-0.5, color='r', linestyle='--')
    plt.title('Daily BBI Simulation')
    plt.legend()
    plt.show()

    # 显示信号（确保valid_signals包含signal列）
    valid_signals = df.iloc[200:]
    print(valid_signals[['date','BBI', 'signal']].tail(10))