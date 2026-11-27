# num —— 数值规格层：可移植的窗口、NaN 与回归口径

**边界**：本包是 `quant1x.formula` / `quant1x.indicators` 的数值底座，只处理一维 `float64`
序列的算子语义；**不**定义行情结构、列对齐、时间索引与 IO。容器在 `quant1x.pandas`，
行情结构在 `quant1x.data.schema`。

**定位**：Python 是 spec；C++ 与 Rust 在本目录内按 §2 的口径实现同语义的对齐实现。
跨语言一致靠 `tests/fixtures/num/cases.csv` 的逐位比较，不靠代码生成或 FFI。C++ / Rust / Go
都在本目录内按 §2 口径对齐实现；Zig 为扩展，当前只承载小功能。

## 1. 路线图

| 阶段 | 语言（角色） | 载体 | 状态 | 说明 |
| --- | --- | --- | --- | --- |
| spec 定语义 | Python（spec） | `quant1x/num/*.py` | 已落地 | NumPy 承载，归约顺序与 NaN 口径固定 |
| 生产实现 | C++（生产） | `quant1x/num/*.h` | 已落地 | `std::vector` + 显式循环，逐位对拍通过 |
| 对齐实现 | Rust | `quant1x/num/*.rs` | 已落地 | `Vec<f64>` + 显式循环 |
| 对齐实现 | Go | `quant1x/num/*.go` | 已落地 | `[]float64` + 显式循环；已移除对 `github.com/quant1x/num` 的依赖 |
| 扩展 | Zig | — | 不设 | 当前只承载小功能 |

`quant1x/formula/formula.h` 仍就地实现公式所需算子；它迁移到本层后删除重复实现。
本包不为「以后可能用到」预置扩展点。

## 2. 功能清单

**统一口径**（四门语言必须一致）：

- **数值类型**：一律 `float64`；整型 / bool / `pandas.Series` 在边界转换一次。
- **缺失值**：只有 IEEE-754 NaN；判断用 `is_nan`，不写 `x == nan`。
- **归约顺序**：从左到右顺序累加，不用 NumPy 的成对求和，也不用滚动的增量加减。
- **空输入**：`sum` / `variance` / `std` / `covariance` 为 `0.0`；`mean` / `median` /
  `min` / `max` 为 NaN。
- **滚动窗口**：完整窗口才计算；`w <= 0` 或 `w > i + 1` 为 NaN；窗口内含 NaN 为 NaN。
- **指数加权**：`y_t = (1-alpha)·y_{t-1} + alpha·x_t`，首值作种子；NaN 跳过、输出沿用前值，
  首个有效值之前为 NaN。
- **回归**：均值中心化最小二乘闭式解；窗口小于 2 或含 NaN 为 NaN。
- **小数位**：半程远离零，NaN 按 0 参与舍入。
- **二元对齐**：加减乘除与比较按第一个操作数的长度对齐，第二个短补 NaN、长截断。

| 功能 | python | c++ | rust | go | zig | 状态 |
| --- | --- | --- | --- | --- | --- | --- |
| 类型与缺失值 | `dtype` | `dtype.h` | `dtype.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 窗口归一化 | `window.as_window` / `window_at` | `window.h` | `window.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 绝对值 / 平方根 / 对数 / 幂 | `elementwise.abs` / `sqrt` / `log` / `pow` | `elementwise.h` | `elementwise.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 加减乘除 | `elementwise.add` / `sub` / `mul` / `div` | `elementwise.h` | `elementwise.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 序列与标量 / 取大 / 取小 / 条件选择 | `elementwise.*_scalar` / `maximum` / `minimum` / `where` | `elementwise.h` | `elementwise.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 比较 | `logic.gt` / `gte` / `lt` / `lte` / `equal` / `not_equal` | `logic.h` | `logic.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 逻辑与 / 或 / 非 / 数值转布尔 | `logic.logic_and` / `logic_or` / `logic_not` / `to_bools` | `logic.h` | `logic.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 全真 / 任一 / 计数 | `logic.all_of` / `any_of` / `count_true` | `logic.h` | `logic.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 求和 / 均值 / 中位数 | `reduce.sum_value` / `mean` / `median` | `reduce.h` | `reduce.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 极值与下标 | `reduce.min_value` / `max_value` / `argmin` / `argmax` | `reduce.h` | `reduce.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 方差 / 标准差 / 协方差 | `reduce.variance` / `std` / `covariance` | `reduce.h`（`stddev`） | `reduce.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 滚动框架 | `rolling.rolling` / `rolling_apply` | `rolling.h` | `rolling.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 滚动求和 / 均值 / 标准差 | `rolling.rolling_sum` / `rolling_mean` / `rolling_std` | `rolling.h` | `rolling.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 滚动极值与位置 | `rolling.rolling_min` / `rolling_max` / `rolling_hhvbars` / `rolling_llvbars` | `rolling.h` | `rolling.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 滚动平均绝对偏差 | `rolling.rolling_avedev` | `rolling.h` | `rolling.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 指数加权均值 | `ewm.ewm_mean` | `ewm.h` | `ewm.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 移位 / 引用 | `shift.shift` / `ref` | `shift.h` | `shift.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 差分 / 累计和 | `shift.diff` / `cumsum` | `shift.h` | `shift.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 缺失值填充 | `fillna.fill` / `fill_na` / `is_na` | `fillna.h` | `fillna.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 对齐 / 平铺 | `align.align` / `pad_to` / `repeat` | `align.h` | `align.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 一元线性回归 | `regression.linear_regression` / `slope` / `forcast` | `regression.h` | `regression.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 小数位 / 变化率 | `decimals.decimal` / `change_rate` / `net_change_rate` | `decimals.h` | `decimals.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 形状与构造 | `matrix.shape` / `concat1d` / `transpose2d` / `zeros` / `ones` / `arange` / `linspace` | `matrix.h` | `matrix.rs` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |

C++ 与 Python 的命名差异只有一处：`reduce.std` → `stddev`（`std` 会遮蔽 `std::` 命名空间）。
同时提供 Rust 关键字规避名 `where_select` / `ref_value` 的对齐别名。

## 3. 依赖

**出向**

| 依赖 | 用途 | 为什么必须是它 |
| --- | --- | --- |
| NumPy | 数组容器与逐元素运算 | 仓库 Python 侧既有依赖，无额外引入 |
| STL（`std::vector` / `<cmath>`） | C++ 侧容器与标量函数 | 本层是最低数值底座，不引入 xtensor，避免其比较运算符与广播语义 |
| `std` 标准库 | Rust 侧容器与标量函数 | 同上，`Vec<f64>` + 显式循环 |

**入向**

| 使用方 | 用途 | 反向约束 |
| --- | --- | --- |
| `quant1x.formula` | 全部公式函数的数值算子 | 只能用本文件的 §2 口径 |
| `quant1x.indicators` | 指标的滚动、回归、填充 | 同上 |
| `quant1x.pandas` | 底层数值算子 | 同上，不另写一份 |

## 4. 测试

| 维度 | 覆盖 |
| --- | --- |
| 单元测试 | `tests/test_num.py`：窗口不足、窗口含 NaN、空输入、除零、回归退化、小数位半程远离零 |
| 跨语言对拍 | `tests/fixtures/num/generate.py` 由 Python spec 生成 163 组样本；`tests/tdd-num-parity.cpp`（C++）、`quant1x/num/num_parity_test.go`（Go）逐位比较（NaN 视为相等）；CMake / Meson / `go test` 均已登记。Go 的 `math.Log` / `math.Pow` 与 NumPy 相差至多 1 ULP，这两类按相对误差 1e-12 比较，纯算术与归约逐位相等 |
| 静态检查 | 仓库既有 ruff 配置；C++ 头文件经 `app-tdd-num-parity` 编译验证 |
| 基准测试 | 本模块不涉及；性能由调用方模块各自度量 |
| 崩溃注入 / 并发 | 本模块不涉及（纯函数、无共享状态） |

## 5. TODO

- 本包按 `quant1x.formula` 的实际用量增长；`argmin` / `argmax` 当前只服务调试与文档，
  未进入指标路径。
- 未提供多维数组的广播语义：公式与指标只处理一维序列，二维需求由调用方在
  `quant1x.pandas` 侧组织。
