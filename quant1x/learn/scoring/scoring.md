# scoring 加权评分

按维度给分，再按权重把多个维度合成为一个 0-100 总分。纯数值计算，不依赖行情与数据源。

## 1. 业务定义

选股、风控、策略评估等场景的最后一环：各维度已经算出 0-100 的分数，需要一个可比较的综合分。本模块提供两件事：

- **合成**：`ScoreCalculator` 把多个维度得分按权重加总；
- **折算**：`RangeProcessor` / `ThresholdProcessor` 把原始指标折算成 0-100 分，供合成使用。

## 2. 模块关系

| 方向 | 内容 |
| --- | --- |
| 上游依赖 | 无（只用标准库 `errors` / `fmt` / `math`） |
| 下游调用方 | 策略、选股、风控评估等需要综合分的场景 |
| 兄弟模块 | `learn/preprocessing`（数值标准化）、`learn/cluster`（聚类）同处 `learn/` |

跨语言对应（同一语义）：

| 语言 | 文件 | 状态 |
| --- | --- | --- |
| Go | `quant1x/learn/scoring/scoring.go`、`processors.go` | 已落地 |
| Python | `quant1x/learn/scoring/scoring.py` | 未落地 |
| C++ | `quant1x/learn/scoring/scoring.h` + `scoring.cpp` | 未落地 |
| Rust | `quant1x/learn/scoring/scoring.rs` | 未落地 |

## 3. 数学模型

**加权合成**：第 $i$ 个维度得分 $s_i \in [0,100]$，权重 $w_i$，总分

$$
S = \sum_{i=1}^{n} w_i \, s_i, \qquad \sum_{i=1}^{n} w_i = 1
$$

**权重分配**：设自定义权重的维度集合为 $C$，自动分配的维度个数为 $n_{auto}$，则

$$
w_{auto} = \frac{1 - \sum_{i \in C} w_i}{n_{auto}}
$$

自定义权重的维度取 $w_i$ 为其给定值，其余维度取 $w_{auto}$。约束 $\sum_{i \in C} w_i \le 1$。

**区间折算**（`RangeProcessor`，区间 $[m, M]$）：

$$
f(x) = \begin{cases}
0, & x < m \\
\dfrac{x - m}{M - m} \cdot 100, & m \le x \le M \\
100, & x > M
\end{cases}
$$

**阈值折算**（`ThresholdProcessor`，阈值 $T > 0$）：

$$
f(x) = \begin{cases}
\dfrac{x}{T} \cdot 100, & x < T \\
100, & x \ge T
\end{cases}
$$

## 4. 数据契约

| 结构 | 字段 | 类型 | 取值 |
| --- | --- | --- | --- |
| `ScoreComponent` | `Name` | `string` | 维度名称，同时作为权重分布的键 |
| | `CustomWeight` | `float64` | 自定义权重，仅 `IsAutoWeight=false` 时有效，$\ge 0$ |
| | `Score` | `float64` | 维度得分，$0 \le s \le 100$ |
| | `IsAutoWeight` | `bool` | 是否参与自动分配权重 |
| `RangeProcessor` | `Min` / `Max` | `float64` | 区间下界 / 上界 |
| `ThresholdProcessor` | `Threshold` | `float64` | 阈值，必须 $> 0$ |

`Process` 只接受 `float64` 入参，其余类型一律返回错误；不存在 NaN / Null 语义，调用方自行处理缺失值。

## 5. 边界与异常

| 场景 | 处理 |
| --- | --- |
| `AddComponent` 得分越界（$<0$ 或 $>100$） | 返回错误，不加入评分项 |
| 自定义权重为负 | 返回错误 |
| `customWeight` 传入多于一个参数 | 返回错误 |
| 自定义权重合计 $> 1$ | `CalculateWeightedScore` 返回错误 |
| 无评分项 | 返回总分 0、权重分布为空，不报错 |
| 全部维度自定义权重 | 按给定权重合成，不做自动分配 |
| `RangeProcessor` 的 `Max < Min` | 返回错误 |
| `RangeProcessor` 的 `Max == Min`（零宽区间） | $x < Min$ 记 0 分，否则记 100 分，不做除法 |
| `ThresholdProcessor` 的 `Threshold <= 0` | 返回错误 |
| `Process` 入参类型不是 `float64` | 返回错误 |
| 同名维度重复添加 | 允许；权重分布按名称记录（后者覆盖），总分逐项累加 |

`WeightDistribution` 返回副本，调用方改动副本不影响内部状态。

## 6. 验证

```bash
cd quant1x
go test ./quant1x/learn/scoring/
```
