# Quant1X 量化交易实验室

![Quant1X Logo](logo.png)

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![Go](https://img.shields.io/badge/Go-1.27-blue.svg)](https://golang.org/)
[![Rust](https://img.shields.io/badge/Rust-1.98.1+-orange.svg)](https://www.rust-lang.org/)
[![Python](https://img.shields.io/badge/Python-3.12+-yellow.svg)](https://www.python.org/)
[![CMake](https://img.shields.io/badge/CMake-3.30+-green.svg)](https://cmake.org/)
![tag](https://img.shields.io/github/tag/quant1x/quant1x.svg?style=flat)
![Crates.io](https://img.shields.io/crates/d/quant1x.svg)

Quant1X 是一个多语言平行实现的量化交易框架，同时提供 C++、Go、Rust、Python 四种语言的等价实现，覆盖数据处理、因子与指标、策略开发、回测引擎、实时行情与交易等完整链路。

四种语言**互不依赖、不跨语言调用**（无 FFI / Cgo / SWIG），通过同名文件、同名函数与统一的 I/O 契约保持语义一致，可通过同一组测试向量交叉验证。

## 目录

- [核心特性](#核心特性)
- [环境要求](#环境要求)
- [项目结构](#项目结构)
- [快速开始](#快速开始)
  - [1. 获取源码](#1-获取源码)
  - [2. 配置文件](#2-配置文件)
  - [3. Go](#3-go)
  - [4. Rust](#4-rust)
  - [5. C/C++](#5-cc)
  - [6. Python](#6-python)
- [附录：开发环境搭建](#附录开发环境搭建)
- [多语言一致性](#多语言一致性)
- [使用示例](#使用示例)
- [文档索引](#文档索引)
- [贡献](#贡献)
- [许可证](#许可证)
- [相关链接](#相关链接)

## 核心特性

- **多语言平行实现**：同一功能在 C++、Go、Rust、Python 中各有独立实现，语义对齐、命名一致
- **高性能**：优化的算法与数据结构，支持 SIMD，关键路径（如 Vyukov MPMC 队列）跨语言性能对齐
- **模块化设计**：按功能划分目录，可插拔组件架构，易于扩展
- **行情与数据**：支持 Level1 行情、K 线、除权除息、板块、F10 等数据集及其缓存更新
- **策略与因子框架**：内置多种技术指标、因子计算与策略模板
- **回测引擎**：高效回测系统，支持多资产组合
- **网络通信**：内置 HTTP / WebSocket 客户端，支持多种协议
- **文档即契约**：重要模块均配同级 `.md` 文档，作为跨语言实现与 AI 辅助开发的依据

> 详细的架构理念、目录规范与命名规范见 [CONTRIBUTING.md](CONTRIBUTING.md)，AI 协作规则见 [AGENTS.md](AGENTS.md)。

## 环境要求

### 系统要求

- 64 位操作系统（Windows / Linux / macOS）
- 至少 8GB 内存，推荐 16GB 以上
- 支持 AVX2 指令集的 CPU

### 语言版本

| 语言 | 版本要求 | 推荐版本 |
|---|---|---|
| Python | 3.12+ | 3.12.x |
| Go | 1.27+ | 1.27.x |
| Rust | 1.98.1+ | 1.98.1+（Edition 2024） |
| C++ | C++20 | GCC 13+ / Clang 17+ / MSVC 14.3+ |

C/C++ 构建另需 CMake 3.30+，推荐 Ninja 作为构建器。

## 项目结构

```text
quant1x/
├── quant1x/            # 各语言的平行实现（按功能划分目录）
│   ├── data/           # 行情与基础数据（K线、除权除息、板块、F10 等）
│   ├── factors/        # 因子与特征计算
│   ├── indicators/     # 技术指标
│   ├── ta/             # 技术分析算子
│   ├── formula/        # 公式引擎（含 MyTT 兼容实现）
│   ├── backtest/       # 回测引擎
│   ├── trader/         # 交易相关
│   ├── realtime/       # 实时行情
│   ├── runtime/        # 并发与运行时组件（含 Vyukov Ringbuffer）
│   ├── encoding/       # 二进制编码与结构化序列化
│   ├── id/             # 分布式 ID
│   ├── distributed/    # 分布式协调
│   ├── learn/          # 机器学习相关（FPGrowth、regime 等）
│   ├── config/         # 配置
│   ├── io/ log/ net/ util/ ...
├── docs/               # 文档（数据字典、交易规则等）
├── examples/           # 示例程序与示例配置
├── tests/              # 跨语言一致性测试（含 Java 用例）
├── benches/            # 基准测试（Vyukov 队列等）
├── proto/              # 协议定义
├── scripts/ labs/      # 脚本与实验 notebook
```

同一功能在 `quant1x/<module>/` 下以 `xxx.py`、`xxx.h` / `xxx.cpp`、`xxx.rs` 平行存在，Go 实现位于同名子目录（如 `data/t1/api.go`，包名为 `t1`），以此规避 Go 的循环依赖限制。

## 快速开始

### 1. 获取源码

```shell
git clone https://gitee.com/quant1x/quant1x.git
cd quant1x
```

### 2. 配置文件

示例配置位于 `examples/quant1x.yaml`，按语言拷贝到对应的用户目录即可使用。配置分为**交易、策略、数据**三个部分，可自定义缓存路径。

| 语言 | 配置目录 | 安装命令 |
|---|---|---|
| Go | `~/.quant1x/` | `cp examples/quant1x.yaml ~/.quant1x/quant1x.yaml` |
| C++ | `~/.q1x/` | `cp examples/quant1x.yaml ~/.q1x/quant1x.yaml` |
| Rust | `~/.q1x-rs/` | `cp examples/quant1x.yaml ~/.q1x-rs/quant1x.yaml` |
| Python | — | 无独立二进制数据，复用 Go / C++ / Rust 导出的数据 |

Python 侧数据源默认读取 Go 版本的配置；如需切换，在开发环境的 `.env` 或环境变量中设置 `QUANT1X_WORK`，取值与目录对应：**C++ 为 `q1x`，Rust 为 `q1x-rust`**（不含点号）。

### 3. Go

Go 版本是目前较为稳定的生产版本。

```shell
# 构建全部包
go build ./...

# 运行测试
go test ./...

# 运行主程序
go run main.go --help
```

### 4. Rust

Rust 版本以 C++ 版本为基础，尽可能 1:1 还原 C++ 的业务逻辑。仓库在 `Cargo.toml` 中声明了可执行二进制 `q1x`（入口 `main.rs`）。

```shell
# Debug 构建
cargo build --bin q1x

# Release 构建（生产推荐）
cargo build --release --bin q1x

# 查看帮助
cargo run --bin q1x -- --help
```

常用子命令：

```shell
# 服务管理（install / uninstall / start / stop / status / run）
cargo run --bin q1x -- service install
cargo run --bin q1x -- service start

# 更新缓存数据（all / calendar / servers / base / features）
cargo run --bin q1x -- update --all
cargo run --bin q1x -- update --calendar
```

也可以直接运行构建产物（Windows 示例）：

```shell
.\target\release\q1x.exe --help
```

帮助输出示例（版本与子命令随实现演进，以实际构建为准）：

```text
quant1x - Rust edition
--------------------------------------------------------------------------------
         Version : <构建版本>
        Author : Quant1X Team
--------------------------------------------------------------------------------

Usage: q1x [OPTIONS] [COMMAND]

Commands:
  service  Manage the service.
  update   Update cached data (base / features)
  help     Print this message or the help of the given subcommand(s)

Options:
    --version
      Print build version information and exit

    --verbose
      显示日志信息到终端

    --debug
      打开日志的调试模式

  -h, --help
      Print help (see a summary with '-h')
```

注意事项：

- `q1x` 二进制是 crate 的前端入口，会调用库中暴露的初始化与子命令钩子（如 `quant1x::global_init`、`quant1x::datasets_init`、`quant1x::engine_daemon`）。从源码运行时请确保启用所需 feature。
- Windows 上可通过 `service` 子命令配合 `--pipe`、`--elevated-out`、`--elevated-pipe` 以服务 / 守护进程模式运行。
- CI 或部署场景请优先使用 `--release` 构建，并在目标平台上编译，避免 `-march=native` 引入不可移植的指令集。

### 5. C/C++

C/C++ 版本具备跨平台的生产能力，代码以 C++20 为目标。

跨平台构建（Ninja + Clang/GCC，推荐）：

```shell
mkdir -p build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20 ..
cmake --build . --config Release
```

Visual Studio（Windows）：

```shell
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

Debug 构建、安装、测试与运行：

```shell
cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -S . -B cmake-build-debug
cmake --build cmake-build-debug --target q1x -j 18
ninja -C cmake-build-debug install
ctest --test-dir cmake-build-debug --output-on-failure
./cmake-build-debug/bin/q1x --help
```

### 6. Python

Python 版本主要用于探索验证与数据分析，运行依赖 Go / C++ / Rust 导出的数据。

```shell
# 安装依赖
pip install -r requirements.txt

# 切换数据源（c++ 对应 q1x，rust 对应 q1x-rust）
export QUANT1X_WORK=q1x
```

## 附录：开发环境搭建

### Python

多版本共存推荐使用 pyenv（安装时避免使用 root 权限）：

```shell
brew install pyenv
pyenv versions          # 查看已安装版本
pyenv install -l        # 查看可安装版本
pyenv install 3.12.9    # 安装指定版本
```

基础工具：

| 工具 | 功能 |
|---|---|
| pip | 类库管理工具（类似 maven / gradle / go mod） |
| pip-autoremove | 自动删除类库及其全部依赖 |
| pipreqs | 项目 / 类库交叉依赖检测 |

配置 pip 镜像源（清华源示例）：

```shell
pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple
pip config set install.trusted-host https://pypi.tuna.tsinghua.edu.cn
```

依赖管理（`requirements.txt` 类似 Java 的 `pom.xml`、Go 的 `go.mod`）：

```shell
pip install -r requirements.txt   # 安装依赖
pip freeze > requirements.txt     # 导出依赖
pip install pipreqs && pipreqs ./ --encoding utf8   # 交叉依赖检测
```

打包与发布到 PyPi：

```shell
pip install --upgrade setuptools wheel twine
python setup.py sdist bdist_wheel
twine upload --repository testpypi dist/*   # 测试环境
twine upload dist/*                          # 正式环境
```

Matplotlib 中文乱码处理：

1. 获取配置文件路径：`python -c "import matplotlib; matplotlib.matplotlib_fname()"`
2. 将中文字体（如 `SimHei.ttf`）复制到该路径下的 `fonts/ttf/` 目录
3. 编辑 `matplotlibrc`：启用 `font.sans-serif` 并填入 `SimHei`；将 `axes.unicode_minus` 设为 `False`
4. 删除 `~/.matplotlib` 下的缓存文件（`tex.cache`、`fontList.json` 等）

### Go

```shell
go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct
go env -w GOPRIVATE=gitee.com
```

安装 protobuf 代码生成插件：

```shell
go install google.golang.org/protobuf/cmd/protoc-gen-go@latest
go install google.golang.org/grpc/cmd/protoc-gen-go-grpc@latest
```

### Rust

```shell
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh   # 安装
rustup update                                                    # 更新工具链
```

### C/C++

推荐工具链：

- 编译器：GCC 14.3+ / Clang 18+ / MSVC（Visual Studio 2022，工具集 14.3+）
- CMake 3.30+，构建器推荐 Ninja
- 包管理：vcpkg 或 Conan（可选）

常用依赖：OpenSSL（网络 / 加密）、protobuf（序列化）、fmt / spdlog（格式化与日志）。

系统依赖安装示例：

```shell
# Ubuntu / Debian
sudo apt update && sudo apt install -y build-essential cmake ninja-build clang pkg-config

# macOS
brew install cmake ninja llvm vcpkg
```

vcpkg 依赖安装示例：

```shell
vcpkg install yaml-cpp zlib asio xtensor mimalloc spdlog fmt duktape benchmark catch2 flatbuffers capnproto
```

protobuf 建议使用 3.21.11（更高版本依赖 abseil，会引入额外复杂度）：

```shell
wget https://gh-proxy.com/github.com/protocolbuffers/protobuf/releases/download/v21.11/protobuf-cpp-3.21.11.zip
unzip protobuf-cpp-3.21.11.zip && cd protobuf-3.21.11
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=$HOME/runtime -Dprotobuf_BUILD_TESTS=OFF -G "Unix Makefiles" ../
make -j$(nproc) && make install
```

Windows（MSVC）示例：

```powershell
mkdir build; cd build
cmake -DCMAKE_INSTALL_PREFIX=d:/runtime -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

建议的编译选项：Release 使用 `-O3 -DNDEBUG`；可选启用 LTO 与目标特定指令集（`-march=native` 仅在目标机测试时启用）。

调试与分析：

- 调试构建中开启 sanitizer（`AddressSanitizer` / `ThreadSanitizer`）以捕获内存与线程错误
- 使用 perf / VTune / Windows Performance Analyzer 做性能剖析

平台注意事项：

- Windows：选择 x64 构建，使用 vcpkg triplet（如 `x64-windows`）安装二进制依赖
- Linux：容器 / CI 中固定基础镜像（如 `ubuntu:22.04`）以保证可重复构建
- macOS：使用 Homebrew 管理依赖，注意 Apple Clang 与 LLVM Clang 的差异

编译器选择：在并发热点代码上，Clang / MSVC 生成的汇编有时优于部分 GCC 版本；追求极致性能时建议在 CI 中做对比测试，同时在主流水线中覆盖 GCC / Clang / MSVC 全部目标编译器。

## 多语言一致性

四语言实现遵循同一份文档契约：同一功能目录下存在同名文件与同名函数签名，I/O 边界统一为全小写 `snake_case` 字段，通过同一组测试向量交叉验证数值结果（epsilon 范围内一致）。

### 性能对齐：Vyukov Ringbuffer

`quant1x/runtime/ringbuffer.h`（C++）与 `quant1x/runtime/ringbuffer.rs`（Rust）是 Vyukov 有界 MPMC 队列的平行实现（Go 见 `ringbuffer.go`），基准位于 `benches/vyukov_bench.{cpp,rs}`，详见 [ringbuffer.md](quant1x/runtime/ringbuffer.md)。

已完成的关键性能对齐（Apple Silicon / arm64 实测）：

| 修复项 | 内容 | 效果 |
|---|---|---|
| aarch64 退避指令 | `yield` → `isb`，与 Rust `core::hint::spin_loop` 逐字节对齐 | 8P8C 整轮 4855µs → 1946µs |
| ctor 槽区分配 | `std::make_unique<Slot[]>` 清零 → `aligned_alloc(64)` + placement new 只写 seq，对齐 Rust `Vec::with_capacity` 不初始化语义 | ctor ~246µs → ~185µs（Rust ~210µs） |
| 基准 harness 计数器 | 共享原子 `consumed.fetch_add()` → 每线程本地计数、join 后合并，消除复合缓存行争用伪影 | 8P8C 消费速率与 backlog 与 Rust 完全对齐（160 vs 159 M/s） |

修复后 uncontended 吞吐 C++ 107~131 M/s，Rust 121~125 M/s（噪声带内对齐）；队列自身差异均已闭合，无已知性能缺陷。

## 使用示例

获取前复权 K 线数据（同一功能在三种语言中的等价调用）：

### Python

```python
from quant1x.factors.base import get_cross_section_forward_adjusted_bars

# 获取前复权K线数据
code = "sh600000"
as_of_date = "2024-12-26"
bars = get_cross_section_forward_adjusted_bars(code, as_of_date)
print(f"Loaded {len(bars)} adjusted bar records for {code}")

# 显示最近5条记录
for bar in bars[-5:]:
    print(f"Date: {bar.date}, Open: {bar.open:.2f}, Close: {bar.close:.2f}")
```

### Go

```go
package main

import (
    "fmt"
    "gitee.com/quant1x/quant1x/quant1x/factors"
)

func main() {
    code := "sh600000"
    asOfDate := "2024-12-26"

    // 获取前复权K线数据
    bars := factors.GetCrossSectionForwardAdjustedBars(code, asOfDate)

    fmt.Printf("Loaded %d adjusted bar records for %s\n", len(bars), code)

    // 显示最近5条记录
    start := len(bars) - 5
    if start < 0 {
        start = 0
    }
    for _, bar := range bars[start:] {
        fmt.Printf("Date: %s, Open: %.2f, Close: %.2f\n", bar.Date, bar.Open, bar.Close)
    }
}
```

### Rust

```rust
use quant1x::factors::base::get_cross_section_forward_adjusted_bars;

fn main() {
    let code = "sh600000";
    let as_of_date = "2024-12-26";

    // 获取前复权K线数据
    let bars = get_cross_section_forward_adjusted_bars(code, as_of_date);

    println!("Loaded {} adjusted bar records for {}", bars.len(), code);

    // 显示最近5条记录
    let start = if bars.len() > 5 { bars.len() - 5 } else { 0 };
    for bar in &bars[start..] {
        println!("Date: {}, Open: {:.2}, Close: {:.2}",
                bar.date, bar.open, bar.close);
    }
}
```

## 文档索引

| 分类 | 文档 |
|---|---|
| 架构与开发规范 | [CONTRIBUTING.md](CONTRIBUTING.md) |
| AI 协作规则 | [AGENTS.md](AGENTS.md) |
| 数据字典（数据项与更新时序） | [docs/README.md](docs/README.md) |
| 模块库说明 | [docs/library.md](docs/library.md) |
| 命令行参数 | [docs/argparse.md](docs/argparse.md) |
| 交易所费用 | [docs/exchange-license-fees.md](docs/exchange-license-fees.md) |
| Vyukov Ringbuffer 契约 | [quant1x/runtime/ringbuffer.md](quant1x/runtime/ringbuffer.md) |
| 技术分析算子 | [quant1x/ta/README.md](quant1x/ta/README.md) |
| 分布式 ID（64/128 位） | [quant1x/id/id64/README.md](quant1x/id/id64/README.md)、[quant1x/id/id128/README.md](quant1x/id/id128/README.md) |
| 结构化二进制编码 | [quant1x/encoding/binary/cstruct/README.md](quant1x/encoding/binary/cstruct/README.md) |
| 市场状态识别 | [quant1x/learn/regime/README.md](quant1x/learn/regime/README.md) |

## 贡献

欢迎贡献代码，流程如下：

1. Fork 本项目
2. 创建特性分支（`git checkout -b feature/AmazingFeature`）
3. 提交更改（`git commit`）
4. 推送到分支（`git push origin feature/AmazingFeature`）
5. 创建 Pull Request

### 开发规范

- 遵循各语言的编码规范，以及 [CONTRIBUTING.md](CONTRIBUTING.md) 中的目录与命名约定
- 新增功能优先补齐 Python 实现与同级 `.md` 文档，再跟进其他语言
- 添加单元测试，并保证同一功能在四种语言下的结果一致
- 更新相关文档，确保所有测试通过

### 提交信息规范

提交信息使用中文，主题格式：`<type>[<language>]：<summary>`

- `type`：`feat` / `fix` / `refactor` / `perf` / `test` / `docs` / `chore`
- `language`：`python` / `cpp` / `rust` / `go` / `multi`，跨语言用 `python+cpp` 形式组合
- 示例：`feat[python]：为 runtime ringbuffer 增加原生 Python 语义实现`

## 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 相关链接

- [文档](docs/)
- [示例](examples/)
- [问题跟踪](https://gitee.com/quant1x/quant1x/issues)
- [讨论区](https://gitee.com/quant1x/quant1x/discussions)

---

**注意**：本项目仅用于学习和研究目的，不构成投资建议。在实际交易前，请充分了解和评估各种风险并咨询专业人士。
