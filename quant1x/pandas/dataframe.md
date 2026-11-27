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
| C++（生产） | `dataframe.h` | 已落地（`tests/tdd-dataframe-release.cpp`） |
| Zig | `dataframe.zig` | 已落地（`quant1x/pandas/dataframe_test.zig`） |

## 4. 字符串列（展示扩展）

`Bar` 有 `date` / `timestamp` 两个字符串字段，打印表格时需要它们，但可移植契约只覆盖
`float64` / `bool` 的 `Series`。C++ 因此额外承载**字符串列**（对照 pandas 的 object dtype）：

| 方法 | 说明 |
| --- | --- |
| `set_strings(name, vector<string>)` | 写展示列 |
| `get_strings(name)` | 取展示列；不是字符串列时返回 `nullptr` |

字符串列不参与任何数值算子，也不参与 `get`（`get` 只返回 `Series`）。
`from_struct_vector` 会按字段类型自动分流：算数字段（含 `int` / `float`）→ `Series(double)`，
`bool` → `Series(bool)`，`std::string` → 字符串列，其它类型跳过。
Rust / Go / Zig 目前只实现数值列；需要打印带字符串的表时再按同一口径补。

## 5. 打印

`to_string()` 对齐 pandas 的表格版式：

- 索引列 + 每列右对齐（pandas 默认 `justify='right'`），列间两个空格；
- 浮点按列统一小数位（至少 1 位），如 `10.0` / `10.50`；
- 量级在 `[1e-4, 1e16)` 外走科学计数法，`NaN` / `inf` 用 pandas 的写法；
- 行数超过 60 时显示首尾各 5 行，中间用 `..`（索引列）/ `...`（数据列）省略，
  末尾附 `[N rows x M columns]`；未截断时没有页脚；
- 空表输出 `Empty DataFrame` / `Columns: []` / `Index: []`。

## 6. 测试

| 维度 | 覆盖 |
| --- | --- |
| 单元测试 | Python `tests/test_pandas_facade.py`；C++ `tests/tdd-dataframe-release.cpp`；Rust `tests/dataframe_test.rs`；Go `quant1x/pandas/dataframe_test.go`；Zig `quant1x/pandas/dataframe_test.zig` |
| 覆盖点 | 构造与列序、取列、子集、写入替换/追加、列长校验、深拷贝、结构体反射构造、打印版式与截断 |
