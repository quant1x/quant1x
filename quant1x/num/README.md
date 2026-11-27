# num — 数值规格层（多语言对齐）

## 职责与边界

`quant1x/num` 是**数值语义的规格层**：Python（NumPy）定语义，C++ / Rust 在同目录内按同一口径
对齐实现，把「窗口不足怎么算、NaN 怎么传播、回归用哪种公式」收敛到一处，供 `quant1x.formula`
与 `quant1x.indicators` 调用。

- 收：一维数值序列的逐元素运算、比较与逻辑、归约、滚动窗口、指数加权、移位/差分/累计、
  缺失值填充、对齐与重复、一元线性回归、小数位、形状与构造。
- 不收：行情结构（`quant1x.data.schema`）、列对齐与 DataFrame 容器（`quant1x.pandas`）、
  IO 与时间索引。

Python 是 spec；C++ / Rust 只用标准库容器与显式循环，不跨语言调用、不从本包生成代码。
跨语言一致靠同一组 fixture 逐位比较（见 `num.md` §4）。

## 文件清单

同一语义按 `模块.扩展名` 平铺，Python / C++ / Rust 一一对应：

| 模块 | python | c++ | rust | 用途 |
| --- | --- | --- | --- | --- |
| 类型 | `dtype.py` | `dtype.h` | `dtype.rs` | 唯一数值类型与缺失值判定 |
| 窗口 | `window.py` | `window.h` | `window.rs` | 固定窗口与逐位置窗口归一化 |
| 逐元素 | `elementwise.py` | `elementwise.h` | `elementwise.rs` | 绝对值、平方根、对数、幂、加减乘除、取大取小、条件选择 |
| 逻辑 | `logic.py` | `logic.h` | `logic.rs` | 比较、逻辑与/或/非、全真/任一/计数 |
| 归约 | `reduce.py` | `reduce.h` | `reduce.rs` | 求和、均值、中位数、极值、下标、总体方差/标准差/协方差 |
| 滚动 | `rolling.py` | `rolling.h` | `rolling.rs` | 滚动框架与求和/均值/标准差/极值/平均绝对偏差/极值位置 |
| 指数加权 | `ewm.py` | `ewm.h` | `ewm.rs` | 通达信 `EMA` / `SMA` 的底层递推 |
| 移位 | `shift.py` | `shift.h` | `shift.rs` | 移位、引用、差分、累计和 |
| 缺失值 | `fillna.py` | `fillna.h` | `fillna.rs` | 缺失值填充与判定 |
| 对齐 | `align.py` | `align.h` | `align.rs` | 按目标长度对齐、平铺 |
| 回归 | `regression.py` | `regression.h` | `regression.rs` | 均值中心化最小二乘：斜率与预测值 |
| 小数位 | `decimals.py` | `decimals.h` | `decimals.rs` | 半程远离零的小数位、变化率 |
| 形状 | `matrix.py` | `matrix.h` | `matrix.rs` | 形状、一维拼接、二维转置、全零/全一、等差/等分序列 |
| 汇总 | — | `num.h` | `mod.rs` | 汇总头 / 模块入口与再导出 |
| 口径 | `num.md` | — | — | 跨语言口径与功能对照表 |

## 与上下层的关系

- 上游：Python 只依赖 NumPy；C++ / Rust 只用标准库。
- 下游：`quant1x.formula`（公式函数）、`quant1x.indicators`（指标）。
- 兄弟模块：`quant1x.pandas` 负责容器与列对齐，数值算子仍委托本包，避免两套口径。

## 跨语言对应关系

| 语义 | python | c++ | rust | go | zig |
| --- | --- | --- | --- | --- | --- |
| 数值算子 | `quant1x/num/*.py` | `quant1x/num/*.h` | `quant1x/num/*.rs` | `github.com/quant1x/num` | — |

四种语言的口径以 `num.md` §2 为准；Go 复用组织级 num 库，Zig 当前只承载小功能。
