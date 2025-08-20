import numpy as np
import pandas as pd
from xgboost import XGBClassifier

# 生成模拟数据
data = pd.DataFrame({
    'close': np.random.normal(0, 1, 1000).cumsum() + 100,
    'volume': np.random.randint(1000, 10000, 1000)
})

# 正确生成标签（次日是否上涨）
data['return'] = data['close'].pct_change()
data['target'] = (data['return'].shift(-1) > 0).astype(int)
data = data.dropna()

# 生成滞后特征（避免未来数据泄漏）
data['MA5'] = data['close'].rolling(5).mean().shift(1)
data['MA20'] = data['close'].rolling(20).mean().shift(1)
data = data.dropna()

# 检查标签分布
print("标签分布:\n", data['target'].value_counts())

# 划分数据集（时间序列安全）
X = data[['close','MA5', 'MA20', 'volume']]
y = data['target']
split_idx = int(len(data) * 0.8)
X_train, X_test = X[:split_idx], X[split_idx:]
y_train, y_test = y[:split_idx], y[split_idx:]

# 处理类别不平衡（示例：正样本较少）
pos_ratio = len(y_train[y_train == 0]) / len(y_train[y_train == 1])
model = XGBClassifier(
    scale_pos_weight=pos_ratio,
    n_estimators=200,
    max_depth=3,
    learning_rate=0.05
)

model.fit(X_train, y_train)
print("模型训练成功!")
import joblib

# 保存模型
joblib.dump(model, 'stock_selection_model.pkl')

# 加载模型
loaded_model = joblib.load('stock_selection_model.pkl')
print("模型训练特征顺序:", loaded_model.feature_importances_)

# 评估
from sklearn.metrics import accuracy_score, classification_report
pred = model.predict(X_test)
print(f"Accuracy: {accuracy_score(y_test, pred):.2f}")
print(classification_report(y_test, pred))

# 1.2 回归模型（预测收益率）
from xgboost import XGBRegressor

# 定义标签（未来5日收益率）
data['target'] = data['close'].pct_change(5).shift(-5)

# 训练回归模型
reg = XGBRegressor(
    n_estimators=200,
    max_depth=4,
    learning_rate=0.05
)
reg.fit(X_train, y_train.dropna())

# 评估
from sklearn.metrics import mean_squared_error
pred = reg.predict(X_test)
mse = mean_squared_error(y_test, pred)
print(f"MSE: {mse:.4f}")

# 2. 回测验证
# 滚动窗口回测
initial_capital = 1000000
position = 0

print('data length:', len(data))
print('X_test length:', len(X_test))
for i in range(len(X_test)):
    #print(i, X_test.index[i])
    current_price = data.loc[X_test.index[i]]['close']

    # 生成预测信号
    signal = model.predict(X_test.iloc[i:i + 1])

    # 交易逻辑
    if signal == 1 and position == 0:
        position = initial_capital // current_price
        initial_capital -= position * current_price
    elif signal == 0 and position > 0:
        initial_capital += position * current_price
        position = 0

# 计算最终收益
final_value = initial_capital + position * data['close'].iloc[-1]
print(f"Final Portfolio Value: {final_value:.2f}")

# 3. 模型优化技巧
# 3.1 特征选择
from sklearn.feature_selection import SelectFromModel

# 基于重要性筛选特征
selector = SelectFromModel(model, threshold=0.1)
X_important = selector.fit_transform(X, y)
# 3.2 参数调优
from sklearn.model_selection import GridSearchCV

param_grid = {
    'max_depth': [3, 5, 7],
    'learning_rate': [0.01, 0.1, 0.2]
}

grid_search = GridSearchCV(model, param_grid, cv=3)
grid_search.fit(X_train, y_train)
print(f"Best params: {grid_search.best_params_}")