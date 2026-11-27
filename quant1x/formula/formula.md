# formula —— 通达信公式函数：可移植的窗口、NaN 与回归口径

**边界**：本模块只做「一维序列 + 参数 → 序列」的纯计算，不读写行情、不持有 DataFrame。
行情结构在 `quant1x/data/schema`，指标业务规则在 `quant1x/indicators`，数值底座在 `quant1x/num`。

## 1. 路线图

| 阶段 | 语言（角色） | 载体 | 状态 | 说明 |
| --- | --- | --- | --- | --- |
| spec 定语义 | Python（spec） | `formula.py` | 已落地 | 全量 0 / 1 / 2 级函数与用例 |
| 生产实现 | C++（生产） | `formula.h` | 已落地 | `Series = std::vector<double>`，逐值对拍 |
| 对齐实现 | Rust | `formula/mod.rs` | 已落地 | `Vec<f64>` + 显式循环，逐值对拍 |
| 对齐实现 | Go | `formula_go.go` / `rolling_go.go` | 已落地 | `[]float64` + 显式循环；复用内部 `quant1x/num` |
| 扩展实现 | Zig | — | 未落地 | 见 §5 TODO |

## 2. 功能清单

**统一口径**（各语言必须一致）：

- **序列**：`float64` 一维序列；布尔序列单独成类型；缺失只有 NaN。C++ 侧用 xtensor 与 NumPy
  对标：`Series = xt::xarray<double>`、`Bools = xt::xarray<bool>`，加减乘除与比较、逻辑运算
  直接用 C++ 运算符，写法与 Python 一致。
- **C++ 的相等比较**：xtensor 的 `operator==` / `operator!=` 是**整表比较**（返回 bool），
  逐元素相等必须用 `equal` / `equal_scalar` / `not_equal`，不能写 `a == b`。
- **C++ 的逻辑运算**：启用 `XTENSOR_USE_XSIMD` 后，`&` / `|` 会走 `bitwise_and` 的 SIMD 路径，
  MSVC 下无法实例化；布尔组合一律写 `&&` / `||` / `!`（或 `logic_and` / `logic_or` / `logic_not`），
  与 Python 的 `&` / `|` / `~` 语义相同。
- **窗口**：常量或逐位置序列。`w <= 0`、`w > i+1`、窗口内含 NaN → 该位置 NaN。
- **`REF` 的序列窗口**：允许负位移（向前引用），越界位置为 NaN。
- **`EMA` / `SMA`**：`y_t = (1-alpha)·y_{t-1} + alpha·x_t`，首值作种子；NaN 跳过并沿用前值。
  `EMA` 的 `alpha = 2/(N+1)`，`SMA` 的 `alpha = M/N`（`M` 缺省 1，`N = 0` 时 `alpha = 1`）。
- **`SLOPE` / `FORCAST`**：均值中心化最小二乘闭式解，窗口小于 2 或含 NaN → NaN。
- **`WMA`**：权重按「窗口内从旧到新 1..n」，与 `WMA = Σi·X_i / Σi` 的通达信定义一致。
- **`RD`**：半程远离零（`0.5 → 1`、`-0.5 → -1`），不是银行家舍入。
- **`ZTPRICE`**：对「最短往返十进制表示」做十进制乘法，再按分 ROUND_HALF_UP。
- **`FILTER`**：返回 0/1 浮点序列，不写回入参（入参可能是 bool，写 0 会抛 `TypeError`）。
- **`BARSLAST`**：条件成立当天为 0，之前未成立过时从 1 开始递增。
- **`BARSSINCEN`**：窗口无效或从未成立为 0；并列取第一次成立。
- **`HHVBARS` / `LLVBARS`**：并列取最近的一个。

| 功能 | python | c++ | rust | go | zig | 状态 |
| --- | --- | --- | --- | --- | --- | --- |
| 绝对值 / 对数 / 幂 / 平方根 | `ABS` / `LN` / `POW` / `SQRT` | `abs` / `log` / `pow` / `sqrt` | `abs` / `log` / `pow` / `sqrt` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 取大 / 取小 / 条件选择 | `MAX` / `MIN` / `IF` / `IFF` / `IFN` | `maximum` / `minimum` / `if_select` | `maximum` / `minimum` / `if_select` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 引用 / 差分 | `REF` / `DIFF` | `ref` / `diff` | `ref_value` / `diff` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 累计和 / 常量序列 | `SUM` / `CONST` | `sum` / `const_series` | `sum` / `const_series` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 极值与位置 | `HHV` / `LLV` / `HHVBARS` / `LLVBARS` | `hhv` / `llv` / `hhvbars` / `llvbars` | `hhv` / `llv` / `hhvbars` / `llvbars` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 均线 | `MA` / `EMA` / `SMA` / `WMA` / `DMA` | `ma` / `ema` / `sma` / `wma` / `dma` | `ma` / `ema` / `sma` / `wma` / `dma` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 波动与回归 | `STD` / `AVEDEV` / `SLOPE` / `FORCAST` | `stddev` / `avedev` / `slope` / `forcast` | `stddev` / `avedev` / `slope` / `forcast` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 区间条件 | `LAST` | `last` | `last` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 计数与存在 | `COUNT` / `EVERY` / `EXIST` | `count` / `every` / `exist` | `count` / `every` / `exist` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 信号过滤 | `FILTER` | `filter` | `filter` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 条件周期数 | `BARSLAST` / `BARSLASTCOUNT` / `BARSSINCEN` | `barslast` / `barslastcount` / `barssincen` | `barslast` / `barslastcount` / `barssincen` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 交叉 | `CROSS` / `LONGCROSS` | `cross` / `longcross` | `cross` / `longcross` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 取值保持 / 区间 | `VALUEWHEN` / `BETWEEN` | `valuewhen` / `between` | `valuewhen` / `between` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 区间位置 | `TOPRANGE` / `LOWRANGE` | `toprange` / `lowrange` | `toprange` / `lowrange` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| MACD / KDJ / RSI | `MACD` / `KDJ` / `RSI` | `macd` / `kdj` / `rsi` | `macd` / `kdj` / `rsi` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| WR / BIAS / BOLL / PSY | `WR` / `BIAS` / `BOLL` / `PSY` | `wr` / `bias` / `boll` / `psy` | `wr` / `bias` / `boll` / `psy` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| CCI / ATR / BBI / DMI | `CCI` / `ATR` / `BBI` / `DMI` | `cci` / `atr` / `bbi` / `dmi` | `cci` / `atr` / `bbi` / `dmi` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| TAQ / KTN / TRIX / VR | `TAQ` / `KTN` / `TRIX` / `VR` | `taq` / `ktn` / `trix` / `vr` | `taq` / `ktn` / `trix` / `vr` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| EMV / DPO / BRAR / DFMA | `EMV` / `DPO` / `BRAR` / `DFMA` | `emv` / `dpo` / `brar` / `dfma` | `emv` / `dpo` / `brar` / `dfma` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| MTM / MASS / ROC / EXPMA | `MTM` / `MASS` / `ROC` / `EXPMA` | `mtm` / `mass` / `roc` / `expma` | `mtm` / `mass` / `roc` / `expma` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| OBV / MFI / ASI / XSII | `OBV` / `MFI` / `ASI` / `XSII` | `obv` / `mfi` / `asi` / `xsii` | `obv` / `mfi` / `asi` / `xsii` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |
| 舍入 / 涨停价 | `RD` / `ZTPRICE` | `rd` / `ztprice` | `rd` / `ztprice` | `*.go` | — | 已对齐 c++/rust/go（缺 zig） |

## 3. 依赖

**出向**

| 依赖 | 用途 | 为什么必须是它 |
| --- | --- | --- |
| `quant1x.num` | Python 侧全部数值算子 | 口径只在 num 定义一次 |
| xtensor | C++ 侧容器与四则/比较/逻辑运算符 | 仓库既有依赖，与 NumPy 语义对标 |
| 标准库（C++） | 超越函数、十进制字符串、容器辅助 | 超越函数保持 `std::` 调用，避免 xsimd 末位差异 |

**入向**

| 使用方 | 用途 | 反向约束 |
| --- | --- | --- |
| `quant1x.indicators` | 五个指标的公式底座 | 只能按本文件口径调用 |
| `quant1x.factors` | 历史因子计算 | 旧接口走 `quant1x/formula.h`，不受本模块影响 |

## 4. 测试

| 维度 | 覆盖 |
| --- | --- |
| 单元测试 | `tests/test_num.py`、`tests/test_pandas_facade.py` 覆盖底层口径 |
| 跨语言对拍 | `tests/fixtures/formula/cases.csv`（57 组）+ `tests/tdd-formula-parity.cpp`（C++）、`tests/formula_parity.rs`（Rust）、`quant1x/formula/formula_parity_test.go`（Go），逐位相等；Go 的 `math.Log` / `math.Pow` 与 NumPy 至多 1 ULP 差异，按相对误差 1e-12 比较 |
| fixture 守门 | `tests/test_formula_parity.py` 断言重生成与签入文件逐字节一致 |
| 静态检查 | C++ 以 `/W4` 编译无警告；Python 走仓库 ruff 配置 |
| 崩溃注入 / 并发 | 本模块不涉及（纯函数、无共享状态） |

对拍命令：

```bash
python tests/fixtures/formula/generate.py
python tests/test_formula_parity.py
cl /nologo /utf-8 /std:c++20 /EHsc /I . tests/tdd-formula-parity.cpp
cargo test --test formula_parity
go test ./quant1x/formula/
```

## 5. TODO

- Rust / Go / Zig 三侧尚未落地：按本文件 §2 的口径逐函数实现，复用同一份 `cases.csv`，
  完成后把 §2 对照表的 `—` 换成文件名。
- `BARSSINCEN` 的窗口内「从未成立」与 Python 的 `fillna(0)` 等价，但 Python 侧窗口无效时
  直接给 0；其它语言照此实现，不要改成 NaN。
- `formula_plus.py` 的 `SAR` / `TDX_SAR` / `SUMBARSFAST` / `DSMA` 尚未纳入对拍，也未在
  C++ 落地；它们依赖有状态迭代，先补 fixture 再推广。
- `quant1x/formula.h`（旧 xtensor 原型）与本模块并存；等 `factors/history.cpp` 迁移后删除。
