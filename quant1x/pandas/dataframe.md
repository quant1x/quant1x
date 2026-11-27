# DataFrame —— 可移植的多列容器

**pandas 就是 spec。** `DataFrame` 只承担「按列名存取、子集、复制」，列对齐与数值语义在
`Series` 与 `quant1x.num`；容器本身不做计算。

## 1. 方法表（跨语言契约）

| 分类 | 方法 | 说明 |
| --- | --- | --- |
| 构造 | `DataFrame()` / `DataFrame(columns)` | Python 另接受 `dict` / `pandas.DataFrame` |
| 属性 | `columns` / `shape` / `len` | 列名按插入序；`shape` 为 `(行数, 列数)` |
| 取列 | `get(name)` | 返回 `Series`；不存在返回空 |
| 子集 | `select(names)` | 按给定列序返回新表 |
| 写列 | `set(name, series)` | 列已存在则原位替换，否则追加到末尾 |
| 复制 | `copy()` | 深拷贝 |
| 互转 | `to_pandas()` | 仅 Python（其它语言无 pandas） |

Python 的 `df[name]` / `df[name] = value` / `df[name_list]` 是 `get` / `set` / `select` 的运算符糖；
其它语言直接调命名方法。

## 2. 语义边界

- **列类型**：只承载 `float64` / `bool` 的 `Series`，与 `Series` 的 dtype 一致。
  字符串等非数值列不在可移植契约内（见 §4）。
- **列对齐**：所有列必须等长；`set` 写入长度不一致时拒绝并报错（Python 侧 pandas 会补 NaN，
  可移植实现不做隐式补齐，避免各语言口径分叉）。
- **列序**：按插入顺序保存，`select` 按调用方给出的顺序输出。
- **空表**：无列时行数为 0；首列决定行数。

## 3. 跨语言状态

| 语言 | 载体 | 状态 |
| --- | --- | --- |
| Python（spec） | `dataframe.py` | 已落地（pandas 承载） |
| Rust（生产） | `dataframe.rs` | 已落地（`tests/dataframe_test.rs`） |
| Go | `dataframe.go` | 已落地（`quant1x/pandas/dataframe_test.go`） |
| C++（生产） | `dataframe.h` | 原型，待迁移（见 §4） |
| Zig | `dataframe.zig` | 已落地（`quant1x/pandas/dataframe_test.zig`） |

## 4. C++ 的现状与差异

`quant1x/pandas/dataframe.h` 是异构列原型：列类型为 `std::variant<std::vector<int>,
std::vector<int64_t>, std::vector<float>, std::vector<double>, std::vector<bool>,
std::vector<std::string>>`，另有 `from_struct_vector`（Boost.PFR 反射结构体字段）与
`to_string()` 表格打印；`quant1x/factors/history.cpp` 用它把 `Bar` 切片转成表并按列取数。

它与本契约的差别有二：

1. 列不是 `Series`，无法复用滚动/归约算子；
2. 支持字符串列（`Bar.date` / `Bar.timestamp`），本契约不覆盖。

迁移方向：数值列改为 `Series`，字符串列保留为独立的展示列（或把 `date` 收敛成数值时间戳），
再把 `factors/history.cpp` 的调用点切过去。迁移前 C++ 侧继续用现有原型。

## 5. 测试

| 维度 | 覆盖 |
| --- | --- |
| 单元测试 | Python `tests/test_pandas_facade.py`；Rust `tests/dataframe_test.rs`；Go `quant1x/pandas/dataframe_test.go` |
| 覆盖点 | 构造与列序、取列、子集、写入替换/追加、列长校验、深拷贝 |
