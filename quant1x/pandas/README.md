# pandas — 列容器规格层（Python / pandas）

## 职责与边界

`quant1x/pandas` 是**列容器的规格层**：用 pandas 承载 `Series` / `DataFrame` 的取值、取列、
写列、复制与逐元素运算符，把滚动、指数加权、归约转给 `quant1x.num`。

- 收：一列同构数据的容器、列选择与写入、算术/比较/逻辑运算符、`rolling` / `ewm` 的入口。
- 不收：数值算法（在 `quant1x.num`）、行情结构与复权（在 `quant1x.data.schema`）、
  IO、绘图、索引对齐规则（本包按位置语义，不按标签对齐）。

## 文件清单

| 文件 | 用途 |
| --- | --- |
| `series.py` | `Series`：一列数据，运算符与 `rolling` / `ewm` 入口 |
| `dataframe.py` | `DataFrame`：列容器，取列 / 写列 / 复制 |
| `rolling.py` | `Rolling`：委托 `quant1x.num.rolling_*` |
| `ewm.py` | `EWM`：委托 `quant1x.num.ewm_mean` |
| `pandas.md` | 跨语言口径与功能对照表 |
| `series.h` / `ewm.h` | C++ 侧 Series / EWM，命名空间已收敛到 `quant1x::pandas` |
| `dataframe.h` / `periods.h` / `rule.h` / `rule.cpp` | C++ 侧早期原型（`ta` 命名空间，待收敛） |
| `mod.rs` | Rust 侧模块声明（仅周期解析） |

## 与上下层的关系

- 上游：`quant1x.num`（全部数值算子）、pandas（容器）。
- 下游：`quant1x.indicators` 用 `DataFrame` 取 `open` / `close` / `high` / `low` / `volume` / `amount`。
- 兄弟模块：`quant1x.data.schema` 的 `Bar` 是对外数据契约；指标在边界处把列转成 `Bar` 或反之。

## 跨语言对应关系

| 语义 | python | c++ | rust | go | zig |
| --- | --- | --- | --- | --- | --- |
| Series | `series.py` | `series.h` | `series.rs` | `series.go` | `series.zig` |
| DataFrame | `dataframe.py` | `dataframe.h` | — | — | — |
| EWM | `ewm.py` | `ewm.h` | `series.rs` | `series.go` | `series.zig` |
| Rolling | `rolling.py` | `series.h` | `series.rs` | `series.go` | `series.zig` |

C++ 的 `Series` / `EWM` 已收敛到 `quant1x::pandas`；`dataframe.h` / `periods.h` / `rule.*`
仍是早期原型（`ta` 命名空间），按模块逐个收敛。
