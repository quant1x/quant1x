// 独立的 Vyukov 有界 MPMC 队列（库变体 - 无 main），带对齐和退避策略
use std::cell::UnsafeCell;
use std::mem::MaybeUninit;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;
use std::thread;
use std::time::Duration;
use std::hint;

#[repr(align(64))]
struct Slot<T> {
    seq: AtomicUsize,
    data: UnsafeCell<MaybeUninit<T>>,
}

unsafe impl<T: Send> Send for Slot<T> {}
unsafe impl<T: Send> Sync for Slot<T> {}

#[repr(align(64))]
struct AlignedAtomicUsize(AtomicUsize);

impl AlignedAtomicUsize {
    fn new(v: usize) -> Self { AlignedAtomicUsize(AtomicUsize::new(v)) }
    fn load(&self, o: Ordering) -> usize { self.0.load(o) }
    fn store(&self, v: usize, o: Ordering) { self.0.store(v, o) }
    fn compare_exchange(&self, a: usize, b: usize, s: Ordering, f: Ordering) -> Result<usize, usize> { self.0.compare_exchange(a, b, s, f) }
}

pub struct Queue<T> {
    buffer: Vec<Slot<T>>,
    mask: usize,
    enqueue_pos: AlignedAtomicUsize,
    dequeue_pos: AlignedAtomicUsize,
    closed: AtomicUsize,
}

unsafe impl<T: Send> Send for Queue<T> {}
unsafe impl<T: Send> Sync for Queue<T> {}

impl<T> Queue<T> {
    pub fn new(capacity: usize) -> Self {
        let cap = capacity.next_power_of_two();
        let mut buffer = Vec::with_capacity(cap);
        for i in 0..cap {
            buffer.push(Slot {
                seq: AtomicUsize::new(i),
                data: UnsafeCell::new(MaybeUninit::uninit()),
            });
        }
        Self {
            buffer,
            mask: cap - 1,
            enqueue_pos: AlignedAtomicUsize::new(0),
            dequeue_pos: AlignedAtomicUsize::new(0),
            closed: AtomicUsize::new(0),
        }
    }

    #[inline]
    fn backoff_spin(iter: &mut u32) {
        if *iter < 4 {
            hint::spin_loop();
        } else if *iter < 8 {
            thread::yield_now();
        } else {
            thread::sleep(Duration::from_micros(50));
        }
        *iter = iter.saturating_add(1);
    }

    pub fn push(&self, value: T) -> Result<(), ()> {
        let mut backoff = 0u32;
        loop {
            let pos = self.enqueue_pos.load(Ordering::Relaxed);
            let index = pos & self.mask;
            let slot = &self.buffer[index];
            let seq = slot.seq.load(Ordering::Acquire);
            if seq == pos {
                if self.enqueue_pos.compare_exchange(pos, pos + 1, Ordering::SeqCst, Ordering::Relaxed).is_ok() {
                    unsafe { (*slot.data.get()).write(value); }
                    slot.seq.store(pos + 1, Ordering::Release);
                    return Ok(());
                } else {
                    Self::backoff_spin(&mut backoff);
                    continue;
                }
            } else if seq < pos {
                // 队列已满
                return Err(());
            } else {
                Self::backoff_spin(&mut backoff);
                continue;
            }
        }
    }

    pub fn pop(&self) -> Result<T, ()> {
        let mut backoff = 0u32;
        loop {
            let pos = self.dequeue_pos.load(Ordering::Relaxed);
            let index = pos & self.mask;
            let slot = &self.buffer[index];
            let seq = slot.seq.load(Ordering::Acquire);
            if seq == pos + 1 {
                if self.dequeue_pos.compare_exchange(pos, pos + 1, Ordering::SeqCst, Ordering::Relaxed).is_ok() {
                    let val = unsafe { (*slot.data.get()).assume_init_read() };
                    slot.seq.store(pos + self.mask + 1, Ordering::Release);
                    return Ok(val);
                } else {
                    Self::backoff_spin(&mut backoff);
                    continue;
                }
            } else if seq < pos + 1 {
                if self.closed.load(Ordering::Acquire) != 0 {
                    return Err(());
                }
                Self::backoff_spin(&mut backoff);
                continue;
            } else {
                Self::backoff_spin(&mut backoff);
                continue;
            }
        }
    }

    pub fn close(&self) {
        // 将 closed 标志置位以表明队列已关闭
        self.closed.store(1, Ordering::Release);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::Arc;

    #[test]
    fn basic() {
        let q = Queue::new(4);
        assert!(q.push(1).is_ok());
        assert!(q.push(2).is_ok());
        assert_eq!(q.pop().unwrap(), 1);
        assert_eq!(q.pop().unwrap(), 2);
    }

    #[test]
    fn mpmc_small() {
        const SIZE: usize = 64;
        let q = Arc::new(Queue::new(SIZE));
        const NUM_PROD: usize = 4;
        const NUM_CONS: usize = 4;
        const PER: i64 = 1000;
        let mut producers = vec![];
        for id in 0..NUM_PROD {
            let q2 = q.clone();
            producers.push(std::thread::spawn(move || {
                let base = (id as i64) * PER;
                for i in 0..PER {
                    loop {
                        if q2.push(base + i).is_ok() { break; }
                        std::thread::yield_now();
                    }
                }
            }));
        }
        let consumed = Arc::new(std::sync::atomic::AtomicUsize::new(0));
        let mut consumers = vec![];
        for _ in 0..NUM_CONS {
            let q2 = q.clone();
            let c = consumed.clone();
            consumers.push(std::thread::spawn(move || {
                loop {
                    match q2.pop() {
                        Ok(_v) => { c.fetch_add(1, Ordering::Relaxed); }
                        Err(_) => break,
                    }
                }
            }));
        }

        for p in producers { p.join().unwrap(); }
        q.close();
        for c in consumers { c.join().unwrap(); }
        assert_eq!(consumed.load(Ordering::Relaxed), (NUM_PROD as usize) * (PER as usize));
    }

    // Heavy performance/stress test matching Go TestMPMCPerformance scale.
    // Marked #[ignore] because it is long-running; run with
    // `cargo test --manifest-path ... --features vyukov --release -- --ignored mpmc_performance_heavy`
    #[test]
    #[ignore]
    fn mpmc_performance_heavy() {
        use std::time::Instant;

        const SIZE: usize = 65536;
        const NUM_PRODUCERS: usize = 8;
        const NUM_CONSUMERS: usize = 8;
        const DATA_PER_PRODUCER: i64 = 300_000;
        const TOTAL: i64 = NUM_PRODUCERS as i64 * DATA_PER_PRODUCER;

        let q = std::sync::Arc::new(Queue::new(SIZE));
        let start = Instant::now();

        let mut producers = Vec::with_capacity(NUM_PRODUCERS);
        for id in 0..NUM_PRODUCERS {
            let q2 = q.clone();
            producers.push(std::thread::spawn(move || {
                let base = (id as i64) * DATA_PER_PRODUCER;
                for i in 0..DATA_PER_PRODUCER {
                    loop {
                        if q2.push(base + i).is_ok() { break; }
                        std::thread::yield_now();
                    }
                }
            }));
        }

        let consumed = std::sync::Arc::new(std::sync::atomic::AtomicI64::new(0));
        let mut consumers = Vec::with_capacity(NUM_CONSUMERS);
        for _ in 0..NUM_CONSUMERS {
            let q2 = q.clone();
            let c2 = consumed.clone();
            consumers.push(std::thread::spawn(move || {
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
        let ops = (TOTAL as f64) / secs;
        println!("heavy test ops/sec = {}", ops);
    }
}
