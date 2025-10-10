// Thin performance driver that uses the library's Vyukov queue implementation.
use std::sync::atomic::Ordering;
use std::sync::Arc;
use std::thread;
use std::time::Instant;

use quant1x::runtime::ringbuffer::Queue as VQueue;

fn run_perf() -> f64 {
    const SIZE: usize = 65536;
    const NUM_PRODUCERS: usize = 8;
    const NUM_CONSUMERS: usize = 8;
    const DATA_PER_PRODUCER: i64 = 300000;
    const TOTAL: i64 = NUM_PRODUCERS as i64 * DATA_PER_PRODUCER;

    let q = Arc::new(VQueue::<i64>::new(SIZE));
    let start = Instant::now();

    let mut producers = Vec::with_capacity(NUM_PRODUCERS);
    for id in 0..NUM_PRODUCERS {
        let q_clone = q.clone();
        producers.push(thread::spawn(move || {
            let base = (id as i64) * DATA_PER_PRODUCER;
            for i in 0..DATA_PER_PRODUCER {
                loop {
                    if q_clone.push(base + i).is_ok() {
                        break;
                    }
                    thread::yield_now();
                }
            }
        }));
    }

    let consumed = Arc::new(std::sync::atomic::AtomicI64::new(0));
    let mut consumers = Vec::with_capacity(NUM_CONSUMERS);
    for _ in 0..NUM_CONSUMERS {
        let q_clone = q.clone();
        let consumed_clone = consumed.clone();
        consumers.push(thread::spawn(move || loop {
            match q_clone.pop() {
                Ok(_v) => {
                    consumed_clone.fetch_add(1, Ordering::Relaxed);
                }
                Err(_) => break,
            }
        }));
    }

    for p in producers {
        p.join().unwrap();
    }
    q.close();
    for c in consumers {
        c.join().unwrap();
    }

    let elapsed = start.elapsed();
    let got = consumed.load(Ordering::Relaxed);
    assert_eq!(got, TOTAL, "expected to consume all items");
    let elapsed_secs = elapsed.as_secs_f64();
    (TOTAL as f64) / elapsed_secs
}

fn main() {
    let ops = run_perf();
    println!("Vyukov MPMC ops/sec = {}", ops);
}

#[cfg(test)]
mod test_driver {
    use super::*;

    #[test]
    #[ignore]
    fn vyukov_perf_main_as_test() {
        let ops = run_perf();
        println!("[ignored test] Vyukov MPMC ops/sec = {}", ops);
    }
}
