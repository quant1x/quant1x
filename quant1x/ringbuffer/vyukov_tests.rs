#![cfg(feature = "vyukov")]

// Feature-gated tests for a Vyukov-style bounded MPMC queue implementation.
// These tests are written TDD-style: they assume a `crate::ringbuffer::vyukov::Queue<T>`
// implementation will be provided later. They will not be compiled/run unless
// the `vyukov` Cargo feature is enabled.

use std::sync::Arc;
use std::sync::atomic::{AtomicI64, Ordering};
use std::thread;

// Note: Expected API of the Vyukov implementation (to be implemented later):
// pub mod vyukov {
//     pub struct Queue<T> { .. }
//     impl<T> Queue<T> {
//         pub fn new(capacity: usize) -> Self;
//         pub fn push(&self, value: T) -> Result<(), ()>; // Err on full or closed
//         pub fn pop(&self) -> Result<T, ()>; // Err on closed/empty
//         pub fn close(&self);
//     }
// }

#[test]
fn vyukov_basic() {
    // Basic functionality smoke test (enqueue/dequeue ordering)
    use crate::ringbuffer::vyukov::Queue as VQueue;

    let q = VQueue::<i32>::new(4);
    assert!(q.push(1).is_ok());
    assert!(q.push(2).is_ok());

    let a = q.pop().expect("expected value");
    let b = q.pop().expect("expected value");
    assert_eq!(a, 1);
    assert_eq!(b, 2);

    q.close();
}

#[test]
fn vyukov_mpmc_performance() {
    use std::time::Instant;
    use crate::ringbuffer::vyukov::Queue as VQueue;

    const SIZE: usize = 65536;
    const NUM_PRODUCERS: usize = 8;
    const NUM_CONSUMERS: usize = 8;
    const DATA_PER_PRODUCER: i64 = 30000;
    const TOTAL: i64 = NUM_PRODUCERS as i64 * DATA_PER_PRODUCER;

    let q = Arc::new(VQueue::<i64>::new(SIZE));

    let start = Instant::now();

    // Spawn producers
    let mut producers = Vec::with_capacity(NUM_PRODUCERS);
    for id in 0..NUM_PRODUCERS {
        let q_clone = q.clone();
        producers.push(thread::spawn(move || {
            let base = (id as i64) * DATA_PER_PRODUCER;
            for i in 0..DATA_PER_PRODUCER {
                // spin until pushed
                loop {
                    if q_clone.push(base + i).is_ok() {
                        break;
                    }
                    thread::yield_now();
                }
            }
        }));
    }

    // Consumers
    let consumed = Arc::new(AtomicI64::new(0));
    let mut consumers = Vec::with_capacity(NUM_CONSUMERS);
    for _ in 0..NUM_CONSUMERS {
        let q_clone = q.clone();
        let consumed_clone = consumed.clone();
        consumers.push(thread::spawn(move || {
            loop {
                match q_clone.pop() {
                    Ok(_v) => {
                        consumed_clone.fetch_add(1, Ordering::Relaxed);
                    }
                    Err(_) => {
                        // If closed and empty, break
                        break;
                    }
                }
            }
        }));
    }

    // Wait for producers
    for p in producers {
        p.join().unwrap();
    }

    // Close queue and wait for consumers
    q.close();
    for c in consumers {
        c.join().unwrap();
    }

    let elapsed = start.elapsed();
    let got = consumed.load(Ordering::Relaxed);
    assert_eq!(got, TOTAL, "expected to consume all items");

    let elapsed_secs = elapsed.as_secs_f64();
    let ops = (TOTAL as f64) / elapsed_secs;
    println!("Vyukov MPMC: producers={}, consumers={}, items={}, ops/sec={:.2}", NUM_PRODUCERS, NUM_CONSUMERS, TOTAL, ops);
}
