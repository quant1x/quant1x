# globmatch：锚定 glob 匹配（globstar）

按「整条锚定 + 逐段 glob」判断一条相对路径是否命中模式：`**` 匹配零到多段，其余段按
`*` / `?` / `[...]` 通配。语义与 Python 3.13 `PurePath.full_match()`、bash 的 `globstar`
同义，只做匹配、不接触文件系统。

## 1. 路线图

**当前位置与下一步**：五种语言的实现与对等用例均已落地，根 `license-header.toml` 的忽略清单
可按同一套 glob 写法填写；本模块无已知语义缺口，剩下的只是各语言接口形态的统一（见 §5）。

| 阶段 | 语言（角色） | 载体 / 文件 | 状态 | 说明 |
| --- | --- | --- | --- | --- |
| spec 定语义 | Python（spec） | `quant1x/base/globmatch.py` | 已落地 | 语义、API 与边界以它为准 |
| 生产实现 | C++（生产） | `quant1x/base/globmatch.hpp` / `globmatch.cpp` | 已落地 | 编入根 CMake 目标 `quant1x` |
| 对齐实现 | Rust（对齐） | `quant1x/base/globmatch.rs`（`quant1x::base::globmatch`） | 已落地 | 与 Python 同一批样本 |
| 对齐实现 | Go（对齐） | `quant1x/base/globmatch.go`（`package base`） | 已落地 | `Match` / `MatchAny` |
| 扩展 | Zig（扩展） | `quant1x/base/globmatch.zig` | 已落地 | `zig test` 直接跑 |

## 2. 功能清单

**统一口径**：

- **输入**：`path` 与 `pattern` 均为 `/` 分隔的字符串；连写 `/` 视同分隔符（空段丢弃）。
  `path` 或 `pattern` 为空时一律返回假。
- **项目根绝对形式**：两者都可带前导 `/`，表示「从项目根算起」，与相对写法等价
  （`/third_party/a.h` ≡ `third_party/a.h`，两侧可混写）；匹配本身始终以项目根为锚，前导 `/` 只是显式标记。
  文件系统绝对路径（`D:/repo/...`、`/home/...`）不是本模块的输入，调用方须先相对化。
- **锚定**：从第一个段起整条比对，不做「右对齐」（`libs/*.h` 不命中 `x/libs/a.h`）。
- **段匹配**：`**`（整段）匹配零到多段；模式以 `/**` 结尾时要求目录下至少一个条目（等价
  `/**/*`，即 `a/**` 不命中 `a` 本身）；其余段按 `*`（任意序列，可空）、`?`（单个 Unicode 标量）、
  `[abc]` / `[a-z]` / `[!a-z]`（单标量集合，`!` 取反）匹配；未闭合的 `[` 按字面量。
- **字符与大小写**：按 Unicode 标量逐个比较，大小写敏感，`*` 可匹配点开头的名字（与 fnmatch 一致）；
  `\` 没有转义含义。
- **不涉及**：花括号展开与文件系统访问；本模块只判断「路径是否命中模式」，配置段怎么定义
  （如忽略清单）由调用方负责。

| 功能 | python（spec） | c++ | rust | go | zig | java | 状态 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 单条模式匹配整条路径 | `match` | `globmatch::matches` | `matches` | `Match` | `matches` | — | 已落地 |
| 任一模式命中 | `match_any` | `globmatch::matches_any` | `matches_any` | `MatchAny` | `matchesAny` | — | 已落地 |

失败约定：全部语言只返回布尔值，不抛异常 / 不返回错误（空输入即假）。

## 3. 依赖

**出向依赖（本模块依赖的东西）**：本模块无三方运行时依赖 —— 五种实现都只用各自标准库
（Python `fnmatch` 仅用于段内通配，Rust / Go / Zig / C++ 为标准库手写实现）。

**入向依赖（谁依赖本模块）**：

| 调用方 | 用途 | 备注 |
| --- | --- | --- |
| — | — | 本仓暂无调用方 |

根 `license-header.toml` 的 `[ignore] paths` 采用同一套 glob 写法，但读取该配置的工具不在本仓，
不构成对本模块的依赖。

## 4. 测试

### 4.1 单元测试

同一批样本（7 组：globstar 零/多段、锚定、单段通配符、点文件与大小写、尾段 `/**`、空输入、
任一命中）在五种语言里逐条对齐，断言同一结果。

| 语言 | 测试文件 | 用例数 | 运行命令 |
| --- | --- | --- | --- |
| Python（spec） | `quant1x/base/globmatch_test.py` | 9 | `python -m quant1x.base.globmatch_test` |
| C++ | `tests/tdd-globmatch.cpp` | 8 | `ctest --test-dir build-msvc -C Release -R globmatch` |
| Rust | `quant1x/base/globmatch.rs` 内联 `mod tests` | 8 | `cargo test --lib globmatch` |
| Go | `quant1x/base/globmatch_test.go` | 8 | `go test ./quant1x/base/` |
| Zig | `quant1x/base/globmatch_test.zig` | 8 | `zig test quant1x/base/globmatch_test.zig` |

Python 侧另有 1 项与 `PurePosixPath.full_match()` 的逐例对拍（3.13 以下自动跳过），用来钉住
「规则名」对应的语义。

### 4.2 覆盖率

| 语言 | 工具 | 门槛 | 运行命令 |
| --- | --- | --- | --- |
| Python | `coverage` | 未设门槛 | `python -m coverage run --source=quant1x.base.globmatch -m quant1x.base.globmatch_test && python -m coverage report` |
| C++ | `lcov` / `gcovr` | 未设门槛 | 随 `ctest` 构建产物生成，本仓未接入 |
| Rust | `cargo-llvm-cov` | 未设门槛 | `cargo llvm-cov --lib globmatch` |
| Go | `go test -cover` | 未设门槛 | `go test ./quant1x/base/ -cover` |
| Zig | `zig test` 覆盖率导出（待定工具） | 未设门槛 | 本仓未接入 |

### 4.3 重复度

本模块不涉及（本仓未引入重复度门禁）。跨语言的五份实现是 spec→生产的语言角色使然，不属于重复。

### 4.4 静态检查

| 语言 | 工具 | 运行命令 | 备注 |
| --- | --- | --- | --- |
| Python | `ruff` ＋ `pyright` | `python -m ruff check quant1x/base/globmatch*.py` ＋ `python -m pyright quant1x/base/globmatch*.py` | 规则集见根 `pyproject.toml` |
| C++ | `clang-tidy` ＋ `clang-format` | 针对 `quant1x/base/globmatch.*`，按根 `.clang-format` / `.clang-tidy` | 本仓验证环境为 MSVC v143 + CMake；clang 工具链未纳入矩阵 |
| Rust | `cargo fmt --check` ＋ `cargo clippy` | `cargo fmt --check` ＋ `cargo clippy -- -D warnings` | |
| Go | `gofmt -l` ＋ `go vet` | `gofmt -l quant1x/base/` ＋ `go vet ./quant1x/base/` | |
| Zig | `zig fmt --check` | `zig fmt --check quant1x/base/globmatch.zig quant1x/base/globmatch_test.zig` | |

### 4.5 基准测试

本模块不涉及（无性能契约；单次匹配为短路径上的逐段比对，调用方是文件遍历，不构成热点）。

### 4.6 契约 / 一致性测试

| 场景 | 测试位置 | 断言内容 | 运行命令 |
| --- | --- | --- | --- |
| 五种语言匹配同一批路径/模式样本 | 各语言用例（样本清单即 §4.1 的 7 组） | 布尔结果逐条一致 | 见 §4.1 各行 |
| Python 与 `PurePosixPath.full_match()` 语义对齐 | `quant1x/base/globmatch_test.py` | 11 条路径 × 10 条模式零分歧 | `python -m quant1x.base.globmatch_test` |

## 5. TODO

| 待办项 | 缺什么 | 卡在哪 | 关联章节 | 关联 issue | 优先级 |
| --- | --- | --- | --- | --- | --- |
| 接口形态统一 | C++ 用 `vector<string_view>`、Go 用 `[]string`、Rust 用泛型 `AsRef<str>` | 无技术阻塞，待排期（各自语言习惯不同，不影响语义） | §2 | — | 低 |
