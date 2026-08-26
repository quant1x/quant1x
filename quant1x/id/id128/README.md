# id — HLC + Uint128 ID 生成器

基于本地 HLC 状态生成可排序的 128-bit ID。提供 **Go**、**Python** 和 **Java** 三套 1:1 对齐的实现，核心语义与位布局完全一致。

当前实现优先保证单实例内严格单调；当配置状态文件后，可以在同一节点跨重启延续高水位，避免重复发号。默认采用**快速路径**（构造时从状态文件恢复一次水位，运行期纯内存推进，状态记录批量缓冲、攒批落盘），适合单写者与多进程顺序接管（failover）；多个写者**同时活跃**共享同一状态文件时，必须显式开启**严格模式**（每次发号读盘取 max）。它不是带远端合并能力的完整分布式 HLC。

## 设计边界

| 维度 | Go | Python | Java |
| ---- |:--:|:------:|:----:|
| 数学表示 | 真 128-bit 整数 `Uint128`，不是字符串拼接 | ← 相同 | `Uint128`（hi/lo 位模式） |
| 单实例时序 | 同一 `HLC` 实例内 `(hlc, seq)` 严格单调 | ← 相同 | ← 相同 |
| 时钟回拨 | 本地时钟回拨不会让新 ID 倒退 | ← 相同 | ← 相同 |
| 重启碰撞 | 启动种子 + 可选状态文件持久化 | ← 相同 | ← 相同 |
| 进程安全 | 内存 `sync.Mutex` | `threading.Lock` | `synchronized` |
| 跨进程安全 | `flock` / `LockFileEx` / `O_EXCL` | `msvcrt` / `fcntl.flock` / `O_EXCL` | `FileChannel.lock()` |

## 文件结构

Go、Python 和 Java 同目录共存，靠文件后缀区分：

```
id/id128/
├── uint128.go       # Uint128（Go）
├── uint128.py       # Uint128（Python）
├── Uint128.java     # Uint128（Java）
├── hlc.go           # HLC 核心（Go）
├── hlc.py           # HLC 核心（Python）
├── HLC.java         # HLC 核心（Java）
├── generator.go     # ID 生成器（Go）
├── generator.py     # ID 生成器（Python）
├── Generator.java   # ID 生成器（Java）
├── id.go            # ID 编解码（Go）
├── id.py            # ID 编解码（Python）
├── Id.java          # ID 编解码（Java）
├── option.go        # 配置项（Go）
├── option.py        # 配置项（Python）
├── Option.java      # 配置项（Java）
├── state_store.go        # 状态持久化（Go）
├── state_store.py        # 状态持久化（Python）
├── StateStore.java       # 状态存储接口（Java）
├── FileStateStore.java   # 状态持久化（Java）
├── PersistentState.java  # 状态值对象（Java）
├── state_lock_unix.go    # 文件锁 Unix（Go）
├── state_lock_windows.go # 文件锁 Windows（Go）
├── state_lock_other.go   # 文件锁 兼容模式（Go）
├── id_test.go       # 测试（Go）
├── id_test.py       # 测试（Python）
└── __init__.py      # Python 包入口（父包 id/__init__.py 仅转发到本子包）
```

各语言测试统一位于仓库 `tests/` 目录（Go / Python 例外，留在本目录）。Java 测试亦在 `tests/` 下，按 package 组织（如 `tests/id/id128/IdTest.java`），物理路径与模块目录保持一致。

## ID 位布局

```text
┌──────────────────────────────────────────────────────────────────────┐
│                           128-bit ID                                 │
├──────────────────────┬──────────────────┬────────────────────────────┤
│ HLC (64 bit)         │ Node ID (32 bit) │ Sequence (32 bit)          │
└──────────────────────┴──────────────────┴────────────────────────────┘
```

等价表达：

```rust
let id = (hlc as u128) << 64
       | (node as u128) << 32
       | seq as u128;
```

## 快速开始

### Go

```go
package main

import (
    "fmt"

    "github.com/quant1x/quant1x/quant1x/id/id128"
)

func main() {
    hlc := id128.NewHLC()
    gen := id128.NewGenerator(1, hlc)
    next := gen.Next()

    fmt.Printf("hi=%d lo=%d\n", next.Hi, next.Lo)
    fmt.Printf("bytes=%x\n", next.Bytes())
}
```

### Python

```python
from quant1x.id.id128 import Generator, HLC, with_state_file

hlc = HLC(with_state_file("./data/id.state"))
gen = Generator(1, hlc)
id = gen.next()

print(id.hi, id.lo)
print(id.to_bytes().hex())
```

### Java

```java
import quant1x.id.id128.HLC;
import quant1x.id.id128.Generator;
import quant1x.id.id128.Uint128;
import quant1x.id.id128.Option;

HLC hlc = new HLC(
        Option.withStateFile("./data/id.state"));
Generator gen = new Generator(1, hlc);
Uint128 id = gen.next();

System.out.println("hi=" + id.high64() + " lo=" + id.low64());
```

## API

| 模块 | Go | Python | Java |
| ---- | ---- | ---- | ---- |
| Uint128 | `uint128.go` — `struct{Hi, Lo uint64}` + 比较/算术/位运算 | `uint128.py` — `@dataclass(frozen=True)` `Uint128` + 全比较 + `from_int/to_int` | `Uint128.java` — hi/lo `long` 位模式 + 无符号比较/算术/位运算 |
| HLC | `NewHLC(opts ...Option) *HLC` / `Now() (uint64, uint32)` | `HLC(*options)` / `now() -> (int, int)` | `new HLC(Option...)` / `now() -> Now` |
| Generator | `NewGenerator(nodeID uint32, hlc *HLC) *Generator` / `Next() Uint128` | `Generator(node_id, hlc)` / `next() -> Uint128` | `new Generator(nodeID, hlc)` / `next() -> Uint128` |
| Option | `WithClock` / `WithLogicalSeed` / `WithStateFile` / `WithStateSyncEvery` | `with_clock` / `with_logical_seed` / `with_state_file` / `with_state_sync_every` | `withClock` / `withLogicalSeed` / `withStateFile` / `withStateSyncEvery` |

### Go

| 方法 | 签名 |
| ---- | ---- |
| `NewHLC(opts ...Option)` | 创建 HLC 实例 |
| `(h *HLC) Now()` | 返回 `(hlc uint64, seq uint32)` |
| `(h *HLC) Timestamp()` | 返回当前物理毫秒值 `int64` |
| `(h *HLC) Close() error` | 刷盘批量缓冲（优雅退出前调用，可零丢失；未启用状态文件时空操作） |
| `NewGenerator(nodeID uint32, hlc *HLC)` | 创建生成器，`hlc` 不能为空 |
| `(g *Generator) Next()` | 返回 `Uint128` |
| `WithClock(now func() int64)` | 注入测试时钟 |
| `WithLogicalSeed(seed uint16)` | 启动逻辑种子 |
| `WithStateFile(path string)` | 持久化状态文件路径 |
| `WithStateSyncEvery(every uint32)` | 同步频率（默认 1000，环境变量 `QUANT1X_ID128_SYNC_EVERY`） |
| `WithStateStrict()` | 严格模式：每次发号读盘取 max（默认关闭） |

### Python

| 方法 | 签名 |
| ---- | ---- |
| `HLC(*options)` | 创建 HLC 实例 |
| `hlc.now()` | 返回 `(hlc: int, seq: int)` |
| `hlc.close()` | 刷盘批量缓冲（优雅退出前调用，可零丢失；未启用状态文件时空操作） |
| `Generator(node_id: int, hlc: HLC)` | 创建生成器 |
| `gen.next()` | 返回 `Uint128` |
| `with_clock(now: Callable)` | 注入测试时钟 |
| `with_logical_seed(seed: int)` | 启动逻辑种子 |
| `with_state_file(path: str)` | 持久化状态文件路径 |
| `with_state_sync_every(every: int)` | 同步频率（默认 1000，环境变量 `QUANT1X_ID128_SYNC_EVERY`） |
| `with_state_strict()` | 严格模式：每次发号读盘取 max（默认关闭） |

### Java

| 方法 | 签名 |
| ---- | ---- |
| `new HLC(Option...)` | 创建 HLC 实例 |
| `hlc.now()` | 返回 `Now`，含 `hlc()` 与 `seq()` |
| `hlc.close()` | 刷盘批量缓冲（优雅退出前调用，可零丢失；未启用状态文件时空操作） |
| `new Generator(nodeID: long, hlc: HLC)` | 创建生成器，`hlc` 不能为 null |
| `gen.next()` | 返回 `Uint128` |
| `Option.withClock(LongSupplier)` | 注入测试时钟 |
| `Option.withLogicalSeed(int)` | 启动逻辑种子（16 位） |
| `Option.withStateFile(String)` | 持久化状态文件路径 |
| `Option.withStateSyncEvery(long)` | 同步频率（默认 1000，环境变量 `QUANT1X_ID128_SYNC_EVERY`） |
| `Option.withStateStrict()` | 严格模式：每次发号读盘取 max（默认关闭） |

## 使用建议

1. `nodeID` 必须由外部系统唯一分配，否则跨节点唯一性无法成立。
2. 如果你需要跨重启的强唯一保证，应启用 `WithStateFile(...)` / `with_state_file(...)` / `withStateFile(...)`，把最后发号状态持久化到稳定存储。默认快速路径即可满足（构造时恢复 + 批量缓冲落盘）；优雅退出前调用 `Close()` / `close()` 把剩余缓冲刷盘，可保证重启零重复。
3. 如果你需要**多个写者同时活跃**共享同一状态文件（如同 JVM/同进程多个 HLC 实例、多进程双活），必须额外开启 `WithStateStrict(...)` / `with_state_strict(...)` / `withStateStrict(...)`——严格模式让每次发号以磁盘最新状态为基准。
4. 如果你需要在吞吐和崩溃恢复之间折中，可以配合 `WithStateSyncEvery(...)` / `with_state_sync_every(...)` / `withStateSyncEvery(...)` 调整批量落盘频率；这会影响进程异常退出时的丢失窗口（最近最多 N 条进度可能尚未落盘）。优雅退出前调用 `Close()` / `close()` 可把剩余缓冲完整刷盘、零丢失。
5. 如果你需要真正的分布式 HLC，请补充远端时间戳合并接口，而不是只依赖本地 `Now()`。

## DB 兼容性

`Uint128.Bytes()` / `Uint128.to_bytes()` 使用 BigEndian，适合直接存储为 16 字节主键：

- MySQL / MariaDB: `BINARY(16)`
- PostgreSQL: `UUID` 或 `BYTEA`
- SQLite: `BLOB`

```sql
CREATE TABLE events (
    id BINARY(16) PRIMARY KEY,
    data TEXT
);
```

**Go**

```go
db.Exec("INSERT INTO events (id, data) VALUES (?, ?)", id.Bytes(), data)

var raw [16]byte
row.Scan(&raw[:])
recovered := id.FromBytes(raw)
```

**Python**

```python
cursor.execute("INSERT INTO events (id, data) VALUES (%s, %s)", (id.to_bytes(), data))

raw = row[0]  # bytes
recovered = Uint128.from_bytes(raw)
```

**Java**

```java
try (PreparedStatement ps = conn.prepareStatement("INSERT INTO events (id, data) VALUES (?, ?)")) {
    ps.setBytes(1, id.toBytes());
    ps.setString(2, data);
    ps.executeUpdate();
}

byte[] raw = rs.getBytes(1);           // 16 字节
Uint128 recovered = Uint128.fromBytes(raw);
```

## 时钟回拨行为

| 场景 | 行为 |
| ---- | ---- |
| 时间前进 | `physical` 更新到当前毫秒，`seq` 归零 |
| 时间不前进或回拨 | 继续增加 `seq`，保持结果单调 |
| `seq` 溢出 | 增加 `logical` |
| `logical` 溢出 | 人工推进 `physical` 1ms，避免倒退 |

这意味着该实现保证"编号单调"，不保证"高位时间始终等于真实墙钟"。

## 跨重启强唯一

只要满足下面两个前提，就可以在同一节点上跨重启避免重复 ID：

1. `nodeID` 在节点维度上稳定且唯一。
2. `WithStateFile(...)` / `with_state_file(...)` / `withStateFile(...)` 指向稳定、可持久化的状态文件。默认快速路径已覆盖单写者与进程顺序接管（failover）场景——新实例构造时读到前任写者的最新水位；若多个写者**同时活跃**共享同一文件，必须显式开启 `WithStateStrict(...)` / `with_state_strict(...)` / `withStateStrict(...)`（以每次发号增加一次磁盘读为代价，保证各写者水位同步）。

**Go**

```go
hlc := id128.NewHLC(
    id128.WithStateFile("./data/id.state"),
)
gen := id128.NewGenerator(1, hlc)
next := gen.Next()
```

**Python**

```python
hlc = HLC(with_state_file("./data/id.state"))
gen = Generator(1, hlc)
id = gen.next()
```

**Java**

```java
HLC hlc = new HLC(
        Option.withStateFile("./data/id.state"));
Generator gen = new Generator(1, hlc);
Uint128 id = gen.next();
```

**快速路径（默认）**：状态记录先在内存批量缓冲中累积，每攒满 `syncEvery` 条才在进程锁保护下一次性追加到文件末尾并 `fsync`；热路径零系统调用。重启后会恢复最后一条有效记录，再继续递增。进程异常退出最多丢失最近 `syncEvery-1` 条进度（这些 ID 重启后可能重复）；优雅退出前调用 `Close()` / `close()`（三语言一致）可把缓冲完整刷盘、零丢失。

**严格模式（`WithStateStrict`）**：每次 `Now()` / `now()` 都先获取进程锁、读盘取 max，再把最新 `(physical, logical, seq)` 作为一条校验过的状态记录追加到文件末尾，用于多写者同时活跃共享。

### 落盘间隔的默认值与环境变量

- 默认 `syncEvery = 1000`：快速路径下每攒满 1000 条状态记录才落盘一次（含锁 + `fsync`）。大多数请求不触碰磁盘，热路径纯内存；进程异常退出最多丢失最近 1000 条**持久化进度**，调用 `Close()` / `close()` 可零丢失。
- 可通过环境变量 `QUANT1X_ID128_SYNC_EVERY` 覆盖默认值（三语言一致），显式 Option 优先级最高：

```bash
# 每 100 条记录刷盘一次
QUANT1X_ID128_SYNC_EVERY=100 ./your-binary
```

- 极端低延迟场景可调大（如 10000），强调持久化可调小（如 1，等价于每条落盘）。

## 测试

### Go

```powershell
Push-Location .\quant1x\id\id128
go test -v
Pop-Location
```

测试覆盖：

- 时钟回拨单调性
- 字段编解码往返
- Option 生效验证
- 跨重启状态恢复
- 多实例共享状态文件
- 尾部坏损容错
- 200k goroutine 并发唯一性 + 有序性

### Python

```powershell
python -m unittest quant1x.id.id128.id_test
```

测试覆盖：

- 时钟回拨单调性
- 字段编解码
- 跨重启状态恢复
- 尾部坏损容错
- 20k thread 并发唯一性

### Java

```powershell
mvn -q test
```

测试覆盖：

- 时钟回拨单调性
- Option 生效验证
- 字段编解码
- 跨重启状态恢复
- 多实例共享状态文件
- 尾部坏损容错
- 200k thread 并发唯一性 + 有序性

## 基准测试

仅 Go 版提供（代码位于 `id_test.go`）：

```powershell
Push-Location .\quant1x\id\id128
go test -run '^$' -bench 'BenchmarkGeneratorNext$|BenchmarkGeneratorNextWithStateFile$|BenchmarkGeneratorNextWithStateFileSyncEvery256$|BenchmarkGeneratorNextWithStateFileStrict$' -benchmem
Pop-Location
```

本机测试环境：`goos=windows` `goarch=amd64` CPU: `12th Gen Intel(R) Core(TM) i7-12700T`

| 基准项 | ns/op | 约等于吞吐量 | B/op | allocs/op |
| ---- | ----: | ----: | ----: | ----: |
| `BenchmarkGeneratorNext` | `20.08` | `约 4980 万/s` | `0` | `0` |
| `BenchmarkGeneratorNextWithStateFile`（快速路径，批量缓冲） | `1,312` | `约 76 万/s` | `0` | `0` |
| `BenchmarkGeneratorNextWithStateFileSyncEvery256` | `5,284` | `约 19 万/s` | `3` | `0` |
| `BenchmarkGeneratorNextWithStateFileStrict`（严格模式） | `292,556` | `约 3418/s` | `1578` | `17` |

结果解读：

1. 纯内存路径适合高吞吐本地发号，数量级约为每秒数千万。
2. 快速路径（默认）：构造时恢复一次水位，运行期纯内存推进，状态记录在批量缓冲中累积、每攒满 `syncEvery`（默认 1000）条才落盘一次，吞吐约每秒 76 万，热路径零系统调用。
3. 严格模式（`WithStateStrict`）：每次发号一次磁盘读（尾部 18B + CRC）+ 一次落盘写，用于多写者活跃共享，吞吐约为每秒数千。
4. `WithStateSyncEvery(N)` 控制批量落盘频率：N 越小落盘越频繁、吞吐越低、崩溃丢失窗口越小；N 越大吞吐越高、丢失窗口越大。进程异常退出时最近最多 N 条进度可能尚未落盘，调用 `Close()` / `close()` 可零丢失。

## Go ↔ Python ↔ Java 横向对比

| 维度 | Go | Python | Java |
| ---- |:--:|:------:|:----:|
| Uint128 | `struct{Hi, Lo uint64}` + `bits.Add64/Sub64` | `@dataclass(frozen=True)` + Python 原生大整数 | hi/lo `long` 位模式 + `Long.compareUnsigned` |
| HLC.Now() | `(uint64, uint32)` | `(int, int)` | `Now{hlc, seq}` |
| Generator.Next() | `Uint128` | `Uint128` | `Uint128` |
| seq 管理 | HLC 内部 | HLC 内部 | HLC 内部 |
| 状态持久化 | CRC32 校验，定长 18 字节 | CRC32 校验，定长 18 字节 | CRC32 校验，定长 18 字节 |
| 文件锁 | 三平台 (`flock` / `LockFileEx` / `O_EXCL`) | 三平台 (`fcntl.flock` / `msvcrt` / `O_EXCL`) | `FileChannel.lock()`（Windows/Unix 通用） |
| 可注入时钟 | `WithClock` | `with_clock` | `withClock(LongSupplier)` |
| 随机种子 | `crypto/rand` | `secrets.randbits` | `SecureRandom` |
| 并发测试规模 | 200k goroutine | 20k thread | 200k thread |

## License

与项目主体保持一致。
