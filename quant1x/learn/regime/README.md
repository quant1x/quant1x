# 带 Regime Switching 的多因子量化框架

## 解决的核心问题

> **回测很美，实盘拉垮** —— 本质是训练集和测试集始终代表"过去的 alpha"，
> 而交易机会回不到测试集的状态。

本框架通过 **显式建模市场状态 (Regime)** 来解决这个非平稳性问题。

---

## 架构总览

```
┌─────────────┐    ┌──────────────┐    ┌─────────────┐    ┌──────────────┐
│  DataEngine │───▶│ RegimeDet.  │───▶│ FactorModel │───▶│  Optimizer  │
│  数据 & 因子 │    │  HMM 状态识别│    │ 在线学习     │    │  组合优化    │
└─────────────┘    └──────────────┘    └─────────────┘    └──────┬───────┘
                                                                  │
┌─────────────┐    ┌──────────────┐    ┌───────────────────────────▼───────┐
│ LiveMonitor │◀──│  Backtester │◀──│  回测引擎 (Regime-Aware)          │
│ 实盘渐进验证 │    │  事件驱动    │    │  next-day-open 成交假设          │
└─────────────┘    └──────────────┘    └───────────────────────────────────┘
```

---

## 六大核心模块

### 1. DataEngine — 数据底座
- 价格数据 → 因子计算 → 中性化 → 标准化
- 因子: 短期/中期/长期动量, 反转, 波动率, 量价
- **防未来函数是底线**: 所有因子 shift(1)

### 2. RegimeDetector — HMM 市场状态识别 ⭐ 核心创新
- 观测变量: [日收益, 20日波动率, 20日偏度, 20日趋势]
- 隐状态: 牛市 / 熊市 / 震荡 (可扩展)
- Baum-Welch EM 训练 + Viterbi 解码
- **每个交易日重新推断当前 regime**
- 实盘: `predict_current_regime()` 每次调仓前调用

### 3. FactorModel — 多因子在线学习 ⭐ 核心创新
- **每个 Regime 独立维护一个 SGD 模型**
- Regime 切换 → 自动切换模型
- 指数衰减权重: 越近数据权重越高
- 自适应: 新 regime 出现时从零快速学习

### 4. PortfolioOptimizer — 组合优化 & 风控
- Top-N 选股 (按 alpha 排序)
- 单票上限 / 行业偏离 / 换手率约束
- 现金缓冲 / 波动率缩放

### 5. Backtester — Regime-Aware 回测
- **next-day-open 成交假设** (无前视偏差)
- 每天: 判断 regime → 更新模型 → 决策 → 计算收益
- 双边交易成本 (默认 2bp)
- 分 regime 统计绩效

### 6. LiveMonitor — 实盘渐进验证 ⭐ 最后防线
- 用最近 W 天实盘收益 vs 回测分布
- Z-score 偏离检测: |z| > 2σ → 降仓, > 3σ → 暂停
- **解决"回测美、实盘垮"的终极防线**

---

## 关键设计决策

| 设计点 | 选择 | 理由 |
|--------|------|------|
| 状态数量 | 3 (牛/熊/震荡) | 太少区分度不够, 太多样本不足 |
| 观测变量 | 收益+波动率+偏度+趋势 | 多维特征提高区分度 |
| 模型类型 | SGDRegressor (每个regime) | 在线学习, 快速适应 |
| 衰减因子 | 0.97 | 平衡遗忘速度与稳定性 |
| 成交假设 | next-day-open | 杜绝前视偏差 |
| 风控触发 | Z-score 2σ | 统计上约95%置信区间 |

---

## 快速开始

```python
from regime_switching_framework import run_full_pipeline

# 准备数据
price_df = pd.DataFrame(...)  # 日频价格, 行=日期, 列=股票
market_idx = price_df.mean(axis=1)  # 或用沪深300等指数

# 一键运行
results = run_full_pipeline(
    price_df=price_df,
    market_index=market_idx,
    n_regimes=3
)

# results 包含: data_engine, regime_detector, factor_model, 
#              backtester, live_monitor
```

---

## 接入真实数据的要点

### 数据要求
- 日频收盘价 (前复权)
- 建议 ≥ 3 年数据 (750+ 交易日)
- 股票数量 ≥ 30 只 (横截面统计才有意义)

### 因子扩展
在 `DataEngine` 中添加:
- 基本面因子: PE, PB, ROE, 营收增速
- 技术因子: RSI, MACD, 布林带位置
- 另类因子: 北向资金, 融资余额变化
- 情绪因子: 新闻情感, 分析师评级变化

### HMM 改进方向
- 用 `hmmlearn` 库替代自实现 (更稳健)
- 增加观测维度: 换手率, 涨跌停比例, 基差
- 用 variational inference 做贝叶斯 HMM

### 实盘部署
- `LiveMonitor` 是关键: 实盘前30天用小资金验证
- Z-score 触发降仓后, 需要人工复盘再恢复
- 建议加一层: 多个异构策略 ensemble, 单一策略失效不影响整体

---

## 文件说明

| 文件 | 说明 |
|------|------|
| `regime_switching_framework.py` | 完整框架代码 (6个类, ~1000行) |
| `plot_results.py` | 可视化脚本 (净值/regime/因子权重/回撤) |
| `regime_switching_results.png` | 运行结果图表 |
| `README.md` | 本文件 |

---

## 方法论总结

```
传统做法 (易过拟合):
  历史数据 → 拟合一个万能模型 → 期望未来也一样 → 💥

本框架做法 (Regime-Aware):
  历史数据 → 识别多种市场状态 → 每个状态学一个模型
  → 实时判断当前状态 → 用对应模型预测 → 持续在线更新
  → 实盘偏离就降仓 → ✅
```

**核心哲学**: 不追求"在历史里找圣杯", 而是"在不确定中活下来"。
