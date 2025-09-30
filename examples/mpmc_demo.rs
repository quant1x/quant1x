use std::sync::{Arc, atomic::{AtomicI64, Ordering}};
use std::thread;
use std::time::Instant;

use quant1x::ringbuffer::vyukov::Queue;

fn main() {
    // small demo values so it runs quickly
    const SIZE: usize = 1024;
    const NUM_PRODUCERS: usize = 4;
    const NUM_CONSUMERS: usize = 4;
    const PER_PRODUCER: i64 = 10000;
    const TOTAL: i64 = NUM_PRODUCERS as i64 * PER_PRODUCER;

    let q = Arc::new(Queue::<i64>::new(SIZE));
    let consumed = Arc::new(AtomicI64::new(0));

    let start = Instant::now();

    // producers
    let mut producers = Vec::new();
    for id in 0..NUM_PRODUCERS {
        let qp = q.clone();
        producers.push(thread::spawn(move || {
            let base = (id as i64) * PER_PRODUCER;
            for i in 0..PER_PRODUCER {
                loop {
                    if qp.push(base + i).is_ok() { break; }
                    thread::yield_now();
                }
            }
        }));
    }

    // consumers
    let mut consumers = Vec::new();
    for _ in 0..NUM_CONSUMERS {
        let qc = q.clone();
        let c = consumed.clone();
        consumers.push(thread::spawn(move || {
            loop {
                match qc.pop() {
                    Ok(_v) => { c.fetch_add(1, Ordering::Relaxed); }
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
    println!("demo consumed {} items (expected {})", got, TOTAL);
    let ops = (TOTAL as f64) / elapsed.as_secs_f64();
    println!("demo ops/sec = {:.2}", ops);
}
