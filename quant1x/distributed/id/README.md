# Distributed ID

`quant1x/distributed/id` 是 distributed 层的 Go 分布式 ID 实现，包名为 `id`。

与默认实现 `quant1x/id/id64` 相互独立、互不引用：ID 编码、HLC、状态文件
（mmap 双槽 checkpoint + 锁字互斥）均在本目录维护；
MPMC 无锁队列复用 `quant1x/runtime.RingBuffer`（Go 版 Vyukov 环形缓冲）。

## 特性

- 64 位可排序 ID
- 基于 HLC 保证单实例内严格递增
- 支持时钟回拨，回拨时递增序列号
- 支持动态节点位宽
- base64url 无填充字符串编码
- 基于 Go 版 Vyukov MPMC ringbuffer 的 ID 队列
- 多生产者、多消费者并发安全
- 可选定长 mmap 状态文件持久化
- 跨进程互斥使用共享映射内的锁字（pid + stamp CAS），持锁进程死亡自动抢占，无锁文件
- Windows、Unix 和兼容平台的跨平台支持

## 位布局

```text
| 1bit 符号(恒 0) | Physical(41bit) | NodeID(workerBits) | Seq(seqBits) |
```

- Epoch：`2026-01-01T00:00:00Z`
- `workerBits = bits.Len(nodeCount)`
- `seqBits = 22 - workerBits`
- `seqBits` 最小为 4（即节点数上限 2^18）

## 使用方式

```go
import "github.com/quant1x/quant1x/quant1x/distributed/id"

clock := id.NewHLC(id.WithNodeCount(1024))
generator := id.NewGenerator(1, clock)

value := generator.Next()
text := value.String()
bytes := value.Bytes()

_ = text
_ = bytes
```

## ID 队列

```go
queue, err := id.NewQueue(1024)
if err != nil {
    panic(err)
}

generator := id.NewGenerator(1, id.NewHLC())
if err := queue.TryPush(generator.Next()); err != nil {
    // 队列满或已关闭
}

value, err := queue.TryPop()
if err == nil {
    _ = value
}
```

`TryPush` 和 `TryPop` 是非阻塞操作；`Push` 和 `Pop` 在队列满或空时等待。

### 后台批量生产（Serve）

把发号器接入队列：后台 goroutine 持续发号填满队列，消费端 `TryPop`
无锁取号，把 HLC 互斥锁开销从每次发号摊薄为后台批量生产。

```go
queue, err := id.NewQueue(4096)
if err != nil {
    panic(err)
}
generator := id.NewGenerator(1, id.NewHLC())

ctx, cancel := context.WithCancel(context.Background())
defer cancel()
go generator.Serve(ctx, queue) // 生产：队满时阻塞等待

value, err := queue.TryPop()   // 消费：无锁
```

语义：
- `Serve` 在 ctx 取消时返回 `ctx.Err()`；队列关闭时返回 nil 并停止发号
- 取消后队列中的存量 ID 仍可被消费者排空（关闭的队列允许读尽后返回 `ErrClosed`）
- 多个 goroutine 可并发对同一队列 `Serve`（MPMC 安全）

## API

### ID

- `Bytes()`：返回 BigEndian 的 8 字节表示
- `String()`：返回 11 字符的 base64url 无填充字符串
- `Physical()`：返回 epoch 相对毫秒
- `NodeID(workerBits)`：解析节点 ID
- `Seq(workerBits)`：解析序列号
- `FromBytes([8]byte)`：从 BigEndian 字节恢复 ID

### HLC

- `NewHLC(options...)`
- `HLC.Now()`：返回严格递增的 (physical, seq)
- `HLC.SeqBits()` / `HLC.Timestamp()`
- `HLC.Close()`：刷新未 checkpoint 的水位并释放映射；优雅退出前调用

### Generator

- `NewGenerator(nodeID, hlc)`：nodeID 越界或 hlc 为 nil 时 panic
- `Generator.Next()`：发号，全局严格递增
- `Generator.Serve(ctx, queue)`：后台持续生产填满队列；ctx 取消返回
  `ctx.Err()`，队列关闭返回 nil
- `Generator.WorkerBits()`

### Options

- `WithClock(now func() int64)`：覆盖时钟（返回绝对毫秒），测试用
- `WithSeqSeed(seed uint16)`：初始 seq 种子（默认随机，降低重启碰撞）
- `WithNodeCount(count uint32)`：按节点总数推导位宽
- `WithSeqBits(seqBits uint8)`：直接指定位宽（[4, 21]）
- `WithStateFile(path string)`：启用 mmap 状态文件持久化
- `WithStateSyncEvery(every uint32)`：每 N 次发号 msync 一次（默认 1000，
  决定崩溃丢失窗口与吞吐的折中；环境变量 `QUANT1X_ID64_SYNC_EVERY` 可设默认值）
- `WithStateStrict()`：多写者活跃共享模式，每次发号锁内读取共享映射取 max

### Queue

- `NewQueue`
- `TryPush` / `TryPop`
- `Push` / `Pop`
- `Len` / `Cap`
- `Close` / `IsClosed`
- `WaitForClose`

## 默认实现边界

`quant1x/id/id64` 是项目默认的 id64 实现（追加式状态日志 + 平台文件锁）。
本包是 distributed 层的独立实现，不引用默认 `id64` 包；
与它共享同一组测试语义（单调、唯一、恢复），但持久化格式不同，
两者的状态文件不能混用。MPMC 无锁队列复用 `quant1x/runtime.RingBuffer`。

启用 `WithStateFile` 后，状态文件是定长 128 字节的共享映射：
前 64 字节为双槽 checkpoint（generation 和 CRC 选最新有效状态，避免文件无限增长），
偏移 64 起为 8 字节跨进程锁字——高 32 位为持锁 PID，低 32 位为加锁时间戳；
CAS(0 -> mine) 加锁、CAS(mine -> 0) 解锁。持锁进程异常退出后，
等待方通过进程探活立即抢占（无法探测存活的平台由 30 秒 TTL 兜底），
不依赖内核文件锁，也不产生 `.lock` 伴生文件。
旧版追加式日志文件会在构造时读取水位后自动迁移为新布局。

## 测试

```bash
go test ./quant1x/distributed/id
```

测试覆盖：

- HLC 时钟回拨后的单调性
- 节点数到 worker/sequence 位宽的计算
- ID 字段解析与 BigEndian/base64 编码往返
- 多 goroutine 发号唯一性
- Serve 生产-消费链路：递增唯一性、关闭即停、取消后排空
- ringbuffer 队列的容量、入队、出队和关闭行为

运行基准：

```bash
go test ./quant1x/distributed/id -run '^$' -bench . -benchmem -benchtime=1s
```

## 性能

以下为 Windows amd64 本地实测（i7-12700T，20 逻辑核），`-benchmem -benchtime=1s`。
绝对数值随硬件与时钟源变化，量级与相对关系具有参考意义。

> 读法说明：`ns/op` 是完成一次操作所需的纳秒数（10 亿分之一秒），越小越快；
> "每秒" 是它的倒数换算，即单个 goroutine 一秒能完成的次数，越大越快。
> 并发基准的速率为全部 worker 加总的聚合值。
>
> 直观参照：敲一个键约 0.2 秒 = 2 亿纳秒。也就是说，本包发一个 ID 的耗时
> 只有敲一个键的千万分之一量级。

### 发号热路径

| 基准 | 场景 | 单次耗时 | 每秒生成 |
|---|---|---|---|
| `BenchmarkHLCNow` | HLC 推进（不组 ID） | 12.9 ns · 0 alloc | 约 7,700 万 |
| `BenchmarkGeneratorNext` | 直接发号（固定时钟） | 15.2 ns · 0 alloc | **约 6,600 万** |
| `BenchmarkGeneratorNextDefault` | 直接发号（真实时钟） | 17.8 ns · 0 alloc | 约 5,600 万 |
| `BenchmarkGeneratorNextParallel` | 20 goroutine 并发发号 | 56.1 ns · 0 alloc | 约 1,800 万（聚合） |
| `BenchmarkIDBytes` | BigEndian 8 字节编码 | 4.5 ns · 0 alloc | 约 2.2 亿（编码） |
| `BenchmarkIDString` | base64url 字符串化 | 21.5 ns · 16 B · 1 alloc | 约 4,600 万（编码） |

无状态文件时单实例吞吐约 **6600 万 ID/s**，全程零内存分配。
真实时钟比固定时钟多 ~2.6ns，是读系统时间（VDSO 调用）的开销。
并发发号聚合速度降到约每秒 1800 万，来自互斥锁在多核间的缓存行争用。

### 状态文件持久化（mmap 双槽 checkpoint）

| 基准 | syncEvery | 单次耗时 | 每秒生成 |
|---|---|---|---|
| `BenchmarkGeneratorNextStateFile` | 1000（默认） | 923 ns · 0 alloc | 约 108 万 |
| `BenchmarkGeneratorNextStateFileSyncEvery` | 256 | 3.52 µs · 0 alloc | 约 28 万 |
| `BenchmarkGeneratorNextStateFileStrict` | 1000（锁内读-改-写） | 1.05 µs · 24 B · 1 alloc | 约 95 万 |

启用持久化后吞吐由磁盘刷写节流决定，摊销近似“单次刷盘耗时 ÷ syncEvery”：
Windows `FlushFileBuffers` 实测约 0.9 毫秒，syncEvery=1000 对应每秒约 108 万，
syncEvery=256 对应约 28 万——两组数据的比值验证了同一模型。
调大 `WithStateSyncEvery` 可线性提升吞吐，代价是进程崩溃后可能重复的 ID 上限变大；
Linux 的 fdatasync 通常低于 100 微秒，同等配置明显更快。

### ID 队列与 Serve 流水线

| 基准 | 场景 | 单次耗时 | 每秒取号 |
|---|---|---|---|
| `BenchmarkQueuePushPop` | 单 goroutine 入队+出队 | 24.6 ns · 0 alloc | 约 4,000 万 |
| `BenchmarkQueueMixedParallel` | 20 goroutine 混合读写 | 225.5 ns · 0 alloc | 约 440 万（聚合） |
| `BenchmarkGeneratorServeTryPop` | 后台 Serve 生产 + TryPop 取号 | 104.4 ns · 0 alloc | 约 960 万 |

对 Serve 流水线的解读：
单次取号 ~104ns 高于直接 Next 的 ~15ns，开销来自跨 goroutine 的
producer/consumer 缓存行交互与偶发空轮询。它的价值不在单线程延迟：

- **消费者线性扩展**：消费端是纯无锁 CAS 出队；直接 Next 在并发下走互斥锁，
  聚合速度掉到约每秒 1800 万且随核数继续恶化；
  队列方案把争用收敛到后台生产者一侧
- **发号热点隔离**：HLC 互斥锁只被生产者 goroutine 触碰

选型建议：低频调用直接 `generator.Next()`（最快）；高并发消费网关用
`Serve` + `TryPop` 流水线（多核下更平稳）。
