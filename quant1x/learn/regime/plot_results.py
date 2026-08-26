# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

"""
可视化脚本: 绘制 Regime Switching 框架的运行结果
"""
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
#matplotlib.rcParams['font.family'] = 'WenQuanYi Micro Hei'
matplotlib.rcParams['axes.unicode_minus'] = False

import sys
sys.path.insert(0, '/data/workspace')

from ..quant1x.learn.regime_switching.regime_switching_framework import (
    DataEngine, RegimeDetector, FactorModel, PortfolioOptimizer,
    RegimeAwareBacktester, LiveMonitor, run_full_pipeline
)

np.random.seed(42)

# ── 更真实的合成数据: 有明显 regime 结构的市场 ──────────────────────
n_days = 1000
dates = pd.date_range('2022-01-01', periods=n_days, freq='B')

# 用马尔可夫链生成 regime 序列 (有持续性)
true_regime = np.zeros(n_days, dtype=int)
true_regime[0] = 0
for t in range(1, n_days):
    if true_regime[t-1] == 0:  # 牛市
        true_regime[t] = np.random.choice([0, 1, 2], p=[0.85, 0.10, 0.05])
    elif true_regime[t-1] == 1:  # 熊市
        true_regime[t] = np.random.choice([0, 1, 2], p=[0.10, 0.80, 0.10])
    else:  # 震荡
        true_regime[t] = np.random.choice([0, 1, 2], p=[0.15, 0.15, 0.70])

drifts = {0: 0.0010, 1: -0.0008, 2: 0.0001}
vols   = {0: 0.010,  1: 0.020,   2: 0.007}

n_stocks = 50
prices = pd.DataFrame(index=dates, 
                      columns=[f'S{i:02d}' for i in range(n_stocks)], 
                      dtype=float)

for i in range(n_stocks):
    # 每只股票有自己的 regime 偏移
    stock_regime = (true_regime + np.random.choice([-1,0,1], p=[0.2,0.6,0.2])) % 3
    rets = np.array([np.random.normal(drifts[r], vols[r]) for r in stock_regime])
    # 加入因子结构: 牛市中动量有效, 熊市中反转有效
    if i < 25:  # 动量型股票
        rets += np.array([0.0003 if r==0 else -0.0002 if r==1 else 0 for r in true_regime])
    else:  # 反转型股票
        rets += np.array([0.0002 if r==1 else -0.0001 if r==0 else 0.0001 for r in true_regime])
    
    price = 100 * np.cumprod(1 + rets)
    prices[f'S{i:02d}'] = price

market_idx = prices.mean(axis=1)

print(f"真实 Regime 分布: 牛市(0)={(true_regime==0).sum()}, "
      f"熊市(1)={(true_regime==1).sum()}, 震荡(2)={(true_regime==2).sum()}")

# ── 运行完整流水线 ──────────────────────────────────────────────────
results = run_full_pipeline(
    price_df=prices,
    market_index=market_idx,
    n_regimes=3
)

backtester = results['backtester']
regime_detector = results['regime_detector']
factor_model = results['factor_model']
live_monitor = results['live_monitor']

# ══════════════════════════════════════════════════════════════════
# 绘图
# ══════════════════════════════════════════════════════════════════
fig, axes = plt.subplots(4, 1, figsize=(16, 20), gridspec_kw={'height_ratios': [3, 1.5, 1.5, 1.5]})
fig.suptitle('Regime Switching 多因子量化框架 — 运行结果', fontsize=18, fontweight='bold', y=0.98)

# ── 图1: 净值曲线 + 市场指数 ──────────────────────────────────────
ax1 = axes[0]
nav = backtester.nav.dropna()
market_ret = market_idx.pct_change(1).fillna(0)
market_nav = (1 + market_ret).cumprod()

ax1.plot(nav.index, nav.values, label='策略净值', color='#E74C3C', linewidth=2)
ax1.plot(market_nav.index, market_nav.values, label='市场等权指数', color='#3498DB', 
         linewidth=1.5, alpha=0.7, linestyle='--')
ax1.set_ylabel('净值', fontsize=13)
ax1.set_title('策略净值 vs 市场基准', fontsize=14)
ax1.legend(fontsize=12, loc='upper left')
ax1.grid(True, alpha=0.3)

# 在净值图上标注 regime 切换
regime_hist = backtester.regime_history
for r in range(3):
    mask = regime_hist == r
    if mask.sum() > 0:
        first_date = mask.idxmax()
        ax1.axvline(x=first_date, color=['#2ECC71','#E74C3C','#F39C12'][r], 
                     alpha=0.3, linewidth=1, linestyle=':')

# ── 图2: Regime 状态序列 ──────────────────────────────────────────
ax2 = axes[1]
colors = ['#2ECC71', '#E74C3C', '#F39C12']
for r in range(3):
    mask = (regime_hist == r).values
    ax2.fill_between(range(len(regime_hist)), 0, 1, 
                     where=mask, 
                     color=colors[r], alpha=0.6, label=f'Regime {r}')

# 叠加真实 regime
true_series = pd.Series(true_regime, index=dates[:len(regime_hist)])
for r in range(3):
    mask_true = (true_series == r).values
    ax2.plot(np.where(mask_true)[0], [0.5]*mask_true.sum(), 
             'k|', markersize=8, alpha=0.3, label=f'真实{r}' if r==0 else None)

ax2.set_xlim(0, len(regime_hist))
ax2.set_yticks([])
ax2.set_title('HMM 推断 Regime (绿色=牛市, 红色=熊市, 橙色=震荡) | 黑色刻度=真实Regime', fontsize=12)
ax2.legend(fontsize=9, ncol=3, loc='upper right')

# ── 图3: 各 Regime 因子权重对比 (柱状图) ────────────────────────
ax3 = axes[2]
factor_names = ['mom_5', 'mom_20', 'mom_60', 'reversal', 'volatility', 'vol_price']
x = np.arange(len(factor_names))
width = 0.25

for k in range(3):
    imp = factor_model.get_factor_importance(k)
    vals = [imp.get(f, 0) for f in factor_names]
    ax3.bar(x + k * width - width, vals, width, 
            label=f'Regime {k}', color=colors[k], alpha=0.8)

ax3.axhline(y=0, color='black', linewidth=0.5)
ax3.set_xticks(x)
ax3.set_xticklabels(factor_names, fontsize=10)
ax3.set_ylabel('因子权重', fontsize=12)
ax3.set_title('各 Regime 下因子权重对比 (SGD 在线学习)', fontsize=14)
ax3.legend(fontsize=11)
ax3.grid(True, alpha=0.3, axis='y')

# ── 图4: 回撤曲线 ──────────────────────────────────────────────────
ax4 = axes[3]
cummax = nav.cummax()
drawdown = (nav - cummax) / cummax * 100  # 百分比
ax4.fill_between(drawdown.index, drawdown.values, 0, 
                 color='#E74C3C', alpha=0.3, label='策略回撤')
ax4.plot(drawdown.index, drawdown.values, color='#E74C3C', linewidth=1)
ax4.set_ylabel('回撤 (%)', fontsize=13)
ax4.set_title('策略回撤曲线', fontsize=14)
ax4.legend(fontsize=12)
ax4.grid(True, alpha=0.3)
ax4.axhline(y=-5, color='gray', linestyle='--', alpha=0.5, label='-5%警戒线')
ax4.legend(fontsize=10)

plt.tight_layout(rect=[0, 0, 1, 0.96])
plt.savefig('./regime_switching_results.png', dpi=150, bbox_inches='tight')
print("\n📊 图表已保存: ./regime_switching_results.png")

# ── 打印关键指标对比 ────────────────────────────────────────────────
perf = backtester.calculate_performance()
print("\n" + "="*60)
print("                    关键指标汇总")
print("="*60)
print(f"  策略总收益:     {perf['total_return']:.2%}")
print(f"  策略年化收益:   {perf['annual_return']:.2%}")
print(f"  策略夏普比率:   {perf['sharpe']:.2f}")
print(f"  策略最大回撤:   {perf['max_drawdown']:.2%}")
print(f"  市场总收益:     {market_nav.iloc[-1]-1:.2%}")
print(f"  超额收益:       {perf['total_return'] - (market_nav.iloc[-1]-1):.2%}")
print("="*60)

# ── 打印 Live Monitor 告警记录 ──────────────────────────────────────
if live_monitor.alerts:
    print(f"\n⚠️  实盘监控触发 {len(live_monitor.alerts)} 次告警:")
    for a in live_monitor.alerts:
        print(f"  {a['date'].date()}: z={a['z']:.2f}, 动作={a['action']}, 仓位={a['scale']:.0%}")
else:
    print("\n✅ 实盘监控期间无告警, 策略运行正常")
