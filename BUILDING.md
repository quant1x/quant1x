# Quant1X 构建指南

本文说明 Quant1X 仓库中各个构建单元的依赖、配置、编译、测试与产物位置。

Quant1X 是**多语言平行实现**：C++、Rust、Go、Python、Zig、Java 各自独立构建、独立运行，**不存在 FFI / Cgo / SWIG 等跨语言调用**。因此每种语言都有自己的一条构建链路，互不依赖；跨语言之间只通过同名接口、统一的数据契约和同一组黄金测试向量保持一致。

若只想改代码、不关心构建细节，可直接跳到对应语言小节：

| 构建单元 | 构建入口 | 产物 | 适用定位 |
|---|---|---|---|
| C/C++ | `CMakeLists.txt` | 静态库 `quant1x` + 可执行 `q1x` | 生产实现 |
| Rust | `Cargo.toml` | 可执行 `q1x`、库 `libquant1x` | 生态扩展 |
| Go | `go.mod` | 可执行 `q1x`（入口 `main.go`） | 生产/生态 |
| Python | `pyproject.toml` | Python 包 `quant1x` | 探索验证（Spec 锚点） |
| Zig | 无清单文件（源码与用例直跑） | 无安装物；`zig test` 直接执行 | 扩展（当前只承载小功能） |
| Java | `pom.xml` | JAR | 分布式 ID / Disruptor 对照实现 |

---

## 1. 通用前置条件

### 1.1 版本要求

各语言的版本以仓库中的清单文件为唯一依据，README 徽章仅作参考：

| 语言 | 版本来源文件 | 当前要求 |
|---|---|---|
| C/C++ | `cmake/compile_options.cmake` | C17、C++20；Windows 使用 MSVC |
| C/C++ 依赖 | `conanfile.py`、`conan/profiles/` | Conan 2.x |
| CMake | `CMakeLists.txt` | ≥ 3.30 |
| Rust | `rust-toolchain.toml`、`Cargo.toml` | 1.100.0（Edition 2024），由 `rust-toolchain.toml` 锁定 |
| Go | `go.mod` | 以 `go` 指令声明的最低版本为准 |
| Python | `pyproject.toml` | `requires-python` 为下限，当前 3.12+ |
| Zig | 无清单文件 | 0.17+（`std.Io` API），本地 0.17.1 实测 |
| Java | `pom.xml` | 源码/字节码目标 Java 8 |

Rust 工具链由 rustup 自动安装：首次在仓库内执行 `cargo` 命令时，若本地缺少 `rust-toolchain.toml` 指定的版本会自动下载，无需手工切换。

### 1.2 构建目录约定

以下目录均为构建产物，已被 `.gitignore` 排除，不要提交：

```text
build/          # C++ 通用构建目录（Ninja / Makefiles）
build-msvc/     # C++ MSVC 构建目录（Windows 默认）
build-clang/    # C++ Clang 构建目录
target/         # Rust 构建目录
build/java/     # Maven 输出目录
```

临时验证脚本、抓取的数据与一次性产物请放在仓库根目录的 `temp/` 下，不要污染源码树。

---

## 2. C/C++ 构建（Windows 使用 MSVC）

C/C++ 是项目的生产实现。第三方依赖统一由 **Conan 2** 获取，CMake 侧只消费 Conan 生成的工具链与 imported target，不再直接拼接依赖目录。

### 2.1 前置要求

| 组件 | 要求 |
|---|---|
| Conan | 2.x（`conan --version` 确认） |
| CMake | ≥ 3.30 |
| Windows | Visual Studio 2022，**统一使用 MSVC 编译** |
| Linux / 其他 Unix | **GCC** 13+（不使用 Clang） |
| macOS | **Clang**（Apple Clang） |

编译器按平台固定：Windows → MSVC，macOS → Clang，Linux/其他 Unix → GCC；与平台不符时 CMake 配置阶段直接报错。

首次使用 Conan 时生成默认 profile：

```bash
conan profile detect --force
```

环境变量 `MSF_RUNTIME` 仍然必须设置（与依赖管理器无关）：

| 环境变量 | 作用 |
|---|---|
| `MSF_RUNTIME` | 安装前缀（`CMAKE_INSTALL_PREFIX`）；protobuf 头文件的查找根；非 Windows 平台其 `lib/` 也进入链接搜索路径 |

Windows（cmd）：

```bat
set MSF_RUNTIME=d:/runtime
```

Linux / macOS：

```bash
export MSF_RUNTIME=$HOME/runtime
```

### 2.2 依赖清单

依赖声明在根目录 `conanfile.py`，版本固定在 Conan Center 可用版本：

| 包 | 版本 | CMake target |
|---|---|---|
| openssl | 3.6.4 | `OpenSSL::SSL` / `OpenSSL::Crypto` |
| libiconv | 1.19 | `Iconv::Iconv` |
| zlib | 1.3.2 | `ZLIB::ZLIB` |
| minizip | 1.2.12 | `minizip::minizip` |
| yaml-cpp | 0.9.0 | `yaml-cpp::yaml-cpp` |
| mimalloc | 3.5.1 | `mimalloc-static`（静态包） |
| cpr | 1.14.2 | `cpr::cpr` |
| capnproto | 1.5.0 | `CapnProto::capnp` / `CapnProto::capnp-json` |
| benchmark | 1.9.5 | `benchmark::benchmark` / `benchmark::benchmark_main` |
| catch2 | 3.16.0 | `Catch2::Catch2` / `Catch2::Catch2WithMain` |
| gtest | 1.18.0 | `GTest::gtest` / `GTest::gtest_main` / `GTest::gmock` |
| fmt | 11.2.0 | `fmt::fmt` |
| spdlog | 1.15.3 | `spdlog::spdlog`（外部 fmt） |
| asio | 1.38.2 | `asio::asio` |
| pfr | 2.2.0 | `pfr::pfr`（header-only；头文件仍为 `<boost/pfr.hpp>`、命名空间 `boost::pfr`） |
| backward-cpp | 1.6 | `Backward::Backward` |
| bshoshany-thread-pool | 5.1.0 | `bshoshany-thread-pool::bshoshany-thread-pool` |
| croncpp | 2023.03.30 | `croncpp::croncpp` |
| csv2 | 0.2 | `csv2::csv2`（本地配方） |
| date | 3.0.5 | `date::date` |
| cpp-httplib | 0.65.0 | `httplib::httplib` |
| magic_enum | 0.9.8 | `magic_enum::magic_enum` |
| mio | cci.20230303 | `mio::mio` |
| nlohmann_json | 3.11.3 | `nlohmann_json::nlohmann_json` |
| tsl-robin-map | 1.4.0 | `tsl::robin_map` |
| protobuf | 3.21.12 | `protobuf::protobuf` |
| xtl | 0.8.0 | `xtl` |
| xsimd | 13.2.0 | `xsimd` |
| xtensor | 0.27.1 | `xtensor` |

统一使用静态库（`*:shared=False`）。说明：

- **csv2 不在 Conan Center**：配方在 `third_party/recipes/csv2`，首次构建前执行
  `conan create third_party/recipes/csv2` 放入本地缓存，之后 `conan install` 才能解析。
  该配方在打包时修复 v0.2 的引号解析：字段最外层的一对 `"` 会被剥离，内部的 `""` 折叠为
  `"`；单引号不参与 CSV 转义，原样保留。
- **mpb 不在 Conan 里**：`quant1x/mpb` 是与本仓库同级的组织内项目，CMake 用
  `QUANT1X_MPB_DIR`（默认 `../mpb`）把源码直接编入 `quant1x`；Meson 同样按相对路径编译。
  可用 `-DQUANT1X_MPB_DIR=<path>` 覆盖。
- **反射用独立的 `pfr`**：依赖从 `boost/1.90.0`（`header_only`）换成 standalone `pfr/2.2.0`，不再引入整套 Boost；该包仍以 `boost/pfr.hpp` 与 `boost::pfr` 命名空间提供，代码侧无需改动。
- **minizip 为 1.2.12**：Conan Center 未提供 1.3.x；现有代码只使用 `unz*` API，兼容。
- **openssl 关闭 FIPS 模块**（`no_fips=True`）：项目不使用 FIPS，关闭后显著缩短源码构建时间。

### 2.3 Windows / MSVC profile

Windows 固定使用 `conan/profiles/windows-msvc-static`：

```ini
[settings]
os=Windows
arch=x86_64
build_type=Release
compiler=msvc
compiler.version=194
compiler.cppstd=20
compiler.runtime=static
compiler.runtime_type=Release

[options]
*:shared=False
minizip/*:bzip2=False
openssl/*:no_fips=True
```

要点：

- `compiler.runtime=static` 对应 `/MT`，与项目 `USE_STATIC_CRT=ON`（`cmake/compile_options.cmake`）必须保持一致，否则会出现 `LNK2038 RuntimeLibrary mismatch`；
- `compiler.version` 按本机 VS 工具集调整（`cl` 版本 19.4x 对应 `194`）；
- **Debug 构建**：复制该 profile，把 `build_type` 与 `compiler.runtime_type` 改为 `Debug`，并使用独立构建目录（如 `build-msvc-debug`）。同一构建目录不能混用 Debug/Release 依赖。

Linux / macOS 直接使用 `conan profile detect` 生成的默认 profile，`conanfile.py` 会强制静态库选项。profile 中的编译器必须与平台策略一致（macOS 为 `apple-clang`，Linux 为 `gcc`），可用 `conan profile show` 确认。

### 2.4 配置与构建

Windows（MSVC，Release）：

```bat
conan install . -of build-msvc -pr:h conan/profiles/windows-msvc-static -pr:b default -s build_type=Release --build=missing

cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DCMAKE_TOOLCHAIN_FILE=build-msvc/conan_toolchain.cmake

cmake --build build-msvc --config Release -j
```

也可以直接运行封装脚本：

```bat
scripts\msvc_configure.bat
```

Linux / macOS（Ninja）：

```bash
conan install . -of build -pr:b default -s build_type=Release --build=missing
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
cmake --build build -j
```

说明：

- `CMakeLists.txt` 会校验 `CMAKE_TOOLCHAIN_FILE` 是否以 `conan_toolchain.cmake` 结尾，未通过 Conan 配置时直接报错并给出上述命令；
- Windows 下会关闭 Visual Studio 自带的 vcpkg 集成（`VcpkgEnabled=false`）并全局定义 `NOMINMAX`，避免系统级 vcpkg 头文件注入以及 `windows.h` 的 `min/max` 宏与 `std::min/std::max` 冲突；
- `CMAKE_BUILD_TYPE` 必须与 `conan install -s build_type=...` 一致；
- Conan Center 没有 MSVC 静态运行库（`/MT`）的预编译包，首次 `conan install --build=missing` 会从源码构建 openssl、libiconv、libcurl、cpr 等依赖，耗时较长；后续命中本地 Conan 缓存；
- 如需强制清理 CMake 缓存后重配，使用 `cmake --fresh`。

### 2.5 构建产物

| 产物 | 路径 |
|---|---|
| 命令 `q1x` | `build-msvc/Release/q1x.exe`（单配置生成器为 `build/q1x`） |
| 静态库 | `build-msvc/Release/quant1x.lib`；单配置生成器为 `build/quant1x.a`（Debug 带 `DEBUG_POSTFIX`） |
| 测试可执行 | `build-msvc/tests/Release/*`（单配置生成器为 `build/tests/app-*`、`catch2-*`、`gtest-*`） |
| 基准可执行 | `build-msvc/benches/Release/benchmark-*` |
| 示例可执行 | `build-msvc/examples/Release/*` |

### 2.6 常用 CMake 选项

| 选项 | 默认 | 说明 |
|---|---|---|
| `CMAKE_BUILD_TYPE` | `Debug` | 需与 Conan 的 `build_type` 一致 |
| `USE_STATIC_CRT` | `ON` | MSVC 静态 CRT（`/MT`），必须与 Conan profile 的 `compiler.runtime=static` 一致 |
| `FAST_CRON_TEST` | `OFF` | 打开后缩短 cron 测试时序，用于快速回归 |
| `ENABLE_MIMALLOC` | `OFF` | 启用 mimalloc；默认只重载 C++ `new/delete` |
| `MIMALLOC_MODE` | `new_delete` | 设为 `global` 时重载 C 分配器（`MI_OVERRIDE=1`） |
| `ENABLE_MIMALLOC_CHECK` | `OFF` | 额外构建 `mimalloc_check` 运行时校验程序 |

编译策略：

- Release 下自动尝试 LTO（`/GL`，编译器支持时启用）；
- MSVC 为 Release `/arch:AVX2`、Debug `/arch:AVX`；GCC / Clang 使用 `-march=native` 等选项；
- 带 `-march=native` 的 GCC/Clang 二进制不可跨机分发，发布构建应在目标平台编译。

### 2.7 测试

测试默认不参与普通构建（全部 `EXCLUDE_FROM_ALL`），先构建 `tests` 聚合目标：

```bat
rem 构建全部测试目标（4 个 Catch2 分组 + app-* + gtest-*）
cmake --build build-msvc --config Release --target tests -j 8

rem 列出 / 运行
ctest --test-dir build-msvc -C Release -N
ctest --test-dir build-msvc -C Release --output-on-failure -j 8

rem 按名称过滤
ctest --test-dir build-msvc -C Release -R tdd-bars -V
```

也可以直接运行测试进程：

```bat
build-msvc\tests\Release\catch2-num.exe "某个测试名"
build-msvc\tests\Release\app-test-ringbuffer.exe
```

分组约定（`tests/CMakeLists.txt` 与 `meson.build` 保持同一份清单）：

| 可执行 | 覆盖 |
| --- | --- |
| `catch2-num` | 公式 / 指标 / 数值 / DataFrame |
| `catch2-data` | 行情、编解码、协议、时间与日历 |
| `catch2-runtime` | 基础组件、日志、网络与调度 |
| `catch2-contrib` | 特色数据、回测、策略与实盘 |

为什么不一个文件一个可执行：每个可执行都要过一遍 1 GB 级的 `quant1x` 静态库，60 多次链接
是纯浪费；合并后仍保留用例级粒度（`catch_discover_tests` 每条 `TEST_CASE` 一个 ctest 条目、
独立进程）。`tdd-ta.cpp`（gtest）仍按旧 `ta::Series` 接口编写，编译不过，暂未纳入，修复后再加回。

部分用例依赖本地数据缓存、网络或平台特性，缺数据时用例内部 `SKIP` 而不是失败。

### 2.8 基准测试

```bat
cmake --build build-msvc --config Release --target benchmark-vyukov_bench -j
build-msvc\benches\Release\benchmark-vyukov_bench.exe
```

`benches/` 与 Rust 侧 `benches/vyukov_bench.rs` 参数对齐，用于跨语言性能对比。

### 2.9 安装

```bat
cmake --install build-msvc --config Release
```

安装前缀固定为 `MSF_RUNTIME`，安装内容：

- `bin/`：`q1x`
- `lib/`：`quant1x` 静态库（含 mpb 编译产物）
- `include/quant1x/`：项目头文件 + 生成的 `build-info.h`
- `lib/cmake/quant1x/`：`quant1x-config.cmake`、`quant1x-targets.cmake`

下游通过 `find_package(quant1x)` 使用时，也需要先执行 `conan install` 让依赖的 config 文件位于 `CMAKE_PREFIX_PATH`（这是外部依赖管理的通用约束）。

### 2.10 新增 C/C++ 源文件

根 `CMakeLists.txt` **显式列举**了全部头文件（`quant1x-headers`）与源文件（`quant1x-sources`），没有使用 `file(GLOB)`。因此：

- 新增 `.cpp` 必须加入 `quant1x-sources`；
- 新增对外头文件必须加入 `quant1x-headers`；
- 新增测试需在 `tests/CMakeLists.txt` 中按类型登记（`add_catch2_executable` / `add_gtest_executable` / `add_app_executable`）；
- 新增基准需在 `benches/CMakeLists.txt` 中登记 `add_benchmark_executable`；
- 新增第三方依赖时，在 `conanfile.py` 的 `requirements()` 中声明，并在 CMake 中链接其 imported target。

---

### 2.11 Meson 构建（可选）

CMake 仍是主构建；Meson 覆盖 `quant1x` 库、`q1x` 与两个跨语言对拍程序。

```bash
# 1) 依赖：与 CMake 共用 conanfile.py，额外生成 Meson 原生文件与 pkg-config 文件
conan install . -of build-meson -pr:h conan/profiles/windows-msvc-static \
    -pr:b default -s build_type=Release --build=missing \
    -g MesonToolchain -g PkgConfigDeps

# 2) 配置（Windows 需先进入 VS 开发者环境）
meson setup build-meson/build --native-file build-meson/conan_meson_native.ini

# 3) 编译与测试
meson compile -C build-meson/build
meson test -C build-meson/build
```

说明：

- 依赖通过 Conan 生成的 `.pc` 文件解析，`MesonToolchain` 已把 `pkg_config_path` 指向 `build-meson`；
- `mpb` 按 `../mpb` 相对路径直接编译（与 CMake 的 `QUANT1X_MPB_DIR` 默认值一致）；
- 测试与 CMake 分组一致：4 个 `catch2-*` 分组合并、`app-*` 独立、gtest 独立，全部
  `build_by_default: false`——`meson compile` 只编库与 `q1x`，`meson test` 按需构建并运行。
  Meson 的 `test()` 是整包粒度（Catch2 无 TAP 协议），要跑单条用例直接调
  `build-meson/build/catch2-num.exe "用例名"`。

### 2.12 IDE 与静态检查（clangd / clang-tidy）

仓库根目录的 `.clangd` 把 clangd 指向 Meson 生成的编译数据库，并兜底 `-std=c++20`：

```yaml
CompileFlags:
  CompilationDatabase: build-meson/build
  Add: [-std=c++20]
```

为什么需要它：clangd 在找不到 `compile_commands.json` 时会退到 `-std=c++14`，于是本仓库
大量使用的嵌套命名空间（`namespace quant1x::pandas`）被判成 “nested namespace definition
is a C++17 extension”，一次能报出数百条误诊。项目的真实基线是 C++20
（CMake `CMAKE_CXX_STANDARD=20`、Meson `cpp_std=c++20`）。

配套约定：

- 编译数据库由 §2.11 的 `meson setup build-meson/build ...` 生成；CMake 的 Visual Studio
  生成器不产出 `compile_commands.json`，想用 CMake 的编译参数请改用 Ninja 生成器并加
  `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`，再把 `.clangd` 的 `CompilationDatabase` 指过去。
- 没有编译数据库时 `Add: [-std=c++20]` 仍然生效，嵌套命名空间的误报会消失，但 Conan 依赖
  的头文件路径缺失，会出现 “file not found” 噪声；此时补数据库而不是改代码。
- `clang-tidy` 读取根目录 `.clang-tidy`，需要编译参数时用
  `clang-tidy -p build-meson/build <file>`。
- 不要把嵌套命名空间改写成 `namespace a { namespace b {` 来迁就工具：项目基线是 C++20，
  改写只会增加缩进层级。

## 3. Rust 构建

Rust 实现与 Python spec 语义一致、与 C++ 实现逐项对位，库入口为 `quant1x/lib.rs`，命令行入口为 `main.rs`。

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
go env -w GOPRIVATE=github.com
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

Java 部分只承载分布式 ID 与 LMAX Disruptor 等对照实现/基准，不参与核心链路（Python / C++ / Rust / Go）。

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

## 7. Zig（扩展）

Zig 侧与 Go 同属扩展语言，当前只承载小功能实现（如 `quant1x/base/argparser`），既没有 `build.zig` 也不产出
安装物：源码与用例放在同一目录，直接以 `zig test` 执行。

```bash
zig test quant1x/base/argparser/parser_test.zig
zig test quant1x/base/argparser/help_test.zig
zig test quant1x/base/argparser/complete_test.zig
zig fmt --check quant1x/base/argparser/*.zig
```

- 最低版本 0.17：文件读写等 API 自 0.17 起要求显式传入 `Io`；本地 0.17.1 实测通过。
- 新增 `.zig` 文件不需要登记构建清单；将来某模块引入 `build.zig` 时，在本节补充该模块的构建与用例命令。

---

## 8. 跨语言一致性验证

构建完成后，各语言实现应能对同一输入给出 epsilon 内一致的结果。验证方式：

1. 准备同一组输入与黄金输出（`tests/` 下的 fixture、CSV、JSON，或由 Python 生成的基准数据）；
2. 分别运行各语言实现；
3. 对浮点结果按约定的 epsilon 比较，对结构体按字段逐一比较（字段名必须是全小写 `snake_case`）。

可参考现有 parity 用例：

- 日历解码：`tests/calendar_decode_test.go`、`tests/calendar_decode.js`、`tests/calendar_integration.rs`
- Ringbuffer 性能对齐：`quant1x/runtime/ringbuffer.md`、`benches/vyukov_bench.{cpp,rs}`

新增功能时，按 Python → C++ → Rust / Go 的顺序落地（Zig 扩展项随其承载的功能同步），并在 `tests/` 中补齐对应语言的验证入口。

---

## 9. 常见问题

| 现象 | 原因与处理 |
|---|---|
| `未使用 Conan 工具链配置本项目` | 配置时未传 `-DCMAKE_TOOLCHAIN_FILE=<build>/conan_toolchain.cmake`，或尚未执行 `conan install` |
| `NOT found ENV MSF_RUNTIME` | 未设置 `MSF_RUNTIME`；它同时是安装前缀 |
| 链接阶段找不到 protobuf | protobuf 由 Conan 的 `protobuf/3.21.12` 提供；先确认 `conan install` 成功，且 `protobuf::protobuf` 已链接 |
| `LNK2038 RuntimeLibrary mismatch` | MSVC profile 的 `compiler.runtime` 与 `USE_STATIC_CRT` 不一致；统一为 `static`（`/MT`） |
| CMake 报找不到 `minizip` / `Iconv` / `ZLIB` 的 config | `conan install` 未执行，或输出目录与 `CMAKE_TOOLCHAIN_FILE` 所在目录不一致 |
| 首次 `conan install` 构建极慢 | MSVC 静态运行库无 CCI 预编译包，openssl/libiconv/libcurl 会从源码构建；属正常现象 |
| 二进制在别的机器上报非法指令 | GCC / Clang 构建默认带 `-march=native`，需在目标平台构建或去掉该选项 |
| `ctest` 报 `No tests were found` | 未构建测试目标；先构建，VS 生成器需指定 `-C Release` |
| Rust 报 `invalid metadata version` | `rust-analyzer` 与工具链版本不一致；让 rustup 按 `rust-toolchain.toml` 重新安装组件 |
| Rust 基准 `running 0 tests` | `[[bench]]` 的 `harness = false` 被改动，导致 libtest 覆盖了 Criterion 的 `main` |
| `pip install -e .` 版本变成 `0.0.0` | 仓库没有可达的 Git Tag；`setuptools_scm` 回退到 `fallback_version` |
| 提交被 pre-commit 拒绝 | 新文件混入了 BOM，按提示去掉 BOM 后重新提交 |
| Java 报 `package tests.xxx does not match ...` | 包名未按 `tests/` 根层级书写；不得额外加 `quant1x.` 前缀 |

---

## 10. 版本与发布约束

- 项目版本由 Git Tag（`vMAJOR.MINOR.PATCH`）唯一决定，C++ 在配置阶段通过 `cmake/git_tag_info.cmake` 读取，Python 通过 `setuptools_scm` 推导。
- `Cargo.toml`、`pom.xml`、`pyproject.toml` 中的版本字段与 `CHANGELOG.md` **不得手工修改**，发布统一走 `autochangelog`。
- 提交信息使用中文，主题格式为 `<type>[<language>]：<summary>`，具体规则见 `CONTRIBUTING.md` 第 8 节。

---

## 11. 相关文档

| 文档 | 内容 |
|---|---|
| [README.md](README.md) | 项目概览、快速开始、使用示例 |
| [CONTRIBUTING.md](CONTRIBUTING.md) | 规范源：语言角色、目录布局、命名与契约、注释、文档、测试、提交、工作准则 |
| [AGENTS.md](AGENTS.md) | AI 入口索引：必读顺序、硬约束、主题索引、常用命令、交付自检 |
| [docs/README.md](docs/README.md) | 数据项与更新时序 |
| [tests/README.md](tests/README.md) | 测试约定与 parity 复现步骤 |
| [examples/README.md](examples/README.md) | 示例程序说明 |
