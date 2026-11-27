# pandas —— 列容器：位置语义、无标签对齐

**边界**：本文件只描述容器层。数值口径（窗口、NaN、回归）以 `quant1x/num/num.md` 为准，
本文件不重复；行情结构以 `quant1x/data/schema/bar.md` 为准。

## 1. 路线图

| 阶段 | 语言（角色） | 载体 | 状态 | 说明 |
| --- | --- | --- | --- | --- |
| spec 定语义 | Python（spec） | `series.py` / `dataframe.py` / `rolling.py` / `ewm.py` | 已落地 | 运算符与容器 |
| 生产实现 | C++（生产） | `series.h` / `ewm.h` / `dataframe.h` | 部分落地 | Series/EWM 已在 `quant1x::pandas`；DataFrame 仍原型 |
| 对齐实现 | Rust / Go / Zig | — | 未落地 | 指标侧暂用各语言原生切片 |

## 2. 功能清单

**统一口径**：

- **位置语义**：本包所有运算按位置对齐，不按标签对齐；长度不齐时由 `quant1x.num`
  按第一个操作数对齐（短补 NaN、长截断），不做 pandas 的索引并集。
- **单值类型**：数值列 `float64`，比较/逻辑列 `bool`；不引入 nullable dtype。
- **复制语义**：`copy()` 深复制；运算符返回新对象，不原地改。
- **滚动 / 指数加权**：语义完全由 `quant1x.num` 决定，本层只做转发。

| 功能 | python | c++ | rust | go | zig | 状态 |
| --- | --- | --- | --- | --- | --- | --- |
| Series 构造与取值 | `series.py` | `series.h` | `series.rs` | — | — | 已对齐 python/c++/rust（go/zig 待补） |
| 算术运算符 | `series.py` | `series.h` | `series.rs` | — | — | 已对齐 python/c++/rust（go/zig 待补） |
| 比较 / 逻辑运算符 | `series.py` | `series.h` | `series.rs` | — | — | 已对齐 python/c++/rust（go/zig 待补） |
| shift / diff / cumsum | `series.py` | `series.h` | `series.rs` | — | — | 已对齐 python/c++/rust（go/zig 待补） |
| rolling 入口 | `rolling.py` | `series.h` | `series.rs` | — | — | 已对齐 python/c++/rust（go/zig 待补） |
| ewm 入口 | `ewm.py` | `ewm.h` | `series.rs` | — | — | 已对齐 python/c++/rust（go/zig 待补） |
| DataFrame 取列 / 写列 | `dataframe.py` | `dataframe.h` | — | — | — | 部分落地（c++ 原型） |

## 3. 依赖

**出向**

| 依赖 | 用途 | 为什么必须是它 |
| --- | --- | --- |
| pandas | 底层容器 | 仓库 Python 侧既有依赖 |
| `quant1x.num` | 全部数值算子 | 容器层不得另写一套口径 |

**入向**

| 使用方 | 用途 | 反向约束 |
| --- | --- | --- |
| `quant1x.indicators` | 读取 OHLCV 列、写入指标列 | 只能按列名访问，不依赖索引 |

## 4. 测试

| 维度 | 覆盖 |
| --- | --- |
| 单元测试 | `tests/test_pandas_facade.py`：运算符、位置对齐、`rolling` / `ewm` 转发、列写入 |
| 跨语言对拍 | 经 `quant1x.formula` / `quant1x.indicators` 的 fixture 间接对拍 |
| 并发 / 崩溃注入 | 本模块不涉及（无共享状态、无持久化） |

## 5. TODO

- C++ 侧 `series.h` / `dataframe.h` 仍用 `ta` 命名空间，与 §4.4 的 `quant1x::pandas` 不一致；
  等指标 C++ 实现落地后一并收敛。
- Rust / Go / Zig 侧暂不设容器层：指标直接接收 `Bar` 切片，避免为一个用例造一套 DataFrame。
