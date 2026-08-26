# Quant1X ID 64 位分布式 ID（`id64`）

> 与 [`../id128/README.md`](../id128/README.md) 同构的 64 位版本。
> Python 是 Spec，C++ 是 Truth（本项目无 C++ 版，Go 为生产参考），Rust / Go 是对齐实现。
> 本模块提供 Go / Python / Java 三语言 1:1 实现，I/O 边界字段全小写 `snake_case`。

## 1. 概述

`id64` 是基于 **HLC（Hybrid Logical Clock）** 的 64 位可排序分布式 ID：

- **64 位紧凑布局**：`1bit 符号 + 41bit 时间戳 + 动态位宽（节点 + 序列）`
- **单调不重复**：物理时钟回拨时由序列号进位保证单调，无需等待墙钟追平
- **跨进程 / 跨重启强唯一**：可选状态文件持久化（与 id128 相同的 18B 记录格式）；默认快速路径（构造时恢复一次水位、运行期纯内存推进、批量缓冲攒批落盘），多写者活跃共享可开启严格模式
- **动态位宽**：节点位宽由预期节点总数推导，天然支持大规模集群（K8s 数千节点）

## 2. 位布局（动态位宽）

```
| 1bit 符号(恒 0) | Physical(41bit) | NodeID(workerBits) | Seq(seqBits) |
|-----------------|-----------------|--------------------|--------------|
      0              毫秒(epoch 起)      节点标识               序列号
```

- **时间戳**：41 bit，起点 `EPOCH = 2026-01-01T00:00:00Z`，毫秒精度，量程约 **69.7 年**（至 2095 年）
- **符号位**：恒 0，`Java long` / `Go uint64` 均无符号负担
- **动态位宽**（与节点总数挂钩）：

```
workerBits = bits.Len(nodeCount)      // ceil(log2(nodeCount))
seqBits    = 64 - 1 - 41 - workerBits // 剩余全部给序列号
```

| nodeCount | workerBits | seqBits | 单节点吞吐/毫秒 | 适用场景 |
|-----------|-----------|---------|----------------|---------|
| 1024（默认） | 11 | 11 | 2048 | 通用 |
| 5000 | 13 | 9 | 512 | K8s 大规模集群 |
| 131072 | 18 | 4 | 16 | 极限节点数 |

- 当 `seqBits < 4`（即节点数 > 2^18 = 262144）时拒绝构造（`panic` / `ValueError` / `IllegalArgumentException`）
- 所有 ID 恒为 64 位，存储与传输零变化，不影响既有表结构

### 与 id128 的差异

| 维度 | id128 | id64 |
|------|-------|------|
| 位宽 | 128（HLC 64 + NodeID 32 + Seq 32） | 64（41 + workerBits + seqBits） |
| logical 位 | 16 bit，参与 HLC 进位 | 无（空间限制） |
| 回拨单调 | logical 递增 | seq 递增，seq 达容量时 physical+1 |
| 启动种子 | logical = seed | seq 初始 = seed（随机化重启碰撞） |
| ID 字符串 | base64url 无填充（22 字符） | base64url 无填充（11 字符） |
| 状态文件 | 18B（physical+logical+seq+CRC32） | 18B 同格式（logical 恒 0，文件兼容） |

## 3. 特性

- **单调性**：`(physical, seq)` 字典序严格递增；时钟回拨时 seq 递增，seq 达 `2^seqBits` 容量则 `physical+1`，**不等待墙钟追平**
- **跨重启强唯一**：`WithStateFile` 持久化高水位，重启后从文件恢复
- **快速路径（默认）**：构造时从状态文件恢复一次高水位，运行期纯内存推进，状态记录在批量缓冲中累积、每攒满 `syncEvery` 条才落盘一次——适合单写者与多进程顺序接管（failover），热路径零系统调用；优雅退出前调用 `Close()` / `close()` 刷盘剩余缓冲可零丢失
- **严格模式（可选）**：`WithStateStrict` 开启后每次发号读盘取 max，保证多个写者**同时活跃**共享同一状态文件时的唯一性（以每次发号一次磁盘读 + 一次落盘写为代价）
- **并发安全**：HLC 内部互斥锁，单实例多协程/线程安全

## 4. 各语言 API

### Go（`quant1x/id/id64`，`package id64`）

```go
import "github.com/quant1x/quant1x/quant1x/id/id64"

// 预期节点总数（K8s 场景从环境变量读取，见下方示例）
hlc := id64.NewHLC(id64.WithNodeCount(5000), id64.WithStateFile("./data/id64.state"))
gen := id64.NewGenerator(nodeID, hlc) // nodeID < 2^workerBits

next := gen.Next()      // id64.ID (uint64 位模式)
next.String()           // base64url 无填充
next.Bytes()            // [8]byte BigEndian
next.Physical()         // epoch 相对毫秒
next.NodeID(gen.WorkerBits())
next.Seq(gen.WorkerBits())
```

节点数从环境变量读取的推荐写法：

```go
nodeCount := uint32(1024)
if v := os.Getenv("EXPECTED_NODE_COUNT"); v != "" {
    if n, err := strconv.ParseUint(v, 10, 32); err == nil && n > 0 {
        nodeCount = uint32(n)
    }
}
hlc := id64.NewHLC(id64.WithNodeCount(nodeCount))
```

### Python（`quant1x.id.id64`）

```python
from quant1x.id.id64 import Generator, HLC, ID, with_node_count, with_state_file

hlc = HLC(with_node_count(5000), with_state_file("./data/id64.state"))
gen = Generator(node_id, hlc)

value = gen.next()       # int 位模式
identifier = ID.from_int(value)
str(identifier)          # base64url 无填充
identifier.physical()    # epoch 相对毫秒
identifier.node_id(gen.worker_bits)
identifier.seq(gen.worker_bits)
```

### Java（`quant1x.id.id64`）

```java
import quant1x.id.id64.Generator;
import quant1x.id.id64.HLC;
import quant1x.id.id64.Id;
import quant1x.id.id64.Option;

HLC hlc = new HLC(
        Option.withNodeCount(5000),
        Option.withStateFile("./data/id64.state"));
Generator gen = new Generator(nodeID, hlc);

long value = gen.next();   // long 位模式（符号位恒 0，恒为正）
Id id = Id.fromLong(value);
id.toString();             // base64url 无填充
id.physical();             // epoch 相对毫秒
id.nodeId(gen.workerBits());
id.seq(gen.workerBits());
```

## 5. 配置项（Options）

| 选项 | Go | Python | Java | 说明 |
|------|----|--------|------|------|
| 覆盖时钟 | `WithClock` | `with_clock` | `withClock` | 测试注入毫秒时钟 |
| 启动种子 | `WithSeqSeed` | `with_seq_seed` | `withSeqSeed` | 随机化初始 seq，默认随机 |
| 状态文件 | `WithStateFile` | `with_state_file` | `withStateFile` | 启用跨进程/重启持久化 |
| 落盘间隔 | `WithStateSyncEvery` | `with_state_sync_every` | `withStateSyncEvery` | 每 N 条状态记录批量落盘一次（默认 1000） |
| 严格模式 | `WithStateStrict` | `with_state_strict` | `withStateStrict` | 每次发号读盘取 max（默认关闭） |
| 节点总数 | `WithNodeCount` | `with_node_count` | `withNodeCount` | 推导动态位宽（核心配置） |
| 序列位宽 | `WithSeqBits` | `with_seq_bits` | `withSeqBits` | 底层选项，一般用节点总数代替 |

### 落盘间隔的默认值与环境变量

- 默认 `syncEvery = 1000`：快速路径下状态记录先在内存批量缓冲中累积，每攒满 1000 条才在进程锁保护下一次性追加到文件末尾并 `fsync`。大多数请求不触碰磁盘，热路径纯内存；进程异常退出最多丢失最近 1000 条**持久化进度**（这些 ID 重启后可能重复），调用 `Close()` / `close()` 可零丢失。
- 可通过环境变量 `QUANT1X_ID64_SYNC_EVERY` 覆盖默认值（三语言一致），显式 Option 优先级最高：

```bash
# 每 100 条记录刷盘一次
QUANT1X_ID64_SYNC_EVERY=100 ./your-binary
```

- 极端低延迟场景可调大（如 10000），强调持久化可调小（如 1，等价于每条落盘）。

## 6. 状态文件格式

18 字节/条，BigEndian，与 id128 完全兼容（logical 字段恒 0）：

```
|------------|---------|---------|-----------|
| Physical   | Logical | Seq     | CRC32     |
| 8B         | 2B(恒0) | 4B      | 4B        |
|------------|---------|---------|-----------|
```

- CRC32 为 IEEE 802.3（Go `crc32.ChecksumIEEE` / Python `zlib.crc32` / Java `java.util.zip.CRC32`），覆盖前 14 字节
- 坏损 / 截断的尾部记录会被忽略（Go 截断文件，Python / Java 向前扫描）

## 7. 文件清单

```
quant1x/id/id64/
├── id64.go             # ID 类型（Go）
├── hlc.go              # HLC（Go）
├── generator.go        # Generator（Go）
├── option.go           # 配置项（Go）
├── state_store.go      # 状态存储（Go）
├── state_lock_windows.go / state_lock_unix.go / state_lock_other.go
├── id_test.go          # Go 测试（含基准）
├── __init__.py         # Python 包入口
├── id.py               # ID 类型（Python）
├── hlc.py              # HLC（Python）
├── generator.py        # Generator（Python）
├── option.py           # 配置项（Python）
├── state_store.py      # 状态存储（Python）
├── id_test.py          # Python 测试
├── Id.java             # ID 类型（Java）
├── HLC.java            # HLC（Java）
├── Generator.java      # Generator（Java）
├── Option.java         # 配置项（Java）
├── StateStore.java     # 状态存储接口（Java）
├── PersistentState.java# 持久化状态（Java）
├── FileStateStore.java # 文件状态存储（Java）
├── ByteIO.java         # BigEndian 编解码（Java）
└── README.md           # 本文档
```

各语言测试统一位于仓库 `tests/` 目录（Go / Python 例外，留在本目录）。Java 测试在 `tests/id/id64/IdTest.java`（`package quant1x.id.id64`），物理路径与模块目录保持一致。

## 8. 测试

```bash
# Go
go test ./quant1x/id/...

# Python（在仓库根目录）
python -m unittest quant1x.id.id64.id_test

# Java（在仓库根目录）
mvn test
```

## 9. 版本与发布

版本号由 Git Tag 唯一决定，`autochangelog` 是唯一合法发布入口。**禁止手动修改任何版本号字符串。**
