use quant1x::runtime::ringbuffer::Queue as VQueue;
use std::sync::Arc;
use std::sync::atomic::{AtomicI64, Ordering};
use std::thread;
use std::time::Instant;

fn run_vyukov_once() -> f64 {
    const SIZE: usize = 65536;
    const NUM_PRODUCERS: usize = 8;
    const NUM_CONSUMERS: usize = 8;
    const DATA_PER_PRODUCER: i64 = 300000; // match earlier run
    const TOTAL: i64 = NUM_PRODUCERS as i64 * DATA_PER_PRODUCER;

    let q = Arc::new(VQueue::<i64>::new(SIZE));
    let start = Instant::now();

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

fn main() {
    const RUNS: usize = 10;
    let mut results = Vec::with_capacity(RUNS);
    for i in 0..RUNS {
        eprintln!("Run {}...", i + 1);
        let ops = run_vyukov_once();
        println!("{}", ops);
        results.push(ops);
    }

    // compute simple stats
    let n = results.len() as f64;
    let mean = results.iter().copied().sum::<f64>() / n;
    let mut sorted = results.clone();
    sorted.sort_by(|a, b| a.partial_cmp(b).unwrap());
    let median = sorted[sorted.len() / 2];
    let var = results.iter().map(|v| (v - mean).powi(2)).sum::<f64>() / (n - 1.0);
    let stddev = var.sqrt();

    eprintln!("--- summary ---");
    eprintln!("runs = {}", results.len());
    eprintln!("mean = {}", mean);
    eprintln!("median = {}", median);
    eprintln!("stddev = {}", stddev);
}
