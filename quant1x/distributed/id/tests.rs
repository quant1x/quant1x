//! 对齐 Go id_test.go 的语义测试

use std::collections::HashSet;
use std::sync::atomic::{AtomicBool, AtomicI64, Ordering};
use std::sync::{mpsc, Arc};
use std::time::Duration;

use super::*;

fn fixed_clock(millis: i64) -> impl Fn() -> i64 + Send + Sync {
    move || millis
}

#[test]
fn test_default_generator() {
    let hlc = Arc::new(
        HlcBuilder::new()
            .with_clock(fixed_clock(EPOCH_MS))
            .build()
            .unwrap(),
    );
    let generator = Generator::new(1, hlc.clone()).unwrap();
    let first = generator.next().unwrap();
    let second = generator.next().unwrap();
    assert!(second > first, "IDs are not increasing: {first} -> {second}");
    assert_eq!(first.node_id(generator.worker_bits()), 1);
}

#[test]
fn test_hlc_rollback_monotonic() {
    let now = Arc::new(AtomicI64::new(EPOCH_MS));
    let n = now.clone();
    let hlc = HlcBuilder::new()
        .with_clock(move || n.load(Ordering::SeqCst))
        .with_seq_seed(9)
        .build()
        .unwrap();
    let (first, first_seq) = hlc.now().unwrap();
    now.store(EPOCH_MS - 1, Ordering::SeqCst);
    let (second, second_seq) = hlc.now().unwrap();
    assert!(
        second > first || (second == first && second_seq > first_seq),
        "rollback broke monotonicity: ({first},{first_seq}) -> ({second},{second_seq})"
    );
}

#[test]
fn test_node_count_derivation() {
    let cases = [
        (1024u32, 11u8, 11u8),
        (5000, 13, 9),
        (3, 2, 20),
        (131072, 18, 4),
    ];
    for (count, worker_bits, seq_bits) in cases {
        let hlc = Arc::new(
            HlcBuilder::new()
                .with_node_count(count)
                .unwrap()
                .build()
                .unwrap(),
        );
        let generator = Generator::new(0, hlc.clone()).unwrap();
        assert_eq!(generator.worker_bits(), worker_bits, "count={count}");
        assert_eq!(hlc.seq_bits(), seq_bits, "count={count}");
    }
}

#[test]
fn test_id_fields_and_encoding() {
    let hlc = Arc::new(
        HlcBuilder::new()
            .with_clock(fixed_clock(EPOCH_MS + 123))
            .build()
            .unwrap(),
    );
    let generator = Generator::new(7, hlc.clone()).unwrap();
    let id = generator.next().unwrap();
    assert_eq!(id.physical(), 123);
    assert_eq!(id.node_id(generator.worker_bits()), 7);
    assert_eq!(Id::from_bytes(id.bytes()), id);
    assert_eq!(id.to_string().len(), 11);
    assert_eq!(Id::parse(&id.to_string()).unwrap(), id);
}

#[test]
fn test_concurrent_unique() {
    let hlc = Arc::new(
        HlcBuilder::new()
            .with_clock(fixed_clock(EPOCH_MS))
            .build()
            .unwrap(),
    );
    let generator = Arc::new(Generator::new(1, hlc).unwrap());
    const COUNT: usize = 10000;
    let (tx, rx) = mpsc::channel();
    let mut handles = Vec::new();
    for _ in 0..8 {
        let tx = tx.clone();
        let generator = generator.clone();
        handles.push(std::thread::spawn(move || {
            for _ in 0..COUNT / 8 {
                tx.send(generator.next().unwrap()).unwrap();
            }
        }));
    }
    drop(tx);
    let mut seen = HashSet::new();
    while let Ok(id) = rx.recv() {
        assert!(seen.insert(id), "duplicate ID: {id}");
    }
    for handle in handles {
        handle.join().unwrap();
    }
    assert_eq!(seen.len(), COUNT);
}

#[test]
fn test_state_file_across_restart() {
    let dir = tempfile::tempdir().unwrap();
    let path = dir.path().join("state.bin");
    let now = EPOCH_MS;
    let hlc1 = HlcBuilder::new()
        .with_clock(fixed_clock(now))
        .with_state_file(path.clone())
        .with_seq_seed(9)
        .build()
        .unwrap();
    let (first, first_seq) = hlc1.now().unwrap();
    hlc1.close().unwrap();
    let hlc2 = HlcBuilder::new()
        .with_clock(fixed_clock(now))
        .with_state_file(path.clone())
        .with_seq_seed(9)
        .build()
        .unwrap();
    let (second, second_seq) = hlc2.now().unwrap();
    hlc2.close().unwrap();
    assert!(
        second > first || (second == first && second_seq > first_seq),
        "restart broke monotonicity: ({first},{first_seq}) -> ({second},{second_seq})"
    );
}

#[test]
fn test_queue_uses_vyukov_ring_buffer() {
    let queue = Queue::new(3).unwrap();
    assert_eq!(queue.cap(), 4);
    let value = Id(42);
    queue.try_push(value).unwrap();
    assert_eq!(queue.try_pop().unwrap(), value);
    queue.close();
    assert_eq!(queue.try_pop(), Err(Error::Closed));
}

// Serve 生产 + TryPop 消费: 取出的 ID 必须全局严格递增且唯一
#[test]
fn test_generator_serve_feeds_queue() {
    let hlc = Arc::new(
        HlcBuilder::new()
            .with_clock(fixed_clock(EPOCH_MS))
            .build()
            .unwrap(),
    );
    let generator = Generator::new(1, hlc).unwrap();
    let queue = Arc::new(Queue::new(1024).unwrap());
    let cancel = Arc::new(AtomicBool::new(false));
    let q = queue.clone();
    let c = cancel.clone();
    let handle = std::thread::spawn(move || generator.serve(&q, &c));

    const COUNT: usize = 4096;
    let mut previous = Id(0);
    let mut index = 0usize;
    while index < COUNT {
        match queue.try_pop() {
            Ok(id) => {
                if index > 0 {
                    assert!(id > previous, "IDs are not increasing: {previous} -> {id}");
                }
                previous = id;
                index += 1;
            }
            Err(Error::QueueEmpty) => std::thread::yield_now(),
            Err(e) => panic!("TryPop at {index}: {e}"),
        }
    }
    cancel.store(true, Ordering::SeqCst);
    match handle.join().unwrap() {
        Err(Error::Canceled) => {}
        other => panic!("Serve() error = {other:?}, want Canceled"),
    }
}

// 关闭队列后 Serve 必须立即停止且不再发号
#[test]
fn test_generator_serve_stops_on_closed_queue() {
    let hlc = Arc::new(
        HlcBuilder::new()
            .with_clock(fixed_clock(EPOCH_MS))
            .build()
            .unwrap(),
    );
    let generator = Generator::new(1, hlc).unwrap();
    let queue = Queue::new(4).unwrap();
    queue.close();
    let cancel = AtomicBool::new(false);
    assert!(
        generator.serve(&queue, &cancel).is_ok(),
        "Serve() on closed queue must return Ok"
    );
}

// 取消后存量 ID 允许继续消费 (graceful drain), 耗尽后返回 ErrClosed
#[test]
fn test_generator_serve_drain_after_cancel() {
    let hlc = Arc::new(
        HlcBuilder::new()
            .with_clock(fixed_clock(EPOCH_MS))
            .build()
            .unwrap(),
    );
    let generator = Generator::new(1, hlc).unwrap();
    let queue = Arc::new(Queue::new(8).unwrap());
    let cancel = Arc::new(AtomicBool::new(false));
    let q = queue.clone();
    let c = cancel.clone();
    let handle = std::thread::spawn(move || generator.serve(&q, &c));

    // 等待生产者至少填入一个 ID
    while queue.len() == 0 {
        std::thread::sleep(Duration::from_millis(1));
    }
    cancel.store(true, Ordering::SeqCst);
    match handle.join().unwrap() {
        Err(Error::Canceled) => {}
        other => panic!("Serve() error = {other:?}, want Canceled"),
    }

    // 关闭队列进入只读排空
    queue.close();
    let mut last = Id(0);
    loop {
        match queue.try_pop() {
            Ok(id) => {
                assert!(id > last, "invalid drained ID {id}");
                last = id;
            }
            Err(Error::Closed) => break,
            Err(e) => panic!("final TryPop() error = {e}, want Closed"),
        }
    }
}
