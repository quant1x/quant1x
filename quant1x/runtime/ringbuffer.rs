// 独立的 Vyukov 有界 MPMC 队列(库变体 - 无 main), 带对齐和退避策略
use std::cell::UnsafeCell;
use std::hint;
use std::mem::MaybeUninit;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;
use std::thread;
use std::time::Duration;

/// 单个槽位(缓存行对齐)
///
/// `Slot` 包含序列号用于确认读写阶段, 以及一个未初始化的数据存储区. 
#[repr(align(64))]
struct Slot<T> {
    /// 序列号, 用于判断该槽位处于可读/可写的哪个阶段
    seq: AtomicUsize,
    /// 数据存储(未初始化), 通过 UnsafeCell 包装以支持无锁写入/读取
    data: UnsafeCell<MaybeUninit<T>>,
}

unsafe impl<T: Send> Send for Slot<T> {}
unsafe impl<T: Send> Sync for Slot<T> {}

/// 对齐包装的 AtomicUsize, 避免不同原子变量因伪共享导致性能下降
#[repr(align(64))]
struct AlignedAtomicUsize(AtomicUsize);

impl AlignedAtomicUsize {
    /// 创建一个新的对齐原子值
    fn new(v: usize) -> Self {
        AlignedAtomicUsize(AtomicUsize::new(v))
    }
    /// 读取原子值(使用给定的内存顺序)
    fn load(&self, o: Ordering) -> usize {
        self.0.load(o)
    }
    /// 存储原子值(使用给定的内存顺序)
    fn store(&self, v: usize, o: Ordering) {
        self.0.store(v, o)
    }
    /// CAS 操作(比较并交换)
    fn compare_exchange(
        &self,
        a: usize,
        b: usize,
        s: Ordering,
        f: Ordering,
    ) -> Result<usize, usize> {
        self.0.compare_exchange(a, b, s, f)
    }
}

/// Vyukov 有界 MPMC 队列(多生产者, 多消费者), 容量为 2 的幂
///
/// 该实现基于无锁算法, 使用槽位序号做状态管理以支持并发推入/弹出. 
pub struct Queue<T> {
    /// 槽位数组, 长度为 capacity(向上取整到 2 的幂)
    buffer: Vec<Slot<T>>,
    /// 用于将序号映射到槽位索引的掩码(mask = capacity - 1)
    mask: usize,
    /// 下一个待写入的位置(生产者索引)
    enqueue_pos: AlignedAtomicUsize,
    /// 下一个待读取的位置(消费者索引)
    dequeue_pos: AlignedAtomicUsize,
    /// 关闭标志(非 0 表示队列已关闭)
    closed: AtomicUsize,
}

unsafe impl<T: Send> Send for Queue<T> {}
unsafe impl<T: Send> Sync for Queue<T> {}

impl<T> Queue<T> {
    /// 创建一个新的环形缓冲区实例, 容量会被调整为不小于指定容量的最小2的幂次方
    ///
    /// # Arguments
    /// * `capacity` - 期望的缓冲区容量, 实际容量会是大于等于该值的最小2的幂次方
    ///
    /// # Examples
    /// ```
    /// use quant1x::runtime::Queue;
    /// let ring_buffer = Queue::<i32>::new(10);  // 实际容量会是16
    /// ```
    ///
    /// # Panics
    /// - 如果内存分配失败会panic
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

    /// 自旋退避策略: 根据重试次数选择不同的退避方式以减少忙等开销
    /// - 早期采用 CPU spin-loop
    /// - 中期 yield 给线程调度器
    /// - 后期短暂 sleep
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

    /// 向队列推入一个元素, 成功返回 Ok, 队列已满返回 Err
    ///
    /// 该操作为无锁设计: 通过比较序号判断槽位是否空闲, 再通过 CAS 争夺写入权限. 
    pub fn push(&self, value: T) -> Result<(), ()> {
        let mut backoff = 0u32;
        loop {
            let pos = self.enqueue_pos.load(Ordering::Relaxed);
            let index = pos & self.mask;
            let slot = &self.buffer[index];
            let seq = slot.seq.load(Ordering::Acquire);
            if seq == pos {
                if self
                    .enqueue_pos
                    .compare_exchange(pos, pos + 1, Ordering::SeqCst, Ordering::Relaxed)
                    .is_ok()
                {
                    // 将值写入槽位(直接写入未初始化的内存), 随后更新序列号为可读状态
                    unsafe {
                        (*slot.data.get()).write(value);
                    }
                    slot.seq.store(pos + 1, Ordering::Release);
                    return Ok(());
                } else {
                    Self::backoff_spin(&mut backoff);
                    continue;
                }
            } else if seq < pos {
                // 当槽位序列号小于期望序号, 说明该槽位尚未被消费者重置, 队列被判定为已满
                return Err(());
            } else {
                Self::backoff_spin(&mut backoff);
                continue;
            }
        }
    }

    /// 从队列弹出一个元素, 成功返回 Ok(value), 当队列关闭且无数据时返回 Err
    ///
    /// 该操作为无锁设计: 通过比较序号判断槽位是否可读, 再通过 CAS 争夺读取权限. 
    pub fn pop(&self) -> Result<T, ()> {
        let mut backoff = 0u32;
        loop {
            let pos = self.dequeue_pos.load(Ordering::Relaxed);
            let index = pos & self.mask;
            let slot = &self.buffer[index];
            let seq = slot.seq.load(Ordering::Acquire);
            if seq == pos + 1 {
                if self
                    .dequeue_pos
                    .compare_exchange(pos, pos + 1, Ordering::SeqCst, Ordering::Relaxed)
                    .is_ok()
                {
                    let val = unsafe { (*slot.data.get()).assume_init_read() };
                    slot.seq.store(pos + self.mask + 1, Ordering::Release);
                    return Ok(val);
                } else {
                    Self::backoff_spin(&mut backoff);
                    continue;
                }
            } else if seq < pos + 1 {
                // 如果序号落后, 说明当前没有可读数据. 
                // 若队列已被关闭, 则返回 Err 表示没有更多元素可读. 
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
        // 关闭后, 生产者应停止推入新数据, 消费者在耗尽现有数据后会收到 Err
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
                        if q2.push(base + i).is_ok() {
                            break;
                        }
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
            consumers.push(std::thread::spawn(move || loop {
                match q2.pop() {
                    Ok(_v) => {
                        c.fetch_add(1, Ordering::Relaxed);
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
        assert_eq!(
            consumed.load(Ordering::Relaxed),
            (NUM_PROD as usize) * (PER as usize)
        );
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
                        if q2.push(base + i).is_ok() {
                            break;
                        }
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
            consumers.push(std::thread::spawn(move || loop {
                match q2.pop() {
                    Ok(_) => {
                        c2.fetch_add(1, Ordering::Relaxed);
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
        assert_eq!(got, TOTAL, "expected all items");
        let secs = elapsed.as_secs_f64();
        let ops = (TOTAL as f64) / secs;
        println!("heavy test ops/sec = {}", ops);
    }
}
