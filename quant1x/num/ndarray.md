# ndarray —— 对标 `np.ndarray` 的可移植一维容器

**numpy 就是 spec。** 本模块把「一维数值数组」收敛成一个方法集合固定的容器 `NDArray`：
Python 用 numpy 承载（存储与互转），C++ / Rust / Go / Zig 按同一张方法表用显式循环实现，
让研究代码能落到生产。

- **契约以命名方法为准**：Go 没有运算符重载，其它语言只需照抄方法表；Python 额外提供运算符
  （`+` / `>` / `&` …）作为糖，一一映射到方法（见下表）。
- 数值口径（窗口、NaN、归约顺序、回归、舍入）全部委托 `quant1x.num` 的自由函数，容器本身不写算法。
- 只支持一维 `float64` / `bool`；不做 N 维广播、花式索引、复数/字符串 dtype。

## 1. 方法表（跨语言契约）

| 分类 | 方法 | Python 运算符糖 |
| --- | --- | --- |
| 构造 | `NDArray(data)` / `zeros` / `ones` / `full` / `arange` / `linspace` | — |
| 属性 | `size` / `shape` / `ndim` / `dtype` / `values` / `to_numpy` / `to_list` | — |
| 序列 | `len` / 迭代 / `getitem`（int 或 slice） / `setitem` / `slice` | `[]` |
| 复制/转换 | `copy` / `astype` | — |
| 四则 | `add` / `sub` / `mul` / `div` | `+` `-` `*` `/`（含反向） |
| 极值 | `maximum` / `minimum` | — |
| 一元 | `abs` / `sqrt` / `log` / `pow` | `-` |
| 比较 | `gt` / `gte` / `lt` / `lte` / `equal` / `not_equal` | `>` `>=` `<` `<=` `==` `!=` |
| 逻辑 | `logic_and` / `logic_or` / `logic_not` | `&` `\|` `~` |
| 归约 | `sum` / `mean` / `median` / `min` / `max` / `argmin` / `argmax` / `var` / `std` / `cumsum` | — |
| 移位/差分 | `shift` / `diff` / `fill_na` | — |
| 滚动 | `rolling_sum` / `rolling_mean` / `rolling_std` / `rolling_min` / `rolling_max` / `rolling_avedev` / `rolling_apply` | — |
| 指数加权 | `ewm_mean` | — |

`add` 等方法接受 `NDArray` 或数值标量；标量参与比较时按序列长度广播。

## 2. 语义边界

- **dtype**：只有 `float64` 与 `bool`。布尔数组用于比较/逻辑结果；参与算术时按 0/1 收敛。
- **对齐**：二元算子按**第一个操作数**长度对齐（第二个短补 NaN、长截断），与 `quant1x.num`
  的自由函数一致；标量按长度广播。
- **滚动 / 移位**：完整窗口才计算、窗口含 NaN 即 NaN、移位越界补 NaN，全部沿用 `num` 口径。
- **错误**：非一维输入、非法 dtype 抛 `ValueError`。

## 3. 跨语言状态

| 语言 | 载体 | 状态 |
| --- | --- | --- |
| Python（spec） | `ndarray.py` | 已落地（numpy 承载） |
| C++（生产） | `ndarray.h` | 已落地（`tests/tdd-num-ndarray.cpp` 6 用例） |
| Rust（生产） | `ndarray.rs` | 已落地（`tests/ndarray_test.rs` 6 用例） |
| Go | `ndarray.go` | 待补 |
| Zig | `ndarray.zig` | 待补 |

## 4. 测试

| 维度 | 覆盖 |
| --- | --- |
| 单元测试 | `tests/test_ndarray.py`：构造/属性、四则与一元、比较与逻辑、归约、移位/差分、滚动、EWM、切片/填充/类型转换 |
| 跨语言对拍 | 后续由 Python 生成 fixture，各语言读同一份逐值比较 |
