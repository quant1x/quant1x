# indicators — 通达信指标（多语言平行实现）

## 职责与边界

`quant1x/indicators` 把 `quant1x.formula` 的公式函数组合成可直接用于行情的指标：

| 指标 | 含义 | 主要输出列 |
| --- | --- | --- |
| `cdtd` | 抄底逃顶 | `cdtd.wen` / `cdtd.S1` / `cdtd.B` / `cdtd.S2` / `cdtd.S3` |
| `f89k` | 89K 图形 | `f89k.zs` / `f89k.b` / `f89k.DN` |
| `hjk` | 黄金坑 | `hjk.b` |
| `ma1x` | 89K 趋势策略线 | `DN` / `ZS` / `YL` / `ma1x.hc` / `ma1x.tp` |
| `p2fb` | 趋势反包 | `p2fb.hgzx` |

- 收：指标定义、列命名、对拍口径。
- 不收：单根 K 线的结构（`quant1x.data.schema.Bar`）、公式算子（`quant1x.formula`）。
- `winner` 只有公式说明（`winner.md`），Python 侧尚无实现，不参与对拍。

## 文件清单

| 文件 | 用途 |
| --- | --- |
| `cdtd.py` / `cdtd.h` | 抄底逃顶：Python spec 与 C++ 实现 |
| `f89k.py` / `f89k.h` | 89K 图形 |
| `hjk.py` / `hjk.h` | 黄金坑 |
| `ma1x.py` / `ma1x.h` | 89K 趋势策略线 |
| `p2fb.py` / `p2fb.h` | 趋势反包 |
| `support.h` | C++ 公共前置：从 `Bar` 序列取列 |
| `*.md` | 每个指标的公式原文与口径 |
| `README.md` | 本文件：目录索引与跨语言状态 |

## 跨语言状态

| 指标 | python | c++ | rust | go | zig | 状态 |
| --- | --- | --- | --- | --- | --- | --- |
| `cdtd` | `cdtd.py` | `cdtd.h` | `mod.rs` | — | — | 已对齐 c++/rust（缺 go / zig） |
| `f89k` | `f89k.py` | `f89k.h` | `mod.rs` | — | — | 已对齐 c++/rust（缺 go / zig） |
| `hjk` | `hjk.py` | `hjk.h` | `mod.rs` | — | — | 已对齐 c++/rust（缺 go / zig） |
| `ma1x` | `ma1x.py` | `ma1x.h` | `mod.rs` | — | — | 已对齐 c++/rust（缺 go / zig） |
| `p2fb` | `p2fb.py` | `p2fb.h` | `mod.rs` | — | — | 已对齐 c++/rust（缺 go / zig） |

## 统一口径

- **输入**：Python 接收 `pandas.DataFrame`（列 `open` / `close` / `high` / `low` / `volume` /
  `amount` / `date`）；C++ 接收 `std::vector<quant1x::data::schema::Bar>`。
- **C++ 写法**：容器用 xtensor（`formula::Series` / `formula::Bools`），加减乘除、比较直接用
  运算符，逐行对照 Python；相等比较用 `equal` / `equal_scalar`（xtensor 的 `==` 是整表比较），
  布尔组合用 `&&` / `||` / `!`（`&` / `|` 在 `XTENSOR_USE_XSIMD` 下会触发 `bitwise_and` 的
  SIMD 实例化错误，MSVC 编不过）。
- **输出**：按上表的列名逐列对齐；布尔列在 fixture 里落成 `0 / 1`，C++ 侧同。
- **涨停比例**：Python 侧由证券代码经 `quant1x.data.market.limit_up_ratio` 推导；C++ 侧因
  `data.market` 的规则引擎尚未跨语言，`hjk` / `p2fb` 把比例作为显式入参，由调用方提供。
  同一比例（如主板 0.10）下两边数值完全一致。
- **对拍**：`tests/fixtures/formula/indicators/` 由 `tests/fixtures/formula/generate.py`
  生成（120 根合成 K 线），C++ 用 `tests/tdd-indicators.cpp` 逐值比对，NaN 位置也必须一致。

# 通达信指标迁移指南

## 1. 逻辑运算

指标迁移中 pandas 不能使用 Python 的逻辑（位）运算符，否则会产生歧义：

| 逻辑 | 通达信 | python |
|:---|:----|:-------|
| 与 | AND | `&` |
| 或 | OR  | `\|` |
| 非 | NOT | `~` |

## 2. 括号

逻辑运算符左右两侧非单一 Series 时必须用括号。
