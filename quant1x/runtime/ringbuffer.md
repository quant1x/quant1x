# Vyukov ringbuffer — 推荐使用 LLVM/Clang（已验证优先级）

本目录包含 Vyukov 有界 MPMC 队列的 C++ 头文件实现（`vyukov.hpp`）和一个微基准 runner（`vyukov_runner.cpp`）。

## 为什么推荐 LLVM/Clang

- 在本仓库的本地测量中，使用 LLVM 后端（clang++ / clang-cl / rustc 的 LLVM）能够显著提高吞吐：clang 与 MSVC 编译出的二进制在同一台机器上常见为 ~25–35M ops/sec，而 g++ 在相同实现下通常落在 ~5–8M（受代码细节影响会有波动）。
- 因此，对于对吞吐敏感的场景，建议在开发/CI 中默认使用 clang（或保持 Rust 的实现）。

## 快速构建命令（示例）

- 在 Linux / 支持 clang++ 的环境下：

```bash
clang++ -std=c++20 -O3 -march=native quant1x/ringbuffer/vyukov_runner.cpp -o quant1x/ringbuffer/vyukov_runner_clang -pthread
./quant1x/ringbuffer/vyukov_runner_clang
```

- 在 Windows（推荐使用 clang-cl，从 Developer Command Prompt/PowerShell 调用 vcvarsall）：

```cmd
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
clang-cl /std:c++20 /O2 /MD /Fe:quant1x\ringbuffer\vyukov_runner_clangcl.exe quant1x\ringbuffer\vyukov_runner.cpp
quant1x\ringbuffer\vyukov_runner_clangcl.exe
```

### 为什么这些 flags

- `-O3 -march=native` 启用更激进的优化并利用目标 CPU 指令集。若需要更可预测的测量，请移除 `-march=native` 并指定固定架构。

## 关于 g++（兼容性说明）

- 我们保留了对 g++ 的兼容实现，但要注意：在 Windows + MinGW（或某些 g++ 版本）组合下，g++ 的 codegen 对该热点代码表现较保守，吞吐会显著低于 clang/MSVC。
- 如果必须使用 g++：
  - 可在代码中引入条件编译分支以提供更 "GCC-friendly" 的实现（通常会在正确性与性能间做权衡）；
  - 或把 g++ 用作功能/回归测试，而在 CI/perf 测试中优先使用 clang。

## 持续集成（建议）

- 仓库中包含一个示例 workflow：`.github/workflows/clang-windows.yml`，用于在 Windows / Ubuntu 上用 clang 构建并运行基准。建议把该 workflow 作为 perf-sensitive 配置的一部分。

## 汇报与再现

- 基准样本会写入：`quant1x/ringbuffer/cpp_perf_samples.csv`（每次运行覆盖）。
- 如果你愿意，我可以把当前的 clang/msvc/gcc 基准结果整理成对比报告并提交为 PR 注释。

## 复现实验（快速步骤）

1. 在 release 模式用现有 runner 采样（runner 会做 10 次采样）：

```powershell
# 直接运行已编译的二进制（若已编译）
.\target\release\vyukov_runner.exe

# 或使用 cargo run（会在未编译时构建）
cargo run --manifest-path d:/projects/quant1x/quant1x/Cargo.toml --bin vyukov_runner --features vyukov --release
```

1. 生成并查看图表与报告（需要 Python + matplotlib）：

```powershell
python quant1x/ringbuffer/plot_perf.py
# 打开 perf_report.md 或 perf_hist.png / perf_box.png
```

## 注意与建议

- 性能波动可能由系统调度、CPU 频率、电源策略、编译器标志（`RUSTFLAGS`）、链接器与运行时差异引起；建议在受控环境（空闲系统、固定 CPU 亲和性、电源性能模式）下重复测量以获得更稳健结果。

## 统计摘要

- 样本数: 200
- 平均 (mean): 43631329.56 ops/sec
- 中位数 (median): 43546578.56 ops/sec
- 标准差 (stddev): 3429885.57 ops/sec
- 最小值: 34444256.28 ops/sec
- 最大值: 54202867.78 ops/sec
- 25th percentile: 41269013.63 ops/sec
- 75th percentile: 45642871.44 ops/sec
