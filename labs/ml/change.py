# 1. 模型训练
# 1.1 分类模型（预测涨跌）
import pandas as pd
from sklearn.model_selection import train_test_split
from xgboost import XGBClassifier

# quant1x 数据
data_path = 'd:/quant1x/data/day/sh000/sh000001.csv'
raw_data = pd.read_csv(data_path)
print(raw_data)
#exit(0)
# 定义标签（次日是否上涨）
data = raw_data.copy()
data['target'] = (data['close'].shift(-1) > 0).astype(int)

# 划分数据集
X = data[['open', 'close', 'high', 'close','volume','amount']]
y = data['target']
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, shuffle=False)

# 训练模型
model = XGBClassifier(
    n_estimators=100,
    max_depth=3,
    learning_rate=0.1
)
model.fit(X_train, y_train)
exit(0)

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

for i in range(len(X_test)):
    current_price = data.iloc[X_test.index[i]]['close']

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

# # 4. 生产部署
# # 4.1 模型保存与加载
# import joblib
#
# # 保存模型
# joblib.dump(model, 'stock_selection_model.pkl')
#
# # 加载模型
# loaded_model = joblib.load('stock_selection_model.pkl')
#
# # 4.2 实时预测
# def real_time_predict(new_data):
#     processed_data = preprocess(new_data)  # 需要与训练一致的预处理流程
#     return loaded_model.predict(processed_data)