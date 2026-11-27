# formula — 通达信公式函数（多语言平行实现）

## 职责与边界

`quant1x/formula` 实现通达信 / 同花顺麦语言里的 0 级（`REF` / `HHV` / `MA` …）、1 级
（`COUNT` / `CROSS` / `BARSLAST` …）与 2 级（`MACD` / `KDJ` / `RSI` …）函数。

- 收：一维数值序列上的纯计算，输入是「序列 + 参数」，输出是「序列 / 多列元组」。
- 不收：行情读写、DataFrame 容器、指标业务规则（在 `quant1x/indicators`）、绘图。

**语义以 Python 为准**（`formula.py`），其它语言逐函数对齐，口径登记在 `formula.md`。

## 文件清单

| 文件 | 用途 |
| --- | --- |
| `formula.py` | Python spec：0 / 1 / 2 级函数，数值算子走 `quant1x.num` |
| `formula_plus.py` | MyTT 进阶函数（`SAR` / `TDX_SAR` / `SUMBARSFAST` / `DSMA`） |
| `formula.h` | C++ 生产实现（`quant1x::formula`，`Series = xt::xarray<double>`，`Bools = xt::xarray<bool>`） |
| `formula.md` | 跨语言口径、功能对照表、测试与 TODO |
| `README-MyTT.md` | MyTT 原始说明 |
| `__init__.py` | 重导出 `formula.py` 的公开符号 |
| `formula.ipynb` | 历史研究笔记，不参与交付 |

`quant1x/formula.h`（仓库 `quant1x/` 根下的旧头文件）是更早的 xtensor 版原型，只被
`factors/history.cpp` 与既有用例引用；本目录的 `formula.h` 是新的跨语言实现，两者暂并存，
等旧调用方迁移后再收敛。

## 与上下层的关系

- 上游：`quant1x.num`（数值算子）、NumPy。
- 下游：`quant1x.indicators`、`quant1x.factors`。
- 兄弟模块：`quant1x.pandas` 只在指标层提供容器，公式函数本身不依赖它。

## 跨语言对应关系

| 语义 | python | c++ | rust | go | zig |
| --- | --- | --- | --- | --- | --- |
| 公式函数 | `formula.py` | `formula.h` | — | — | — |
| 进阶函数 | `formula_plus.py` | — | — | — | — |
