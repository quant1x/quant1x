# ringbuffer

本目录只保留 Vyukov 有界 MPMC 队列的 Rust 实现（`vyukov.rs`）。

简要说明：

- 之前仓库中存在一个旧的 `RingBuffer` 实现（已移除/覆盖），现在模块入口 `mod.rs` 仅导出 `vyukov`：

  ```rust
  pub mod vyukov;
  pub use vyukov::*;
  ```

- 具体实现见 `vyukov.rs`，它实现了基于序号（per-slot sequence）和原子索引的无锁有界 MPMC 队列。

使用与测试

- 在 release 模式下运行库内的被忽略性能测试（会打印吞吐）：

```powershell
cargo test --manifest-path d:/projects/quant1x/quant1x/Cargo.toml --features vyukov --release --lib ringbuffer::vyukov::tests::mpmc_performance_heavy -- --ignored --nocapture
```

- 运行示例（如果存在示例 `mpmc_demo`）：

```powershell
cargo run --manifest-path d:/projects/quant1x/quant1x/Cargo.toml --example mpmc_demo --features vyukov --release
```

性能测量产物（已生成）

在本次工作中生成了以下性能测量文件，位于本目录：

- `perf_samples_200.csv` — 200 条 ops/sec 原始样本（CSV，一列）。
- `perf_hist.png` — 基于样本的直方图（PNG）。
- `perf_box.png` — 基于样本的箱形图（PNG）。
- `perf_report.md` — 自动生成的 Markdown 报告，包含统计摘要与嵌入图片。

复现实验（快速步骤）

1. 在 release 模式用现有 runner 采样（runner 会做 10 次采样）：

```powershell
# 直接运行已编译的二进制（若已编译）
.\target\release\vyukov_runner.exe

# 或使用 cargo run（会在未编译时构建）
cargo run --manifest-path d:/projects/quant1x/quant1x/Cargo.toml --bin vyukov_runner --features vyukov --release
```

2. 生成并查看图表与报告：

```powershell
# 需要 Python 和 matplotlib
python quant1x/ringbuffer/plot_perf.py
# 打开 perf_report.md 或 perf_hist.png / perf_box.png
```

注意与建议

- 我已按照当前指示避免任何与 git 有关的操作（未做提交或创建分支）。
- 性能波动可能由系统调度、CPU 频率、电源策略、编译器标志（`RUSTFLAGS`）、链接器与运行时差异引起；建议在受控环境（空闲系统、固定 CPU 亲和性、电源性能模式）下重复测量以获得更稳健结果。

下一步（可选）

- 我可以把数据文件 (`perf_samples_200.csv`) 和生成的图片合并到一个更完整的报告中，或在同一图上对比不同编译选项的结果；如果需要，请告诉我要对比的配置（例如：`target-cpu=native`、`codegen-units=1`、启/不启 LTO）。

如需我继续自动化这些步骤，请直接告诉我你想要的对比或输出格式。
