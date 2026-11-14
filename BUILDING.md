# Quant1X 构建指南

本文说明 Quant1X 仓库中各个构建单元的依赖、配置、编译、测试与产物位置。

Quant1X 是**多语言平行实现**：C++、Rust、Go、Python、Java 各自独立构建、独立运行，**不存在 FFI / Cgo / SWIG 等跨语言调用**。因此每种语言都有自己的一条构建链路，互不依赖；跨语言之间只通过同名接口、统一的数据契约和同一组黄金测试向量保持一致。

若只想改代码、不关心构建细节，可直接跳到对应语言小节：

| 构建单元 | 构建入口 | 产物 | 适用定位 |
|---|---|---|---|
| C/C++ | `CMakeLists.txt` | 静态库 `quant1x` + 可执行 `q1x` | 生产实现 |
| Rust | `Cargo.toml` | 可执行 `q1x`、库 `libquant1x` | 生态扩展 |
| Go | `go.mod` | 可执行 `q1x`（入口 `main.go`） | 生产/生态 |
| Python | `pyproject.toml` | Python 包 `quant1x` | 探索验证（Spec 锚点） |
| Java | `pom.xml` | JAR | 分布式 ID / Disruptor 对照实现 |

---

## 1. 通用前置条件

### 1.1 版本要求

各语言的版本以仓库中的清单文件为唯一依据，README 徽章仅作参考：

| 语言 | 版本来源文件 | 当前要求 |
|---|---|---|
| C/C++ | `cmake/compile_options.cmake` | C17、C++20 |
| CMake | `CMakeLists.txt` | ≥ 3.30 |
| Rust | `rust-toolchain.toml`、`Cargo.toml` | 工具链由 `rust-toolchain.toml` 锁定，Edition 2024 |
| Go | `go.mod` | 以 `go` 指令声明的最低版本为准 |
| Python | `pyproject.toml` | `requires-python` 为下限，建议 3.12+ |
| Java | `pom.xml` | 源码/字节码目标 Java 8 |

Rust 工具链由 rustup 自动安装：首次在仓库内执行 `cargo` 命令时，若本地缺少 `rust-toolchain.toml` 指定的版本会自动下载，无需手工切换。

### 1.2 构建目录约定

以下目录均为构建产物，已被 `.gitignore` 排除，不要提交：

```text
build/          # C++ 通用构建目录（Ninja / Makefiles）
build-msvc/     # C++ MSVC 构建目录
build-clang/    # C++ Clang 构建目录
target/         # Rust 构建目录
build/java/     # Maven 输出目录
vcpkg_installed/# Windows 下 vcpkg 的本地安装树
```

临时验证脚本、抓取的数据与一次性产物请放在仓库根目录的 `temp/` 下，不要污染源码树。

---

## 2. C/C++ 构建

C/C++ 是项目的生产实现。构建由根目录 `CMakeLists.txt` 统一驱动，子目录 `tests/`、`benches/`、`examples/` 各自追加目标。

### 2.1 依赖与环境变量

CMake 配置阶段**强制要求两个环境变量**，缺失会直接报 `FATAL_ERROR`：

| 环境变量 | 作用 |
|---|---|
| `VCPKG_ROOT` | 指向 vcpkg 仓库根目录；用于加载 `vcpkg.cmake` 工具链文件 |
| `MSF_RUNTIME` | 依赖与安装的根目录；既是 `CMAKE_INSTALL_PREFIX`，也是 protobuf 头文件的查找根（非 Windows 平台其 `lib/` 也进入链接搜索路径） |

Windows（cmd）示例：

```bat
set VCPKG_ROOT=d:/runtime/vcpkg
set MSF_RUNTIME=d:/runtime
```

Linux / macOS（bash）示例：

```bash
export VCPKG_ROOT=$HOME/runtime/vcpkg
export MSF_RUNTIME=$HOME/runtime
```

vcpkg 依赖清单见根目录 `vcpkg.json`（manifest 模式）。若无网络或使用预装依赖，可关闭 manifest 模式：

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVCPKG_MANIFEST_MODE=OFF
```

此时需保证 `VCPKG_INSTALLED_DIR` 对应的 triplet 目录下已装好依赖：

```bash
vcpkg install openssl libiconv zlib minizip yaml-cpp mimalloc cpr capnproto benchmark catch2 gtest
```

### 2.2 平台与 triplet

CMake 会根据平台自动选择 vcpkg triplet（可用 `-DVCPKG_TARGET_TRIPLET=...` 覆盖）：

| 平台 | 编译器 | triplet |
|---|---|---|
| Windows | MSVC 或 Clang | `x64-windows-static` |
| Windows | MinGW | `x64-mingw-static` |
| Linux | GCC / Clang | `x64-linux` |
| macOS | Apple Clang（arm64） | `arm64-osx` |
| macOS | Apple Clang（x86_64） | `x64-osx` |

所有依赖按**静态链接**配置：`BUILD_SHARED_LIBS=OFF`、`VCPKG_LIBRARY_LINKAGE=static`、`VCPKG_CRT_LINKAGE=static`；Windows 默认 `USE_STATIC_CRT=ON`。使用 GNU 工具链（GCC / MinGW）时会额外追加 `-static -lpthread`。

Windows 使用 MSVC 或 Clang 时，vcpkg 安装树会落在仓库内的 `vcpkg_installed/`；其他平台使用 `$VCPKG_ROOT/installed`。

### 2.3 protobuf 的独立安装

protobuf 不走 vcpkg 清单，而是按固定版本手工安装到 `MSF_RUNTIME`：

- `cmake/protobuf.cmake` 先尝试从 `${MSF_RUNTIME}/include/google/protobuf/port_def.inc` 解析版本号；
- 库按名字直接链接：MSVC 为 `libprotobuf`（Debug 为 `libprotobufd`），Clang/Linux/GCC 为 `protobuf`（部分平台 Debug 为 `libprotobufd`）。

推荐 3.21.x 系列：更高版本引入 abseil 依赖，会显著增加构建复杂度。

### 2.4 配置

Windows + MinGW（当前仓库 `build/` 使用的组合）：

```bat
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
```

Windows + Visual Studio 2022：

```bat
cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-windows-static
```

Linux / macOS（推荐 Ninja）：

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

说明：

- 未显式指定 `CMAKE_BUILD_TYPE` 时默认 `Debug`；
- Visual Studio 是多配置生成器，构建类型在 `--config` 中指定；
- 配置阶段会输出编译器、标准、依赖版本汇总，便于排查环境问题；
- 已开启 `CMAKE_EXPORT_COMPILE_COMMANDS`，供 clangd / IDE 使用。

### 2.5 构建

```bash
# 全量构建
cmake --build build -j

# 只构建主库与命令行入口
cmake --build build --target quant1x q1x -j
```

产物位置（单配置生成器，如 Ninja / MinGW Makefiles）：

| 产物 | 路径 |
|---|---|
| 命令 `q1x` | `build/q1x`（Windows 为 `build/q1x.exe`） |
| 静态库 | Release 为 `build/quant1x.a`（MSVC 为 `quant1x.lib`）；Debug 带 `DEBUG_POSTFIX`，如 `quant1x-debug-gnu-win.a` |
| 测试可执行 | `build/tests/app-*`、`catch2-*`、`gtest-*` |
| 基准可执行 | `build/benches/benchmark-*` |
| 示例可执行 | `build/examples/*` |

Visual Studio 生成器会在配置名子目录下，如 `build-msvc/Release/q1x.exe`。

### 2.6 常用 CMake 选项

| 选项 | 默认 | 说明 |
|---|---|---|
| `CMAKE_BUILD_TYPE` | `Debug` | `Debug` / `Release` / `RelWithDebInfo` / `MinSizeRel` |
| `USE_STATIC_CRT` | `ON` | Windows 下使用静态 CRT（`MultiThreaded`） |
| `FAST_CRON_TEST` | `OFF` | 打开后缩短 cron 测试时序，用于快速回归 |
| `ENABLE_MIMALLOC` | `OFF` | 启用 mimalloc；默认只重载 C++ `new/delete` |
| `MIMALLOC_MODE` | `new_delete` | 设为 `global` 时重载 C 分配器（`MI_OVERRIDE=1`） |
| `ENABLE_MIMALLOC_CHECK` | `OFF` | 额外构建 `mimalloc_check` 运行时校验程序 |

编译策略：

- Release 下自动尝试 LTO（`-flto` / `/GL`，编译器支持时启用）；
- 指令集按编译器与架构区分：MSVC 为 Release `/arch:AVX2`、Debug `/arch:AVX`；GCC / Clang 在 x86 上追加 `-mavx -mavx2 -march=native -mtune=native`，在 arm64 上追加 `-march=native -mtune=native`；
- **带 `-march=native` 的二进制不可跨机分发**，发布/CI 构建应在目标平台编译，或手工去掉该选项。

### 2.7 测试

C++ 测试通过 CTest 统一驱动。`tests/CMakeLists.txt` 中只有 Catch2 目标会自动注册用例（`catch_discover_tests`）并附加整程序的回退 `add_test`；`app-*`、`gtest-*` 目标只生成可执行文件，需要直接运行：

```bash
# 运行全部测试
ctest --test-dir build --output-on-failure -j 8

# 按名称过滤（名称以 ctest -N 输出为准）
ctest --test-dir build -R vyukov -V

# 列出所有已注册用例
ctest --test-dir build -N
```

也可以直接运行测试进程，便于挂调试器：

```bash
build/tests/catch2-tdd-bars "某个测试名"
build/tests/app-test-ringbuffer
```

注意：部分用例依赖本地数据缓存、网络或平台特性（如 `tdd-http`、`tdd-history`、crash 相关），在离线环境可能失败，属预期现象，需结合用例注释判断。

### 2.8 基准测试

```bash
cmake --build build --target benchmark-vyukov_bench -j
build/benches/benchmark-vyukov_bench --benchmark_filter=...
```

`benches/` 与 Rust 侧 `benches/vyukov_bench.rs` 参数对齐，用于跨语言性能对比。

### 2.9 安装

```bash
cmake --build build -j
cmake --install build
```

安装前缀固定为 `MSF_RUNTIME`（`CMAKE_INSTALL_PREFIX=${MSF_RUNTIME}`），安装内容：

- `bin/`：`q1x`
- `lib/`：`quant1x` 静态库
- `include/quant1x/`：项目头文件 + 生成的 `build-info.h`
- `include/third_party/`：聚合的第三方头文件
- `lib/cmake/quant1x/`：`quant1x-config.cmake`、`quant1x-targets.cmake`，供下游 `find_package(quant1x)` 使用

### 2.10 新增 C/C++ 源文件

根 `CMakeLists.txt` **显式列举**了全部头文件（`quant1x-headers`）与源文件（`quant1x-sources`），没有使用 `file(GLOB)`。因此：

- 新增 `.cpp` 必须加入 `quant1x-sources`，否则不会参与编译；
- 新增对外头文件必须加入 `quant1x-headers`，否则安装时可能缺失；
- 新增测试需在 `tests/CMakeLists.txt` 中按类型登记（`add_catch2_executable` / `add_gtest_executable` / `add_app_executable`）；
- 新增基准需在 `benches/CMakeLists.txt` 中登记 `add_benchmark_executable`。

---

## 3. Rust 构建

Rust 实现以 C++ 为基准做 1:1 语义还原，库入口为 `quant1x/lib.rs`，命令行入口为 `main.rs`。

### 3.1 构建

```bash
# Debug
cargo build --bin q1x

# Release（生产推荐）
cargo build --release --bin q1x
```

产物：

- `target/debug/q1x`、`target/release/q1x`（Windows 加 `.exe`）
- 库同时产出 `rlib` 与 `cdylib`（见 `[lib] crate-type`）

### 3.2 其他可执行目标

`Cargo.toml` 中除 `q1x` 外还声明了性能工具：

```bash
cargo run --release --bin vyukov_perf
cargo run --release --bin vyukov_runner
```

### 3.3 测试与基准

```bash
# 常规测试
cargo test

# Ringbuffer（Vyukov）相关测试受 feature 控制，需显式开启
cargo test --features vyukov

# 基准（使用 Criterion，harness 已关闭）
cargo bench --bench vyukov_bench
```

说明：`[[bench]] vyukov_bench` 的 `harness = false` 是必需的，否则 Criterion 的 `main` 会被 libtest 覆盖，表现为 `running 0 tests` 却以退出码 0 通过。

### 3.4 平台差异

- Windows 专有依赖通过 `[target.'cfg(windows)'.dependencies]` 引入 `winapi`；
- Unix 通过 `[target.'cfg(unix)'.dependencies]` 引入 `libc`；
- `service` 子命令的命名管道、服务托管与提权参数主要面向 Windows 使用。

---

## 4. Go 构建

Go 实现位于 `quant1x/` 的功能子目录中，包名与目录名一致（如 `quant1x/data/t1/` 为 `package t1`）。模块路径以 `go.mod` 为准。

### 4.1 构建

```bash
# 编译全部包（含子包）
go build ./...

# 编译命令行入口
go build -o q1x-go .
```

交叉编译示例：

```bash
GOOS=linux GOARCH=amd64 go build -o q1x-linux .
```

### 4.2 测试与静态检查

```bash
go test ./...
go vet ./...
gofmt -l .
```

`tests/` 下存在跨语言一致性用例（如日历解码 `TestDecodeJavaScript`、`TestYReadsBits`）：

```bash
go test ./tests -v
go test ./tests -run TestDecodeJavaScript -v
```

### 4.3 依赖代理

网络受限时可通过环境变量配置：

```bash
go env -w GOPROXY=https://goproxy.cn,direct
go env -w GOPRIVATE=gitee.com
```

---

## 5. Python 构建

Python 侧是 Spec 锚点，同时承担数据探索与验证。包版本由 `setuptools_scm` 从 Git Tag 推导，仓库中不保存手写版本号。

### 5.1 安装

```bash
python -m venv .venv
# Windows: .venv\Scripts\activate
# Linux/macOS: source .venv/bin/activate

pip install -U pip
pip install -e .
```

说明：

- 构建后端为 `setuptools.build_meta`，已声明 `setuptools`、`wheel`、`setuptools_scm`；
- `setuptools_scm` 会把版本写入 `quant1x/_version.py`（该文件被 `.gitignore` 排除，不要手工编辑）；
- 仓库无 `requirements.txt`，运行依赖统一在 `pyproject.toml` 的 `dependencies` 中维护；
- 若在无 Tag 的浅克隆中安装，版本会回退到 `0.0.0`。

### 5.2 测试

测试运行器未列入运行依赖，需单独安装：

```bash
pip install pytest
python -m pytest tests -q
```

`tests/conftest.py` 会把仓库根目录加入 `sys.path`，保证在仓库根目录直接运行 pytest 时能 `import quant1x`。部分用例依赖本地数据缓存或网络（如 `test_real_kline.py`、`test_margin_trading.py`），离线环境下应按需跳过。

### 5.3 打包

```bash
pip install build
python -m build
```

产物位于 `dist/`（`*.tar.gz` 与 `*.whl`）。打包内容通过 `MANIFEST.in` 与 `[tool.setuptools.packages.find]` 控制，`third_party`、`tests`、`target` 等目录已排除。

### 5.4 数据源切换

Python 默认读取 Go 版本的数据缓存；如需切换到 C++ / Rust 生成的缓存，设置环境变量：

```bash
export QUANT1X_WORK=q1x        # C++
export QUANT1X_WORK=q1x-rust   # Rust
```

---

## 6. Java 构建

Java 部分只承载分布式 ID 与 LMAX Disruptor 等对照实现/基准，不参与四语言核心链路。

### 6.1 构建与测试

```bash
# Linux / macOS
./mvnw -B test
./mvnw -B -DskipTests package

# Windows
mvnw.cmd -B test
mvnw.cmd -B -DskipTests package
```

若 wrapper 不可用，直接使用本机 Maven：`mvn -B test`。

### 6.2 工程约定

- 源码根为仓库根目录，通过 `includes` 白名单只编译 `quant1x/**/*.java` 与 `tests/**/*.java`；
- 测试源码目录为 `tests/`，包名必须与目录层级一致（如 `tests/distributed/id` 对应 `package tests.distributed.id`）；
- 输出目录固定为 `build/java/`，与 Rust 的 `target/` 隔离，`mvn clean` 不会误删其他语言产物；
- 字节码目标为 Java 8，`-Xlint:-options` 仅用于隐藏 JDK 9+ 对旧 `source/target` 的告警。

---

## 7. 跨语言一致性验证

构建完成后，四语言应能对同一输入给出 epsilon 内一致的结果。验证方式：

1. 准备同一组输入与黄金输出（`tests/` 下的 fixture、CSV、JSON，或由 Python 生成的基准数据）；
2. 分别运行各语言实现；
3. 对浮点结果按约定的 epsilon 比较，对结构体按字段逐一比较（字段名必须是全小写 `snake_case`）。

可参考现有 parity 用例：

- 日历解码：`tests/calendar_decode_test.go`、`tests/calendar_decode.js`、`tests/calendar_integration.rs`
- Ringbuffer 性能对齐：`quant1x/runtime/ringbuffer.md`、`benches/vyukov_bench.{cpp,rs}`

新增功能时，按 Python → C++ → Rust/Go 的顺序落地，并在 `tests/` 中补齐对应语言的验证入口。

---

## 8. 常见问题

| 现象 | 原因与处理 |
|---|---|
| `请设置 VCPKG_ROOT 环境变量指向全局 vcpkg 目录` | 未设置 `VCPKG_ROOT`，或指向的目录不存在 |
| `NOT found ENV MSF_RUNTIME` | 未设置 `MSF_RUNTIME`；它同时是安装前缀与 protobuf 查找根 |
| 链接阶段找不到 protobuf | protobuf 需手工安装到 `$MSF_RUNTIME`；Debug 构建在 MSVC/Clang 下需 `libprotobufd` |
| MinGW 下 zlib 链接失败 | vcpkg 的 MinGW 静态库名为 `libzs.a`，CMake 已自动指定；如仍失败，检查 triplet 是否为 `x64-mingw-static` |
| 二进制在别的机器上报非法指令 | GCC / Clang 构建默认带 `-march=native`，需在目标平台构建或去掉该选项 |
| `ctest` 报 `No tests were found` | 未构建测试目标，先 `cmake --build build -j` 再执行 `ctest` |
| Rust 报 `invalid metadata version` | `rust-analyzer` 与工具链版本不一致；让 rustup 按 `rust-toolchain.toml` 重新安装组件 |
| Rust 基准 `running 0 tests` | `[[bench]]` 的 `harness = false` 被改动，导致 libtest 覆盖了 Criterion 的 `main` |
| `pip install -e .` 版本变成 `0.0.0` | 仓库没有可达的 Git Tag；`setuptools_scm` 回退到 `fallback_version` |
| 提交被 pre-commit 拒绝 | 新文件混入了 BOM，按提示去掉 BOM 后重新提交 |
| Java 报 `package tests.xxx does not match ...` | 包名未按 `tests/` 根层级书写；不得额外加 `quant1x.` 前缀 |

---

## 9. 版本与发布约束

- 项目版本由 Git Tag（`vMAJOR.MINOR.PATCH`）唯一决定，C++ 在配置阶段通过 `cmake/git_tag_info.cmake` 读取，Python 通过 `setuptools_scm` 推导。
- `Cargo.toml`、`pom.xml`、`pyproject.toml` 中的版本字段与 `CHANGELOG.md` **不得手工修改**，发布统一走 `autochangelog`。
- 提交信息使用中文，主题格式为 `<type>[<language>]：<summary>`，具体规则见 `AGENTS.md` 与 `CONTRIBUTING.md`。

---

## 10. 相关文档

| 文档 | 内容 |
|---|---|
| [README.md](README.md) | 项目概览、快速开始、使用示例 |
| [CONTRIBUTING.md](CONTRIBUTING.md) | 架构理念、目录规范、命名与开发流程 |
| [AGENTS.md](AGENTS.md) | AI 协作规则、提交规范、版本约束 |
| [docs/README.md](docs/README.md) | 数据项与更新时序 |
| [tests/README.md](tests/README.md) | 测试约定与 parity 复现步骤 |
| [examples/README.md](examples/README.md) | 示例程序说明 |
