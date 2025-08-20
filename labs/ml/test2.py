import numpy as np
import pandas as pd
from sklearn.ensemble import RandomForestClassifier

# 生成模拟数据
np.random.seed(42)
dates = pd.date_range('2020-01-01', periods=500)
data = pd.DataFrame({
    'close': np.random.normal(0, 1, 500).cumsum() + 100,
    'MA5': np.random.randn(500),
    'MA20': np.random.randn(500),
    'RSI': np.random.randn(500)
}, index=dates)
data['target'] = np.where(data['close'].shift(-1) > data['close'], 1, 0)
data = data.dropna()

# 划分训练集和测试集
split_idx = int(len(data) * 0.8)
train_data = data.iloc[:split_idx]
test_data = data.iloc[split_idx:]

# 训练模型
model = RandomForestClassifier(n_estimators=100)
model.fit(train_data[['MA5', 'MA20', 'RSI']], train_data['target'])


# 回测函数
def backtest(test_data, model):
    portfolio = [1000000]
    position = 0

    for i in range(len(test_data)):
        current = test_data.iloc[i]
        price = current['close']
        features = current[['MA5', 'MA20', 'RSI']].values.reshape(1, -1)

        signal = model.predict(features)[0]

        if signal == 1 and position == 0:
            position = portfolio[-1] // price
            portfolio.append(portfolio[-1] - position * price)
        elif signal == 0 and position > 0:
            portfolio.append(portfolio[-1] + position * price)
            position = 0
        else:
            portfolio.append(portfolio[-1])

    return portfolio


# 执行回测并绘图
portfolio = backtest(test_data, model)
pd.Series(portfolio).plot(figsize=(12, 6), title='Backtesting Result')
