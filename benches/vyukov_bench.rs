// Vyukov 有界 MPMC 队列的 Criterion 基准测试
//
// 修复说明 (对应 benches 此前存在的缺陷):
// - Cargo.toml 必须声明 `harness = false`, 否则 libtest 注入的 main 会覆盖
//   criterion_main!, 导致基准一次都不运行且静默通过;
// - Throughput 由 TOTAL 常量推导, 早期硬编码 8*300000 与实际工作量脱节,
//   吞吐被高估 60 倍;
// - 同时覆盖背压 (小容量) 与无背压 (大容量) 两种场景: 早期容量 65536 大于单轮总量,
//   队列永不满, 背压路径从未被测量;
// - 线程创建开销以差分方式扣除 (见 spawn_and_join_idle_threads): Windows 上
//   16 个线程的创建+join 约 1ms, 早期实现每轮仅处理 4 万条、耗时约 4ms,
//   该开销占比可达 25% 且被直接计入队列吞吐;
// - 单轮数据量取 16 万条, 使扣除后剩余耗时远大于线程开销, 降低差分测量的相对误差.
//
// 关于线程复用: 曾尝试复用生产/消费线程以彻底消除创建开销, 实测反而劣化且方差极大
// (6.8 Melem/s → 0.3 Melem/s). 原因是常驻消费者在两轮之间持续阻塞在 pop 上,
// 而 pop 在队列空时会退避到 sleep(50us); 下一轮开始时必须先等这些睡眠中的消费者
// 被唤醒, 该延迟远大于线程创建开销. 因此保留每轮创建线程的结构.

use std::sync::Arc;
use std::sync::atomic::{AtomicI64, Ordering};
use std::thread;
use std::time::Instant;

use criterion::{Criterion, Throughput, black_box, criterion_group, criterion_main};

use quant1x::runtime::ringbuffer::Queue as VQueue;

const NUM_PRODUCERS: usize = 8;
const NUM_CONSUMERS: usize = 8;
/// 每个生产者单轮生产条数: 取较大值以摊薄线程创建开销
const DATA_PER_PRODUCER: i64 = 20_000;
/// 单轮总元素数: throughput 声明与结果断言均以此为准, 杜绝硬编码脱节
const TOTAL: i64 = (NUM_PRODUCERS as i64) * DATA_PER_PRODUCER;

/// 背压场景: 容量远小于单轮总量, push 必然阻塞等待消费者腾位
const CAPACITY_BACKPRESSURE: usize = 1024;
/// 无背压场景: 容量大于单轮总量, push 不会阻塞 (每个 slot 64B, 约 16MB)
const CAPACITY_UNCONTENDED: usize = 1 << 18;

/// 运行一轮完整的生产-消费, 返回消费总数
fn run_round(capacity: usize) -> i64 {
    let queue = Arc::new(VQueue::<i64>::new(capacity));
    let consumed = Arc::new(AtomicI64::new(0));

    let mut producers = Vec::with_capacity(NUM_PRODUCERS);
    for id in 0..NUM_PRODUCERS {
        let queue = queue.clone();
        producers.push(thread::spawn(move || {
            let base = id as i64 * DATA_PER_PRODUCER;
            for index in 0..DATA_PER_PRODUCER {
                // push 写共享内存, 不会被优化掉; 热路径不放 black_box 以免干扰测量
                loop {
                    if queue.push(base + index).is_ok() {
                        break;
                    }
                    // 队满: 让出时间片等待消费者腾位
                    thread::yield_now();
                }
            }
        }));
    }

    let mut consumers = Vec::with_capacity(NUM_CONSUMERS);
    for _ in 0..NUM_CONSUMERS {
        let queue = queue.clone();
        let consumed = consumed.clone();
        consumers.push(thread::spawn(move || {
            loop {
                match queue.pop() {
                    Ok(_) => {
                        consumed.fetch_add(1, Ordering::Relaxed);
                    }
                    // pop 仅在队列关闭且为空时返回 Err
                    Err(_) => break,
                }
            }
        }));
    }

    for producer in producers {
        producer.join().expect("生产者线程崩溃");
    }
    // 关闭队列, 消费者排空存量后退出
    queue.close();
    for consumer in consumers {
        consumer.join().expect("消费者线程崩溃");
    }
    consumed.load(Ordering::Acquire)
}

/// 创建并回收与一轮生产消费等量的线程, 但不做任何队列操作
///
/// 用于差分扣除线程创建开销: Windows 上 16 个线程的创建+join 约 1ms,
/// 在单轮耗时中占比可达 10%, 不扣除会把线程调度成本算进队列吞吐.
fn spawn_and_join_idle_threads() {
    let mut handles = Vec::with_capacity(NUM_PRODUCERS + NUM_CONSUMERS);
    for _ in 0..(NUM_PRODUCERS + NUM_CONSUMERS) {
        handles.push(thread::spawn(|| black_box(0)));
    }
    for handle in handles {
        handle.join().expect("线程崩溃");
    }
}

fn bench_mpmc(c: &mut Criterion, name: &str, capacity: usize) {
    let mut group = c.benchmark_group("vyukov_mpmc");
    group.throughput(Throughput::Elements(TOTAL as u64));
    group.bench_function(name, |b| {
        b.iter_custom(|iters| {
            // 完整轮次 (含线程创建与回收)
            let start = Instant::now();
            for _ in 0..iters {
                let consumed = run_round(capacity);
                assert_eq!(consumed, TOTAL, "每轮必须消费全部元素");
                black_box(consumed);
            }
            let total = start.elapsed();

            // 等量轮次的空转线程开销, 从总耗时中扣除, 得到纯队列操作耗时
            let start = Instant::now();
            for _ in 0..iters {
                spawn_and_join_idle_threads();
            }
            let overhead = start.elapsed();
            total.saturating_sub(overhead)
        });
    });
    group.finish();
}

/// 对照基准: 16 个线程的创建+join 开销
///
/// 两个 mpmc 基准已用差分扣除该开销, 本项单独保留是为了让开销可见,
/// 便于判断单轮耗时中线程调度的占比是否仍需要进一步优化.
fn bench_thread_overhead(c: &mut Criterion) {
    let mut group = c.benchmark_group("vyukov_mpmc");
    group.bench_function("thread_overhead", |b| {
        b.iter(|| {
            let mut handles = Vec::with_capacity(NUM_PRODUCERS + NUM_CONSUMERS);
            for _ in 0..(NUM_PRODUCERS + NUM_CONSUMERS) {
                handles.push(thread::spawn(|| black_box(0)));
            }
            for handle in handles {
                handle.join().expect("线程崩溃");
            }
        });
    });
    group.finish();
}

fn vyukov_benchmark(c: &mut Criterion) {
    bench_mpmc(c, "mpmc_backpressure", CAPACITY_BACKPRESSURE);
    bench_mpmc(c, "mpmc_uncontended", CAPACITY_UNCONTENDED);
    bench_thread_overhead(c);
}

criterion_group!(vyukov_group, vyukov_benchmark);
criterion_main!(vyukov_group);
