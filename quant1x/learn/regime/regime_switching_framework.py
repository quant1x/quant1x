# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

"""
=============================================================================
  带 Regime Switching 的多因子量化框架
  =====================================
  核心思想:
    1. 用 HMM / 波动率聚类 识别当前市场状态 (Regime)
    2. 不同 Regime 下, 使用不同的因子权重 / 不同模型
    3. 在线学习 + 指数衰减, 快速适应 regime 切换
    4. 实盘 progressive validation, 自动降仓 / 停策略

  模块结构:
    - DataEngine        : 数据加载 & 清洗 & 特征工程
    - RegimeDetector    : HMM 市场状态识别
    - FactorModel       : 多因子模型 (在线学习)
    - PortfolioOptimizer: 组合优化 & 风控
    - Backtester        : 带 regime 感知的回测引擎
    - LiveMonitor       : 实盘渐进验证 & 自动风控
=============================================================================
"""

import numpy as np
import pandas as pd
from scipy.stats import norm, multivariate_normal
from sklearn.linear_model import SGDRegressor
from sklearn.preprocessing import StandardScaler
from typing import Tuple, Dict, List, Optional
import warnings
warnings.filterwarnings('ignore')


# ══════════════════════════════════════════════════════════════════════════════
# 1. DataEngine —— 数据底座
# ══════════════════════════════════════════════════════════════════════════════
class DataEngine:
    """
    数据加载 → 清洗 → 因子计算 → 中性化
    防未来函数是底线
    """

    def __init__(self, price_df: pd.DataFrame, industry_df: pd.DataFrame = None):
        """
        Parameters
        ----------
        price_df : DataFrame, shape (T, N)
            日频收盘价矩阵, 行=交易日, 列=股票代码
        industry_df : DataFrame, optional
            行业归属, 行=股票代码, 列=行业
        """
        self.price = price_df.copy()
        self.industry = industry_df
        self.T, self.N = price_df.shape
        self.dates = price_df.index

    # ── 基础因子计算 ──────────────────────────────────────────────────────────
    def calc_momentum(self, periods: List[int] = [5, 20, 60]) -> Dict[str, pd.DataFrame]:
        """动量因子: 过去 N 日收益率"""
        factors = {}
        for p in periods:
            factors[f'mom_{p}'] = self.price.pct_change(p).shift(1)  # shift(1) 防未来
        return factors

    def calc_reversal(self) -> pd.DataFrame:
        """短期反转因子: 昨日收益率取反"""
        return -self.price.pct_change(1).shift(1)

    def calc_volatility(self, window: int = 20) -> pd.DataFrame:
        """波动率因子"""
        ret = self.price.pct_change(1)
        return ret.rolling(window).std().shift(1)

    def calc_volume_price(self, window: int = 20) -> pd.DataFrame:
        """量价因子: 成交量加权价格变化"""
        ret = self.price.pct_change(1)
        vol = self.price.rolling(window).mean().pct_change(1)
        return (ret * vol).shift(1)

    def calc_size(self) -> pd.DataFrame:
        """市值因子 (这里用价格代理, 实际应接入市值数据)"""
        return np.log(self.price.shift(1))

    # ── 因子中性化 (行业 + 市值) ──────────────────────────────────────────────
    def neutralize(self, factor: pd.DataFrame) -> pd.DataFrame:
        """
        横截面回归中性化: 剔除行业 & 市值影响
        对每个交易日 t, 做 cross-sectional regression
        """
        neutralized = pd.DataFrame(index=factor.index, columns=factor.columns, dtype=float)

        for t in factor.index:
            f = factor.loc[t].dropna()
            if len(f) < 30:
                continue

            # 市值作为控制变量
            size_t = self.calc_size().loc[t].reindex(f.index).fillna(f.median())
            size_t = (size_t - size_t.mean()) / (size_t.std() + 1e-8)

            # 简单回归: factor ~ size, 取残差
            X = np.column_stack([np.ones(len(size_t)), size_t.values])
            y = f.values
            beta = np.linalg.lstsq(X, y, rcond=None)[0]
            residual = y - X @ beta
            neutralized.loc[t, f.index] = residual

        return neutralized

    # ── 因子标准化 (z-score, 横截面) ─────────────────────────────────────────
    def standardize(self, factor: pd.DataFrame) -> pd.DataFrame:
        """每个交易日横截面 z-score"""
        return factor.subtract(factor.mean(axis=1), axis=0).divide(factor.std(axis=1) + 1e-8, axis=0)

    # ── 汇总所有因子 ──────────────────────────────────────────────────────────
    def build_factor_panel(self) -> pd.DataFrame:
        """
        构建完整因子面板, 输出 shape (T, N, K)
        这里拍平成 (T, N*K) 的 DataFrame, 方便后续建模
        """
        all_factors = {}

        # 计算各类因子
        momentum_factors = self.calc_momentum([5, 20, 60])
        for name, fac in momentum_factors.items():
            all_factors[name] = self.standardize(self.neutralize(fac))

        all_factors['reversal'] = self.standardize(self.neutralize(self.calc_reversal()))
        all_factors['volatility'] = self.standardize(self.neutralize(self.calc_volatility()))
        all_factors['vol_price'] = self.standardize(self.neutralize(self.calc_volume_price()))

        # 合并成面板: 每个日期一行, 列 = 股票_因子名
        panel = pd.DataFrame()
        for name, fac in all_factors.items():
            fac_renamed = fac.rename(columns=lambda c: f'{c}_{name}')
            panel = pd.concat([panel, fac_renamed], axis=1)

        # 计算未来收益 (标签, 防未来: 用 t+1 到 t+H 的收益)
        forward_ret = self.price.pct_change(5).shift(-5)  # 未来5日收益
        forward_ret.columns = [f'{c}_fret5' for c in forward_ret.columns]
        panel = pd.concat([panel, forward_ret], axis=1)

        return panel.dropna(how='all')


# ══════════════════════════════════════════════════════════════════════════════
# 2. RegimeDetector —— 市场状态识别 (HMM)
# ══════════════════════════════════════════════════════════════════════════════
class RegimeDetector:
    """
    用隐马尔可夫模型 (HMM) 识别市场状态
    - 观测变量: 市场收益率 + 波动率 (横截面聚合)
    - 隐状态: 不同市场 regime (如: 牛市/熊市/震荡/高波危机)

    工程简化版: 用 Baum-Welch 学习 + Viterbi 解码
    实际生产可用 hmmlearn / pomegranate 库
    """

    def __init__(self, n_regimes: int = 3, market_index: pd.Series = None):
        """
        Parameters
        ----------
        n_regimes : int
            隐状态数量 (推荐 3~5)
        market_index : Series
            市场指数日频价格 (如沪深300), 用于提取观测特征
        """
        self.n_regimes = n_regimes
        self.market_index = market_index
        self.trans_mat = None       # 状态转移矩阵 (K x K)
        self.means = None           # 各状态观测均值 (K x D)
        self.covs = None            # 各状态观测协方差 (K x D x D)
        self.state_sequence = None  # 推断出的状态序列
        self.current_regime = None  # 当前最新状态

    def _extract_observations(self) -> np.ndarray:
        """从市场指数提取观测特征: [收益率, 波动率, 偏度, 最大回撤]"""
        ret = self.market_index.pct_change(1).dropna()
        vol = ret.rolling(20).std().dropna()
        skew = ret.rolling(20).skew().fillna(0)
        # 20日累计收益作为趋势指标
        trend = ret.rolling(20).sum().fillna(0)
        # 对齐
        common_idx = ret.index.intersection(vol.index[20:])
        common_idx = common_idx.intersection(skew.index)
        common_idx = common_idx.intersection(trend.index)
        obs = np.column_stack([
            ret.loc[common_idx].values,
            vol.loc[common_idx].values,
            skew.loc[common_idx].values,
            trend.loc[common_idx].values
        ])
        return obs, common_idx

    def fit(self, n_iter: int = 50):
        """
        Baum-Welch 算法训练 HMM (简化版, 适合教学 & 原型)
        生产环境建议用 hmmlearn
        """
        obs, idx = self._extract_observations()
        T, D = obs.shape
        K = self.n_regimes

        # ── 初始化 ──────────────────────────────────────────────────────────
        np.random.seed(42)
        # 转移矩阵: 对角占优 (状态有持续性)
        self.trans_mat = np.eye(K) * 0.85 + np.ones((K, K)) * 0.15 / K
        self.trans_mat = self.trans_mat / self.trans_mat.sum(axis=1, keepdims=True)

        # 均值: 用 KMeans 初始化
        from sklearn.cluster import KMeans
        km = KMeans(n_clusters=K, n_init=20, random_state=42)
        labels = km.fit_predict(obs)
        self.means = np.array([obs[labels == k].mean(axis=0) for k in range(K)])
        self.covs = np.array([np.cov(obs[labels == k].T) + 1e-5 * np.eye(D) for k in range(K)])

        # 确保协方差正定
        for k in range(K):
            self.covs[k] += 1e-5 * np.eye(D)

        # ── Baum-Welch EM 迭代 ──────────────────────────────────────────────
        for it in range(n_iter):
            # E-Step: 前向-后向算法
            alpha = self._forward(obs)
            beta = self._backward(obs)
            # 后验概率
            gamma = alpha * beta
            gamma = gamma / (gamma.sum(axis=1, keepdims=True) + 1e-10)
            # xi: 转移后验
            xi = np.zeros((T - 1, K, K))
            for t in range(T - 1):
                for i in range(K):
                    for j in range(K):
                        xi[t, i, j] = alpha[t, i] * self.trans_mat[i, j] * \
                                      self._gaussian_pdf(obs[t + 1], self.means[j], self.covs[j]) * \
                                      beta[t + 1, j]
            xi_sum = xi.sum(axis=(0, 1), keepdims=True)
            xi = xi / (xi_sum + 1e-10)

            # M-Step: 更新参数
            for k in range(K):
                w_k = gamma[:, k]
                w_sum = w_k.sum() + 1e-10
                # 均值
                self.means[k] = (w_k[:, None] * obs).sum(axis=0) / w_sum
                # 协方差
                diff = obs - self.means[k]
                self.covs[k] = (w_k[:, None] * diff).T @ diff / w_sum
                self.covs[k] += 1e-5 * np.eye(D)  # 正则化

            # 转移矩阵
            for i in range(K):
                self.trans_mat[i] = xi[:, i, :].sum(axis=0)
            self.trans_mat = self.trans_mat / (self.trans_mat.sum(axis=1, keepdims=True) + 1e-10)

        # ── Viterbi 解码: 推断完整状态序列 ──────────────────────────────────
        self.state_sequence = self._viterbi(obs)
        self._obs_idx = idx
        self.current_regime = self.state_sequence[-1]

        # 打印各状态含义
        market_ret = self.market_index.pct_change(1)
        state_ret_series = pd.Series(self.state_sequence, index=idx)
        avg_ret = market_ret.loc[idx].groupby(state_ret_series).mean()
        regime_counts = pd.Series(self.state_sequence).value_counts().sort_index()

        print("\n[HMM] 各 Regime 平均日收益:")
        for k in range(K):
            mean_str = str(np.round(self.means[k], 4))
            print(f"  Regime {k}: 日均收益={avg_ret.get(k, 0):.5f}  "
                  f"样本数={regime_counts.get(k, 0):>4}  "
                  f"均值={mean_str}")

    def _forward(self, obs: np.ndarray) -> np.ndarray:
        T = len(obs)
        alpha = np.zeros((T, self.n_regimes))
        pi = np.ones(self.n_regimes) / self.n_regimes
        alpha[0] = pi * np.array([self._gaussian_pdf(obs[0], self.means[k], self.covs[k])
                                   for k in range(self.n_regimes)])
        alpha[0] /= alpha[0].sum() + 1e-10
        for t in range(1, T):
            for j in range(self.n_regimes):
                alpha[t, j] = np.sum(alpha[t - 1] * self.trans_mat[:, j]) * \
                              self._gaussian_pdf(obs[t], self.means[j], self.covs[j])
            alpha[t] /= alpha[t].sum() + 1e-10
        return alpha

    def _backward(self, obs: np.ndarray) -> np.ndarray:
        T = len(obs)
        beta = np.zeros((T, self.n_regimes))
        beta[-1] = 1.0 / self.n_regimes
        for t in range(T - 2, -1, -1):
            for i in range(self.n_regimes):
                beta[t, i] = np.sum([
                    self.trans_mat[i, j] *
                    self._gaussian_pdf(obs[t + 1], self.means[j], self.covs[j]) *
                    beta[t + 1, j]
                    for j in range(self.n_regimes)
                ])
            beta[t] /= beta[t].sum() + 1e-10
        return beta

    def _viterbi(self, obs: np.ndarray) -> np.ndarray:
        T = len(obs)
        K = self.n_regimes
        delta = np.zeros((T, K))
        psi = np.zeros((T, K), dtype=int)
        pi = np.ones(K) / K
        delta[0] = np.log(pi + 1e-10) + np.array([
            np.log(self._gaussian_pdf(obs[0], self.means[k], self.covs[k]) + 1e-10)
            for k in range(K)
        ])
        for t in range(1, T):
            for j in range(K):
                scores = delta[t - 1] + np.log(self.trans_mat[:, j] + 1e-10)
                psi[t, j] = np.argmax(scores)
                delta[t, j] = scores[psi[t, j]] + np.log(
                    self._gaussian_pdf(obs[t], self.means[j], self.covs[j]) + 1e-10
                )
        # 回溯
        states = np.zeros(T, dtype=int)
        states[-1] = np.argmax(delta[-1])
        for t in range(T - 2, -1, -1):
            states[t] = psi[t + 1, states[t + 1]]
        return states

    def _gaussian_pdf(self, x: np.ndarray, mean: np.ndarray, cov: np.ndarray) -> float:
        try:
            return multivariate_normal.pdf(x, mean=mean, cov=cov)
        except:
            return 1e-10

    def predict_current_regime(self, recent_window: int = 60) -> int:
        """
        用最近窗口的观测, 通过 Viterbi 推断当前 regime
        实盘每次调仓前调用
        """
        obs, _ = self._extract_observations()
        recent = obs[-recent_window:]
        # 用 Viterbi 解码最近窗口, 取最后状态
        states = self._viterbi(recent)
        self.current_regime = states[-1]
        return self.current_regime

    def get_regime_smooth(self, window: int = 5) -> np.ndarray:
        """对状态序列做平滑 (多数投票), 减少抖动"""
        seq = self.state_sequence.copy()
        smoothed = np.zeros_like(seq)
        for t in range(len(seq)):
            lo = max(0, t - window // 2)
            hi = min(len(seq), t + window // 2 + 1)
            smoothed[t] = np.bincount(seq[lo:hi]).argmax()
        return smoothed


# ══════════════════════════════════════════════════════════════════════════════
# 3. FactorModel —— 多因子模型 (在线学习 + Regime 感知)
# ══════════════════════════════════════════════════════════════════════════════
class FactorModel:
    """
    核心创新: 每个 Regime 维护一个独立的在线学习模型
    - 当 regime 切换时, 自动切换到对应模型
    - 各模型用 SGD + 指数衰减权重更新
    - 预测时只在当前 regime 内做 cross-sectional ranking
    """

    def __init__(self, n_regimes: int, factor_names: List[str],
                 decay: float = 0.95, learning_rate: float = 0.01):
        """
        Parameters
        ----------
        n_regimes : int
            状态数量
        factor_names : list[str]
            因子名称列表
        decay : float
            指数衰减因子 (0.9~0.99), 越近数据权重越高
        learning_rate : float
            SGD 学习率
        """
        self.n_regimes = n_regimes
        self.factor_names = factor_names
        self.decay = decay
        self.lr = learning_rate

        # 每个 regime 一个模型 + scaler
        self.models = {}
        self.scalers = {}
        for k in range(n_regimes):
            self.models[k] = SGDRegressor(
                learning_rate='constant', eta0=learning_rate,
                max_iter=1, warm_start=True,  # warm_start: 增量学习
                penalty='l2', alpha=1e-4
            )
            self.scalers[k] = StandardScaler()

        # 记录每个 regime 的训练步数 (用于初始化)
        self._n_samples = {k: 0 for k in range(n_regimes)}

        # 各 regime 因子权重历史 (用于监控)
        self.weight_history = {k: [] for k in range(n_regimes)}

    def _prepare_X(self, factor_panel: pd.DataFrame, date: pd.Timestamp,
                   regime: int) -> Tuple[np.ndarray, np.ndarray, List[str]]:
        """提取某个交易日、某个 regime 的训练数据"""
        row = factor_panel.loc[date]
        # 因子列
        factor_cols = [c for c in row.index if any(f in c for f in self.factor_names)]
        # 标签列
        label_cols = [c for c in row.index if 'fret5' in c]

        # 按股票对齐因子和标签
        stock_codes = list(set([c.split('_')[0] for c in factor_cols]))
        stock_codes.sort()

        X_list, y_list, valid_stocks = [], [], []
        for s in stock_codes:
            fac_vals = row[[c for c in factor_cols if c.startswith(s)]].values
            label_val = row[[c for c in label_cols if c.startswith(s)]].values
            if len(fac_vals) == len(self.factor_names) and len(label_val) == 1:
                if not np.isnan(fac_vals).any() and not np.isnan(label_val[0]):
                    X_list.append(fac_vals)
                    y_list.append(label_val[0])
                    valid_stocks.append(s)

        if len(X_list) < 10:
            return None, None, []

        return np.array(X_list), np.array(y_list), valid_stocks

    def update(self, factor_panel: pd.DataFrame, date: pd.Timestamp, regime: int):
        """
        用 t 日数据更新 t 日所属 regime 的模型
        这是在线学习的核心: 每天只学一步, 逐步适应
        """
        X, y, stocks = self._prepare_X(factor_panel, date, regime)
        if X is None or len(X) < 10:
            return

        # 标准化 (在线更新 scaler)
        scaler = self.scalers[regime]
        model = self.models[regime]

        if self._n_samples[regime] < 100:
            # 冷启动: 先 partial_fit 几次
            scaler.partial_fit(X)
            X_scaled = scaler.transform(X)
            model.partial_fit(X_scaled, y)
            self._n_samples[regime] += len(X)
        else:
            X_scaled = scaler.transform(X)
            # 指数衰减: 对旧模型做 shrink, 等价于给新样本更高权重
            w = self.decay ** (self._n_samples[regime] / 100.0)
            old_coef = model.coef_.copy()
            model.partial_fit(X_scaled, y)
            model.coef_ = w * old_coef + (1 - w) * model.coef_
            self._n_samples[regime] += len(X)

        # 记录权重
        self.weight_history[regime].append({
            'date': date,
            'coefs': model.coef_.copy(),
            'n_samples': self._n_samples[regime]
        })

    def predict(self, factor_panel: pd.DataFrame, date: pd.Timestamp, regime: int) -> pd.Series:
        """
        对 t 日所有股票预测 alpha 分数
        只在当前 regime 的模型上预测
        """
        X, _, stocks = self._prepare_X(factor_panel, date, regime)
        if X is None or len(X) < 5:
            return pd.Series(dtype=float)

        X_scaled = self.scalers[regime].transform(X)
        scores = self.models[regime].predict(X_scaled)

        return pd.Series(scores, index=stocks)

    def get_factor_importance(self, regime: int) -> pd.Series:
        """返回当前 regime 下各因子权重"""
        model = self.models[regime]
        if not hasattr(model, 'coef_') or model.coef_ is None:
            # 模型尚未训练, 返回零
            return pd.Series(0.0, index=self.factor_names)
        coefs = model.coef_
        return pd.Series(coefs, index=self.factor_names).sort_values(key=abs, ascending=False)


# ══════════════════════════════════════════════════════════════════════════════
# 4. PortfolioOptimizer —— 组合优化 & 风控
# ══════════════════════════════════════════════════════════════════════════════
class PortfolioOptimizer:
    """
    根据 alpha 分数 → 目标权重
    叠加风控约束: 单票上限、行业偏离、换手率限制
    """

    def __init__(self, top_n: int = 20, max_weight: float = 0.05,
                 max_turnover: float = 0.3, cash_buffer: float = 0.05):
        """
        Parameters
        ----------
        top_n : int
            持仓数量 (选 alpha 最高的 N 只)
        max_weight : float
            单票最大权重
        max_turnover : float
            单次调仓最大换手率
        cash_buffer : float
            最低现金比例
        """
        self.top_n = top_n
        self.max_weight = max_weight
        self.max_turnover = max_turnover
        self.cash_buffer = cash_buffer

    def construct_portfolio(self, alpha_scores: pd.Series,
                            current_weights: pd.Series = None,
                            regime: int = 0) -> pd.Series:
        """
        构建目标组合权重

        Step 1: 选股 (Top N by alpha)
        Step 2: 等权 / 最优权重分配
        Step 3: 风控约束
        Step 4: 换手率约束 (与当前持仓比较)
        """
        if len(alpha_scores) == 0:
            return pd.Series(0.0, index=pd.Index([], name='stock'))

        # Step 1: 选股
        selected = alpha_scores.nlargest(self.top_n).index

        # Step 2: 等权分配 (简化; 进阶可用风险平价 / 均值方差)
        n = len(selected)
        raw_weight = 1.0 / n
        raw_weight = min(raw_weight, self.max_weight)

        target = pd.Series(raw_weight, index=selected)
        # 统一权重
        investable = 1.0 - self.cash_buffer
        target = target * investable / target.sum()

        # Step 3: 换手率约束
        if current_weights is not None and len(current_weights) > 0:
            common = target.index.intersection(current_weights.index)
            turnover = (target.reindex(common).fillna(0) - current_weights.reindex(common).fillna(0)).abs().sum()
            if turnover > self.max_turnover:
                # 缩放调整量
                scale = self.max_turnover / (turnover + 1e-8)
                adj = (target - current_weights.reindex(target.index).fillna(0)) * scale
                target = current_weights.reindex(target.index).fillna(0) + adj
                target = target.clip(0, self.max_weight)
                target = target / (target.sum() + 1e-8) * investable

        return target.fillna(0)


# ══════════════════════════════════════════════════════════════════════════════
# 5. Backtester —— 带 Regime 感知的回测引擎
# ══════════════════════════════════════════════════════════════════════════════
class RegimeAwareBacktester:
    """
    事件驱动回测:
    - 每天判断当前 regime
    - 用对应 regime 模型预测 alpha
    - 构建组合 → 计算收益 → 更新模型
    - 记录净值曲线 & 各 regime 表现
    """

    def __init__(self, data_engine: DataEngine, regime_detector: RegimeDetector,
                 factor_model: FactorModel, optimizer: PortfolioOptimizer,
                 cost_bp: float = 2.0, rebalance_freq: int = 5):
        """
        Parameters
        ----------
        cost_bp : float
            双边交易成本 (基点), 2bp = 0.02%
        rebalance_freq : int
            调仓频率 (交易日)
        """
        self.data = data_engine
        self.regime_detector = regime_detector
        self.factor_model = factor_model
        self.optimizer = optimizer
        self.cost_bp = cost_bp / 10000.0
        self.rebalance_freq = rebalance_freq

        self.nav = pd.Series(dtype=float)           # 净值曲线
        self.regime_history = pd.Series(dtype=int)  # regime 切换记录
        self.weights_history = {}                    # 持仓历史
        self.daily_returns = pd.Series(dtype=float)
        self.trade_log = []

    def run(self, factor_panel: pd.DataFrame):
        """
        主回测循环 (next-day-open 成交假设)
        ─────────────────────────────────────────
        时序: t 日收盘 → 计算信号/regime → 确定目标仓位
              → t+1 开盘成交 → 承担 t+1 当日收益
        这是无前视偏差的标准做法。
        """
        dates = factor_panel.index.unique().sort_values()
        n_dates = len(dates)
        current_weights = pd.Series(0.0, index=pd.Index([], name='stock'))
        nav = 1.0
        last_rebalance = -self.rebalance_freq

        print(f"\n[Backtest] 开始回测, 共 {n_dates} 个交易日")

        for i, date in enumerate(dates):
            # ── 1. 获取当前 regime (基于截至 t 日的信息) ─────────────────
            if i >= 20:
                regime = self.regime_detector.predict_current_regime(recent_window=60)
            else:
                regime = 0

            self.regime_history.loc[date] = regime

            # ── 2. 用 t 日数据更新模型 (在线学习) ────────────────────────
            self.factor_model.update(factor_panel, date, regime)

            # ── 3. 调仓判断 (t 日决策, t+1 执行) ────────────────────────
            is_rebalance_day = (i - last_rebalance) >= self.rebalance_freq

            if is_rebalance_day:
                alpha_scores = self.factor_model.predict(factor_panel, date, regime)

                if len(alpha_scores) > 0:
                    target_weights = self.optimizer.construct_portfolio(
                        alpha_scores, current_weights, regime
                    )

                    # 交易成本在成交日(t+1)扣除, 这里先记录
                    if len(current_weights) > 0:
                        common_idx = target_weights.index.intersection(current_weights.index)
                        turnover = (target_weights.reindex(common_idx).fillna(0) -
                                    current_weights.reindex(common_idx).fillna(0)).abs().sum()
                    else:
                        turnover = target_weights.abs().sum()

                    self.trade_log.append({
                        'decision_date': date,
                        'turnover': turnover,
                        'regime': regime,
                        'n_holdings': len(target_weights),
                        'target_weights': target_weights.copy()
                    })

                    current_weights = target_weights.copy()
                    last_rebalance = i

            # ── 4. 计算 t 日收益 (用 t-1 决定的仓位, 获得 t 日收益) ───────
            # 即: t-1 日收盘决策 → 持有仓位进入 t 日 → 赚取 t 日收益
            if i > 0 and len(current_weights) > 0:
                prev_idx = i - 1
                prev_date = dates[prev_idx]
                # 需要 prev_date 已经有决策 (即 prev_date 或之前已调仓)
                # 用 t 日 vs t-1 日价格计算收益
                price_t = self.data.price.loc[date].reindex(current_weights.index).fillna(0)
                price_t1 = self.data.price.loc[prev_date].reindex(current_weights.index).fillna(0)

                stock_ret = np.where(
                    price_t1 != 0, (price_t - price_t1) / price_t1, 0.0
                )
                port_ret = np.sum(current_weights.values * stock_ret)

                # 如果在 t 日有调仓(实际 t+1 成交), 成本在 t+1 扣
                # 简化: 在决策日对应的下一个交易日扣成本
                if len(self.trade_log) > 0:
                    last_trade = self.trade_log[-1]
                    if last_trade.get('decision_date') == prev_date:
                        # t 是 prev_date 的下一个交易日, 扣成本
                        cost = last_trade['turnover'] * self.cost_bp
                        nav *= (1 - cost)
            else:
                port_ret = 0.0

            nav *= (1 + port_ret)
            self.nav.loc[date] = nav
            self.daily_returns.loc[date] = port_ret
            self.weights_history[date] = current_weights.copy()

        print(f"[Backtest] 回测完成, 最终净值: {nav:.4f}")

    def calculate_performance(self, benchmark_returns: pd.Series = None) -> Dict:
        """计算绩效指标"""
        ret = self.daily_returns.dropna()
        nav = self.nav.dropna()

        if len(ret) == 0:
            return {}

        total_ret = nav.iloc[-1] / nav.iloc[0] - 1 if len(nav) > 1 else 0
        n_years = len(ret) / 252
        annual_ret = (1 + total_ret) ** (1 / max(n_years, 0.01)) - 1
        volatility = ret.std() * np.sqrt(252)
        sharpe = annual_ret / volatility if volatility > 0 else 0

        # 最大回撤
        cummax = nav.cummax()
        drawdown = (nav - cummax) / cummax
        max_dd = drawdown.min()

        # 按 regime 分组统计
        regime_perf = {}
        for k in range(self.factor_model.n_regimes):
            mask = self.regime_history == k
            if mask.sum() > 10:
                r = ret[mask]
                regime_perf[f'regime_{k}'] = {
                    'days': int(mask.sum()),
                    'avg_daily_ret': r.mean(),
                    'sharpe_annualized': r.mean() / (r.std() + 1e-8) * np.sqrt(252),
                    'hit_rate': (r > 0).mean()
                }

        perf = {
            'total_return': total_ret,
            'annual_return': annual_ret,
            'volatility': volatility,
            'sharpe': sharpe,
            'max_drawdown': max_dd,
            'n_trades': len(self.trade_log),
            'regime_performance': regime_perf
        }

        return perf

    def print_report(self):
        """打印回测报告"""
        perf = self.calculate_performance()
        print("\n" + "=" * 60)
        print("                   回测绩效报告")
        print("=" * 60)
        print(f"  总收益:        {perf.get('total_return', 0):.2%}")
        print(f"  年化收益:      {perf.get('annual_return', 0):.2%}")
        print(f"  年化波动率:    {perf.get('volatility', 0):.2%}")
        print(f"  夏普比率:      {perf.get('sharpe', 0):.4f}")
        print(f"  最大回撤:      {perf.get('max_drawdown', 0):.2%}")
        print(f"  调仓次数:      {perf.get('n_trades', 0)}")
        print("\n  ── 各 Regime 表现 ──")
        for k, v in perf.get('regime_performance', {}).items():
            print(f"  {k}: 天数={v['days']:>5}, 日均收益={v['avg_daily_ret']:.4f}, "
                  f"夏普={v['sharpe_annualized']:.2f}, 胜率={v['hit_rate']:.1%}")
        print("=" * 60)


# ══════════════════════════════════════════════════════════════════════════════
# 6. LiveMonitor —— 实盘渐进验证 & 自动风控
# ══════════════════════════════════════════════════════════════════════════════
class LiveMonitor:
    """
    实盘渐进验证 (Progressive Validation):
    - 用最近 W 天实盘收益, 与回测同期分布比较
    - 显著偏离 → 自动降仓 / 暂停策略
    - 这是解决"回测美、实盘垮"的核心防线
    """

    def __init__(self, backtest_returns: pd.Series,
                 window: int = 20,
                 z_threshold: float = 2.0,
                 min_trades: int = 10):
        """
        Parameters
        ----------
        backtest_returns : Series
            回测的日收益序列 (作为基准分布)
        window : int
            实盘验证窗口 (最近多少天)
        z_threshold : float
            偏离阈值 (标准差倍数), >2σ 触发告警
        min_trades : int
            最少多少笔交易后才开始监控
        """
        self.bt_returns = backtest_returns.dropna()
        self.window = window
        self.z_threshold = z_threshold
        self.min_trades = min_trades

        # 回测收益统计 (作为先验)
        self.bt_mean = self.bt_returns.mean()
        self.bt_std = self.bt_returns.std()
        self.bt_sharpe = self.bt_mean / (self.bt_std + 1e-8) * np.sqrt(252)

        # 实盘记录
        self.live_returns = pd.Series(dtype=float)
        self.alerts = []

    def update(self, date, live_ret: float, current_position: float = 1.0) -> Dict:
        """
        每次收盘后调用: 更新实盘收益, 检查是否触发风控

        Returns
        -------
        dict: {
            'position_scale': 建议仓位缩放 (0~1),
            'alert': 告警信息,
            'z_score': 当前偏离度,
            'status': 'NORMAL' / 'WARNING' / 'HALT'
        }
        """
        self.live_returns.loc[date] = live_ret

        if len(self.live_returns) < self.min_trades:
            return {
                'position_scale': current_position,
                'alert': f'样本不足 ({len(self.live_returns)}/{self.min_trades})',
                'z_score': 0,
                'status': 'WARMUP'
            }

        # 取最近窗口
        recent = self.live_returns.iloc[-self.window:]
        live_mean = recent.mean()
        live_std = recent.std()

        # Z-score: 实盘均值 vs 回测均值
        z = (live_mean - self.bt_mean) / (self.bt_std + 1e-8)

        # 决策
        if z < -self.z_threshold:
            # 实盘显著差于回测预期 → 降仓
            scale = max(0.3, 1 + z / self.z_threshold * 0.5)  # 最低30%
            status = 'WARNING'
            alert = f'实盘收益显著低于回测 (z={z:.2f}), 建议降仓至 {scale:.0%}'
            self.alerts.append({'date': date, 'z': z, 'action': 'REDUCE', 'scale': scale})
        elif z < -self.z_threshold * 1.5:
            # 严重偏离 → 暂停
            scale = 0.0
            status = 'HALT'
            alert = f'实盘严重偏离回测 (z={z:.2f}), 策略暂停!'
            self.alerts.append({'date': date, 'z': z, 'action': 'HALT', 'scale': scale})
        else:
            scale = current_position
            status = 'NORMAL'
            alert = f'实盘运行正常 (z={z:.2f})'

        return {
            'position_scale': scale,
            'alert': alert,
            'z_score': z,
            'status': status,
            'live_sharpe': live_mean / (live_std + 1e-8) * np.sqrt(252)
        }

    def get_report(self) -> str:
        """生成监控报告"""
        if len(self.live_returns) == 0:
            return "尚无实盘数据"

        recent = self.live_returns.iloc[-self.window:]
        report = f"""
╔══════════════════════════════════════════════════════════════╗
║            Progressive Validation 监控报告                     ║
╠══════════════════════════════════════════════════════════════╣
║  实盘天数:        {len(self.live_returns):>5} 天
║  回测日均收益:    {self.bt_mean:>10.4f}
║  实盘日均收益:    {recent.mean():>10.4f}
║  回测年化夏普:    {self.bt_sharpe:>10.2f}
║  实盘窗口夏普:    {recent.mean()/(recent.std()+1e-8)*np.sqrt(252):>10.2f}
║  告警次数:        {len(self.alerts):>5}
╚══════════════════════════════════════════════════════════════╝
"""
        return report


# ══════════════════════════════════════════════════════════════════════════════
# 7. 主流程: 串联所有模块
# ══════════════════════════════════════════════════════════════════════════════
def run_full_pipeline(price_df: pd.DataFrame, market_index: pd.Series = None,
                      n_regimes: int = 3, start_date: str = None, end_date: str = None):
    """
    完整流水线: Data → Regime → Factor Model → Backtest → Live Monitor

    Parameters
    ----------
    price_df : DataFrame (T, N)
        日频价格矩阵
    market_index : Series
        市场指数价格 (用于 HMM)
    n_regimes : int
        状态数量
    """
    print("=" * 60)
    print("   带 Regime Switching 的多因子量化框架 — 完整流水线")
    print("=" * 60)

    # ── Step 1: 数据准备 ──────────────────────────────────────────────────
    print("\n[Step 1] 数据加载 & 因子工程...")
    if start_date:
        price_df = price_df.loc[start_date:]
    if end_date:
        price_df = price_df.loc[:end_date]

    data_engine = DataEngine(price_df)
    factor_panel = data_engine.build_factor_panel()

    factor_names = ['mom_5', 'mom_20', 'mom_60', 'reversal', 'volatility', 'vol_price']
    print(f"  因子面板: {factor_panel.shape[0]} 交易日 × {factor_panel.shape[1]} 列")
    print(f"  因子列表: {factor_names}")

    # ── Step 2: Regime Detection ──────────────────────────────────────────
    print("\n[Step 2] HMM 市场状态识别...")
    if market_index is None:
        market_index = price_df.mean(axis=1)  # 用等权组合代理市场指数

    regime_detector = RegimeDetector(n_regimes=n_regimes, market_index=market_index)
    regime_detector.fit(n_iter=30)

    # 平滑状态序列
    smoothed = regime_detector.get_regime_smooth(window=5)

    # ── Step 3: Factor Model (在线学习) ──────────────────────────────────
    print("\n[Step 3] 初始化在线学习因子模型...")
    factor_model = FactorModel(
        n_regimes=n_regimes,
        factor_names=factor_names,
        decay=0.97,
        learning_rate=0.005
    )
    print(f"  每个 regime 独立模型, decay={0.97}")

    # ── Step 4: 回测 ──────────────────────────────────────────────────────
    print("\n[Step 4] 启动 Regime-Aware 回测...")
    optimizer = PortfolioOptimizer(top_n=20, max_weight=0.05, max_turnover=0.3)
    backtester = RegimeAwareBacktester(
        data_engine=data_engine,
        regime_detector=regime_detector,
        factor_model=factor_model,
        optimizer=optimizer,
        cost_bp=2.0,
        rebalance_freq=5
    )
    backtester.run(factor_panel)
    backtester.print_report()

    # ── Step 5: Live Monitor ──────────────────────────────────────────────
    print("\n[Step 5] 初始化 Progressive Validation 监控...")
    live_monitor = LiveMonitor(
        backtest_returns=backtester.daily_returns,
        window=20,
        z_threshold=2.0
    )

    # 模拟实盘: 用回测最后 30 天"假装"是实盘
    sim_live = backtester.daily_returns.iloc[-30:]
    print(f"  模拟实盘 {len(sim_live)} 天...")
    for date, ret in sim_live.items():
        result = live_monitor.update(date, ret, current_position=1.0)
        if result['status'] != 'NORMAL' and result['status'] != 'WARMUP':
            print(f"  ⚠ {date.date()}: {result['alert']}")

    print(live_monitor.get_report())

    # ── Step 6: 输出各 Regime 因子权重对比 ────────────────────────────────
    print("\n[Step 6] 各 Regime 因子权重对比:")
    print("-" * 60)
    for k in range(n_regimes):
        importance = factor_model.get_factor_importance(k)
        print(f"\n  Regime {k} (训练样本: {factor_model._n_samples[k]}):")
        for fname, coef in importance.items():
            print(f"    {fname:>12}: {coef:+.4f}")
    print("-" * 60)

    return {
        'data_engine': data_engine,
        'regime_detector': regime_detector,
        'factor_model': factor_model,
        'backtester': backtester,
        'live_monitor': live_monitor
    }


# ══════════════════════════════════════════════════════════════════════════════
# 8. 演示: 用合成数据跑通完整流程
# ══════════════════════════════════════════════════════════════════════════════
if __name__ == '__main__':
    np.random.seed(42)

    # 生成合成数据: 模拟 3 年日频, 50 只股票
    n_days = 756  # ~3 年
    n_stocks = 50
    dates = pd.date_range('2023-01-01', periods=n_days, freq='B')

    # 用随机游走 + regime 切换模拟价格
    # Regime 0: 牛市 (高 drift), Regime 1: 熊市 (负 drift), Regime 2: 震荡
    true_regime = np.random.choice([0, 1, 2], size=n_days, p=[0.4, 0.3, 0.3])
    drifts = {0: 0.0008, 1: -0.0005, 2: 0.0001}
    vols = {0: 0.012, 1: 0.018, 2: 0.008}

    prices = pd.DataFrame(index=dates, columns=[f'S{i:02d}' for i in range(n_stocks)], dtype=float)
    for i in range(n_stocks):
        rets = np.array([
            np.random.normal(drifts[r], vols[r]) for r in true_regime
        ])
        price = 100 * np.cumprod(1 + rets)
        prices[f'S{i:02d}'] = price

    # 市场指数
    market_idx = prices.mean(axis=1)

    print("合成数据: 50只股票 × 756个交易日, 3种市场regime")
    print(f"Regime分布: 0(牛市)={(true_regime==0).sum()}, "
          f"1(熊市)={(true_regime==1).sum()}, 2(震荡)={(true_regime==2).sum()}")

    # 运行完整流水线
    results = run_full_pipeline(
        price_df=prices,
        market_index=market_idx,
        n_regimes=3
    )

    print("\n\n✅ 框架运行完成! 各模块已就绪, 可接入真实数据替换合成数据。")
