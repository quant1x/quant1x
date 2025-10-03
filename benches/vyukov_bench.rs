use criterion::{criterion_group, criterion_main, Criterion, Throughput};
use std::sync::Arc;
use std::sync::atomic::{AtomicI64, Ordering};
use std::thread;

use quant1x::runtime::ringbuffer::Queue as VQueue;

fn run_vyukov_once() -> f64 {
    const SIZE: usize = 65536;
    const NUM_PRODUCERS: usize = 8;
    const NUM_CONSUMERS: usize = 8;
    // Use a smaller per-iteration workload so Criterion can repeat the measurement
    const DATA_PER_PRODUCER: i64 = 5_000;
    const TOTAL: i64 = NUM_PRODUCERS as i64 * DATA_PER_PRODUCER;

    let q = Arc::new(VQueue::<i64>::new(SIZE));
    let start = std::time::Instant::now();

    // producers
    let mut producers = Vec::with_capacity(NUM_PRODUCERS);
    for id in 0..NUM_PRODUCERS {
        let q2 = q.clone();
        producers.push(thread::spawn(move || {
            let base = (id as i64) * DATA_PER_PRODUCER;
            for i in 0..DATA_PER_PRODUCER {
                loop {
                    if q2.push(base + i).is_ok() { break; }
                    thread::yield_now();
                }
            }
        }));
    }

    // consumers
    let consumed = Arc::new(AtomicI64::new(0));
    let mut consumers = Vec::with_capacity(NUM_CONSUMERS);
    for _ in 0..NUM_CONSUMERS {
        let q2 = q.clone();
        let c2 = consumed.clone();
        consumers.push(thread::spawn(move || {
            loop {
                match q2.pop() {
                    Ok(_) => { c2.fetch_add(1, Ordering::Relaxed); }
                    Err(_) => break,
                }
            }
        }));
    }

    for p in producers { p.join().unwrap(); }
    q.close();
    for c in consumers { c.join().unwrap(); }

    let elapsed = start.elapsed();
    let got = consumed.load(Ordering::Relaxed);
    assert_eq!(got, TOTAL, "expected all items");
    let secs = elapsed.as_secs_f64();
    (TOTAL as f64) / secs
}

fn vyukov_benchmark(c: &mut Criterion) {
    let mut group = c.benchmark_group("vyukov_mpmc");
    group.throughput(Throughput::Elements(8 * 300000)); // total operations
    group.bench_function("mpmc_performance", |b| {
        b.iter(|| run_vyukov_once());
    });
    group.finish();
}

criterion_group!(vyukov_group, vyukov_benchmark);
criterion_main!(vyukov_group);
