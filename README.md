# Quant1X 量化交易实验室

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![Go](https://img.shields.io/badge/Go-1.25-blue.svg)](https://golang.org/)
[![Rust](https://img.shields.io/badge/Rust-1.90+-orange.svg)](https://www.rust-lang.org/)
[![Python](https://img.shields.io/badge/Python-3.12+-yellow.svg)](https://www.python.org/)
[![CMake](https://img.shields.io/badge/CMake-3.30+-green.svg)](https://cmake.org/)
![tag](https://img.shields.io/github/tag/quant1x/quant1x.svg?style=flat)
![Crates.io](https://img.shields.io/crates/d/quant1x.svg)

Quant1X 是一个多语言量化交易框架，支持 C++、Go、Rust、Python，提供完整的量化交易解决方案，包括数据处理、策略开发、回测引擎、实时交易等功能。

## ✨ 特性

- **多语言支持**：C++20、Go 1.25、Rust 1.90+、Python 3.12+
- **高性能**：优化的算法和数据结构，支持 SIMD 指令集
- **模块化设计**：可插拔的组件架构，易于扩展
- **实时数据**：支持 Level1 市场数据
- **策略框架**：内置多种技术指标和策略模板
- **回测引擎**：高效的回测系统，支持多资产组合
- **网络通信**：内置 HTTP/WebSocket 客户端，支持多种协议

## 📋 环境要求

### 系统要求

- 64位操作系统 (Windows/Linux/macOS)
- 至少 8GB RAM，推荐 16GB+
- 支持 AVX2 指令集的 CPU

### 语言版本

| 语言 | 版本要求 | 推荐版本 |
|------|----------|----------|
| Python | 3.12+ | 3.12.x |
| Go | 1.25+ | 1.25.x |
| Rust | 1.88+ | 1.90+ (2021) |
| C++ | C++20 | GCC 14.3+/Clang 18+/MSVC 14.3+ |

## 🚀 快速开始


# 1. 环境配置
## 1.1 默认均为64位操作系统
| python | golang | rust       | c++                         |
|:-------|:-------|:-----------|:----------------------------|
| 3.12.x | 1.25.x | 1.90+/2021 | gcc14.3+/clang18+/msvc14.3+ |

## 1.2 环境安装推荐使用brew
安装brew时需要注意避免使用root权限 


# 2. python

python的运行环境可能存在多个版本冲突的问题，那么怎么来解决多版本的共存的问题呢？使用pyenv。

## 2.1 安装pyenv
```shell
brew install pyenv
```
### 2.1.1 查看已安装的版本
```shell
pyenv versions
```
我的电脑返回以下版本信息
```text
  system
  3.8.16
* 3.12.9 (set by /Users/${USERNAME}/.pyenv/version)
```
### 2.1.2 查看可安装的版本
```shell
pyenv install -l
```
### 2.1.3 安装指定版本的python, 本文指定3.12.9或3.12.x更新版本
```shell
pyenv install 3.12.9
```

### 2.1.4 pip类库管理工具
安装python完成之后, python类库管理工具pip已经默认安装完成了

### 2.1.5 python基础工具

| 工具  | 功能                           |
|:----|:-----------------------------|
| pip | 类似maven、gradle、go mod的类库管理工具 |
| pip-autoremove| 自动删除类库所有依赖库                  |
|pipreqs| 项目/类库交叉依赖检测                  |

### 2.1.6 pip 源配置
windows
```shell
cd ~\AppData\Roaming\pip
notepad.exe pip.ini
```
*nix
```shell
cd ~/.pip
vim pip.conf
```
输入以下内容
```text
[global]
index-url = https://pypi.tuna.tsinghua.edu.cn/simple
[install]
trusted-host = https://pypi.tuna.tsinghua.edu.cn
```

## 2.2 依赖库
python环境中依赖管理的配置文件为requirements.txt，类似java的maven pom.xml、golang的go.mod。

### 2.2.1 安装项目依赖的库
```shell
pip install -r requirements.txt
```

### 2.2.2 检测项目依赖输出到requirements.txt
```shell
pip freeze > requirements.txt
```

### 2.2.3 交叉依赖

```shell
pip install pipreqs
pipreqs ./ --encoding utf8
```

## 2.3 上传package到PyPi

### 2.3.1 安装或更新setuptools、wheel、twine
```shell
pip install --upgrade setuptools wheel twine
```

### 2.3.2 打包并生成tar.gz和whl文件
```shell
python setup.py sdist bdist_wheel
```

### 2.3.3 上传package到PyPi的测试环境
```shell
twine upload --repository testpypi dist/*
```

### 2.3.4 上传package到PyPi的正式环境
```shell
twine upload dist/*
```

## 2.4 Matplotlib中文乱码问题解决方案

### 2.4.1 编写如下代码，获取matplotlib包所在的配置文件的路径
```python
import matplotlib
matplotlib.matplotlib_fname() #输出matplotlib包所在的配置文件的路径
```

### 2.4.2 根据上面的路径打开文件夹（根据自己实际的输出路径去操作）
我选择了SimHei中文字体, 复制到fonts/ttf/目录下
```shell
cp -r /Users/${USERNAME}/Library/Fonts/SimHei.ttf fonts/ttf/
```

### 2.4.3 编辑2.4.1获得路径matplotlibrc文件
#### 2.4.3.1 找到 #font.sans-serif，去掉前面的#，并在：后面写上在准备工作加入的中文字体的名称SimHei
#### 2.4.3.2 找到#axes.unicode_minus，去掉前面的#，并在：改为False
### 2.4.4 控制台切换到~/.matplotlib目录, 删除tex.cache文件和fontList.json文件
```shell
cd ~/.matplotlib
rm -rf *
```


# 3. golang 开发环境

环境设定
```shell
go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct
go env -w GOPRIVATE=gitee.com
```

# 4. Rust 开发环境

```bash
# 安装 Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# 更新到最新版本
rustup update
```

## Rust: 构建与运行 `q1x` 二进制

本仓库在 `Cargo.toml` 中声明了一个名为 `q1x` 的可执行二进制（路径为 `main.rs`）。下面是构建与常见运行示例：

- 在开发模式下构建：

```sh
cargo build --bin q1x
```

- 在发布/生产模式下构建（优化）：

```sh
cargo build --release --bin q1x
```

- 运行并查看帮助信息：

```sh
# 打印程序帮助（包含子命令和选项）
cargo run --bin q1x -- --help
```

- 常见子命令示例（基于项目根 `main.rs` 中使用的 clap 定义）：

```sh
# 管理服务（install/uninstall/start/stop/status/run）
cargo run --bin q1x -- service install
cargo run --bin q1x -- service start

# 更新缓存数据（calendar / servers / all / base / features）
cargo run --bin q1x -- update --all
cargo run --bin q1x -- update --calendar
```

### 直接示例：从已构建二进制查看帮助

你也可以直接运行已构建的二进制查看实际帮助文本，例如：

```powershell
# Debug 二进制
.\target\debug\q1x.exe --help

# Release 二进制
.\target\release\q1x.exe --help
```

下面是我在本地运行 `target/release/q1x.exe --help` 捕获到的输出（供参考）：

```text
quant1x - Rust edition
--------------------------------------------------------------------------------
         Version : 0.6.10
        Author : Quant1X Team
--------------------------------------------------------------------------------


Usage: q1x.exe [OPTIONS] [COMMAND]

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

说明与注意事项：

- `stock` 二进制是一个可与库（crate）协同工作的前端：它会尝试调用库中暴露的初始化与子命令钩子（如 `quant1x::global_init`, `quant1x::datasets_init`, `quant1x::engine_daemon` 等）。如果你直接从源码运行，确保启用了需要的 feature 或在编译时链接到库中实现这些函数的 crate。
- 在 Windows 上，可以通过 `service` 子命令与提升参数（`--pipe`, `--elevated-out`, `--elevated-pipe`）交互以支持以服务/守护进程模式运行。具体行为由 `engine::daemon` 的实现决定。
- 若打算在 CI 或部署中使用，请优先使用 `--release` 构建并根据目标平台交叉编译或在对应平台上构建以避免 -march/native 引入不可移植的指令集。

如果需要，我可以：

- 为 `stock` 添加示例配置和 systemd/Windows service 安装脚本。
- 在 README 中加入更详细的运行参数说明（基于库中 `engine::daemon` 的实现）。

# 5. c/c++ 开发环境

本项目的 C/C++ 代码以 C++20 为目标，强烈建议在开发/构建阶段使用较新的编译器和现代构建工具以获得最佳性能与可维护性。

推荐工具链与版本

- 编译器：GCC 14.3+ / Clang 18+ / MSVC (Visual Studio 2022)（MSVC 工具集 14.3+）
- CMake：3.30+（建议最新版）
- 构建器：Ninja（推荐）或 Make/MSBuild
- 包管理：vcpkg 或 Conan（可选，但用于管理第三方依赖非常有用）

常用依赖（示例）

- OpenSSL（网络/加密）
- protobuf（序列化）
- fmt / spdlog（格式化与日志）

Linux/macOS 快速安装（示例）

- Ubuntu / Debian:

  - sudo apt update && sudo apt install -y build-essential cmake ninja-build clang pkg-config
  - 若使用 vcpkg，请参考 vcpkg 文档进行安装。

- macOS (Homebrew):

  - brew install cmake ninja llvm vcpkg

Windows（Visual Studio）

- 推荐安装 Visual Studio 2022 + Desktop development with C++，并从 “x64 本机工具命令提示符” 或者 VS 开发者 PowerShell 构建。
- 使用 vcpkg 管理依赖（示例：vcpkg integrate install），并在 CMake 调用中传递 `-DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake`。

构建示例（以仓库根目录为例）

- 使用 Ninja + Clang/GCC（跨平台推荐）：

  - mkdir -p build && cd build
  - cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20 ..
  - cmake --build . --config Release

- 使用 Visual Studio（Windows）：

  - mkdir build && cd build
  - cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release ..
  - cmake --build . --config Release

建议的编译选项（可在 CMakeLists 或构建命令中添加）

- Release 编译：-O3 -DNDEBUG
- 可选：启用 LTO（Link Time Optimization）与目标特定指令集：-march=native（仅在目标机测试时启用）
- 针对跨平台性能调优：保持内存对齐、禁用不必要的异常展开（视模块而定）、使用合适的预取/缓存策略

关于性能与编译器选择（说明）

- 在本项目的微基准中，LLVM/Clang 与 MSVC 在某些并发热点代码上生成的汇编与性能通常优于某些 GCC 版本；若追求最高性能，建议在 CI 中使用 clang 或 MSVC 做对比测试。
- 但为保证广泛兼容性，请在主 CI 流水线中测试所有目标编译器（GCC / Clang / MSVC）。

调试与分析工具

- 使用 sanitizers（AddressSanitizer, ThreadSanitizer）在调试构建中快速捕获内存/线程错误：在 CMake 中开启 -DSANITIZE_ADDRESS=ON（项目支持时）。
- 使用 perf / VTune / Windows Performance Analyzer 进行性能剖析。

依赖管理（vcpkg 简短示例）

- 克隆并引导 vcpkg：

  ```sh
  git clone https://github.com/microsoft/vcpkg.git
  ./vcpkg/bootstrap-vcpkg.sh  # Linux/macOS
  .\vcpkg\bootstrap-vcpkg.bat  # Windows (PowerShell/CMD)
  ```

- 在 CMake 调用中添加 toolchain 文件：

  ```sh
  cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
  ```

平台特定注意事项

- Windows：注意选择 x64 构建，使用 vcpkg 的 triplet（例如 `x64-windows`）来安装二进制依赖。
- Linux：若在容器/CI 中构建，尽量固定基础镜像以保证可重复构建（例如 `ubuntu:22.04`）。
- macOS：使用 Homebrew 管理依赖，并注意 Apple Clang 与 LLVM Clang 之间的小差别。

如果你希望我为项目添加一个方便的 CMake 构建示例（例如 top-level `build` 脚本和 CI job 示例），我可以继续创建并把它加入仓库。

附加：C++ 具体构建与测试示例

依赖安装（protobuf/其他）示例：

- protobuf 推荐使用 3.21.11（高版本可能依赖 abseil，引入额外复杂性）：

  ```sh
  # 下载并解压源码
  wget https://gh-proxy.com/github.com/protocolbuffers/protobuf/releases/download/v21.11/protobuf-cpp-3.21.11.zip
  unzip protobuf-cpp-3.21.11.zip && cd protobuf-3.21.11
  mkdir build && cd build
  cmake -DCMAKE_INSTALL_PREFIX=$HOME/runtime -Dprotobuf_BUILD_TESTS=OFF -G "Unix Makefiles" ../
  make -j$(nproc) && make install
  ```

- Windows (MSVC) 编译示例：

  ```ps1
  mkdir build; cd build
  cmake -DCMAKE_INSTALL_PREFIX=d:/runtime -G "Visual Studio 17 2022" -A x64 ..
  cmake --build . --config Release
  ```

其它依赖安装（vcpkg 举例）:

```sh
vcpkg install yaml-cpp zlib asio xtensor mimalloc spdlog fmt duktape benchmark catch2 flatbuffers capnproto
```

项目快速编译（Debug 示例）：

```sh
cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -S . -B cmake-build-debug
cmake --build cmake-build-debug --target q1x -j 18
```

安装主程序示例：

```sh
ninja -C cmake-build-debug install
```

运行测试（ctest）：

```sh
ctest --test-dir cmake-build-debug --output-on-failure
```

运行示例程序：

```sh
./cmake-build-debug/bin/q1x --help
```

## 🤝 贡献

欢迎贡献代码！请遵循以下步骤：

1. Fork 本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

### 开发规范

- 遵循各语言的编码规范
- 添加单元测试
- 更新文档
- 确保所有测试通过

## 📄 许可证

本项目采用 Apache 2.0 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 📚 相关链接

- [文档](docs/)
- [示例](examples/)
- [问题跟踪](https://gitee.com/quant1x/quant1x/issues)
- [讨论区](https://gitee.com/quant1x/quant1x/discussions)

---

**注意**: 本项目仅用于学习和研究目的，不构成投资建议。在实际交易前，请充分了解风险并咨询专业人士。
