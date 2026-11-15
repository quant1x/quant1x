# learn/scoring 加权评分

## 职责与边界

- **收**：多维度得分按权重合成 0-100 总分（`ScoreCalculator`），以及把原始指标折算成 0-100 分的处理器（`RangeProcessor` / `ThresholdProcessor`）。
- **不收**：指标本身的计算（归 `indicators/`、`ta/`、`factors/`）、因子标准化（归 `learn/preprocessing/`）、行情与数据源访问。

本目录只有纯数值计算，不 import 本仓其它模块，可被任意策略与工具复用。语义冻结：只修缺陷，不在这里新增语义。

## 文件清单

| 文件 | 用途 |
| --- | --- |
| `scoring.go` | `ScoreComponent` / `ScoreCalculator`：`AddComponent` / `CalculateWeightedScore` / `WeightDistribution` |
| `processors.go` | `RangeProcessor`（区间线性映射）、`ThresholdProcessor`（阈值比例折算） |
| `scoring_test.go` | 单元测试：权重分配、错误分支、零宽区间、零/负阈值 |
| `scoring.md` | 模块档案：公式、数据契约、边界与异常 |

## 与上下层的关系

- 上游：仅标准库（`errors`、`fmt`、`math`）。
- 下游：策略、选股、风控评估等需要综合分的调用方。
- 兄弟模块：`learn/preprocessing`（标准化）、`learn/cluster`（聚类）、`factors/safety_score.*`（个股东风险分，可作为本模块的维度来源）。

## 跨语言对应

| 语言 | 文件 | 状态 |
| --- | --- | --- |
| Go | `quant1x/learn/scoring/scoring.go`、`processors.go` | 已落地 |
| Python | `quant1x/learn/scoring/scoring.py` | 未落地 |
| C++ | `quant1x/learn/scoring/scoring.h` + `scoring.cpp` | 未落地 |
| Rust | `quant1x/learn/scoring/scoring.rs` | 未落地 |

Go 包名为 `scoring`，与目录名一致；导出符号按 Go 规范用 PascalCase，语义名与文档、其它语言的 `snake_case` 名对齐。

## 验证

```bash
cd quant1x
go test ./quant1x/learn/scoring/
```
