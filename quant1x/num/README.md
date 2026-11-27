# num — 数值规格层（Python / NumPy）

## 职责与边界

`quant1x/num` 是**数值语义的规格层**：用 NumPy 承载一组可移植的数值算子，把「窗口不足怎么算、
NaN 怎么传播、回归用哪种公式」收敛到一处，供 `quant1x.formula` 与 `quant1x.indicators` 调用。

- 收：一维数值序列的逐元素运算、比较与逻辑、归约、滚动窗口、指数加权、移位/差分/累计、
  缺失值填充、对齐与重复、一元线性回归、小数位、形状与构造。
- 不收：行情结构（`quant1x.data.schema`）、列对齐与 DataFrame 容器（`quant1x.pandas`）、
  IO 与时间索引。

本包**只有 Python 实现**：C++ / Rust / Go / Zig 侧的数值算子由各语言按 `num.md` 的同一口径
就地实现，不跨语言调用，也不从本包生成代码。

## 文件清单

| 文件 | 用途 |
| --- | --- |
| `dtype.py` | `DType` / `NAN` / `as_series` / `is_nan`：唯一数值类型与缺失值判定 |
| `window.py` | 固定窗口与逐位置窗口归一化为 `int64` 数组 |
| `elementwise.py` | 绝对值、平方根、对数、幂、加减乘除、取大取小、条件选择 |
| `logic.py` | 比较、逻辑与/或/非、全真/任一/计数 |
| `reduce.py` | 求和、均值、中位数、极值、下标、总体方差/标准差/协方差 |
| `rolling.py` | 滚动框架与求和/均值/标准差/极值/平均绝对偏差/极值位置 |
| `ewm.py` | 指数加权移动平均（通达信 `EMA` / `SMA` 的底层递推） |
| `shift.py` | 移位、引用、差分、累计和 |
| `fillna.py` | 缺失值填充与判定 |
| `align.py` | 按目标长度对齐、平铺 |
| `regression.py` | 均值中心化最小二乘：斜率与预测值 |
| `decimals.py` | 半程远离零的小数位、变化率 |
| `matrix.py` | 形状、一维拼接、二维转置、全零/全一、等差/等分序列 |
| `num.md` | 跨语言口径与功能对照表 |

## 与上下层的关系

- 上游：只依赖 NumPy；不依赖 pandas、不依赖 `quant1x` 其它子包。
- 下游：`quant1x.formula`（公式函数）、`quant1x.indicators`（指标）。
- 兄弟模块：`quant1x.pandas` 负责容器与列对齐，数值算子仍委托本包，避免两套口径。

## 跨语言对应关系

| 语义 | python | c++ | rust | go | zig |
| --- | --- | --- | --- | --- | --- |
| 数值算子 | `quant1x/num/*.py` | — | — | — | — |

各语言按 `num.md` §2 的口径在自模块内实现，不设置同名 `num` 目录。
