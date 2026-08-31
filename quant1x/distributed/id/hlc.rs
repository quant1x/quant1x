//! 混合逻辑时钟 (HLC, 对应 Go HLC) 与构建器 (对应 Go Option)

use std::path::PathBuf;
use std::sync::Mutex;
use std::time::{SystemTime, UNIX_EPOCH};

use super::id::PAYLOAD_BITS;
use super::state_store::{default_sync_every_value, FileStateStore, PersistentState};
use super::Error;

/// 默认 seq 位数 (22 bit payload - 11 bit worker)
pub(crate) const DEFAULT_SEQ_BITS: u8 = 11;

/// 当前 Unix 毫秒时间
fn unix_millis() -> i64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_millis() as i64)
        .unwrap_or(0)
}

/// 进程级随机种子; 熵源不可用时退化为 UnixNano 与 PID 混洗 (对齐 Go 退化策略)
fn random_u16() -> u16 {
    #[cfg(unix)]
    {
        use std::io::Read;
        if let Ok(mut file) = std::fs::File::open("/dev/urandom") {
            let mut buf = [0u8; 2];
            if file.read_exact(&mut buf).is_ok() {
                return u16::from_be_bytes(buf);
            }
        }
    }
    let nanos = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_nanos() as u64)
        .unwrap_or(0);
    (nanos ^ ((std::process::id() as u64) << 16)) as u16
}

/// seq 掩码
fn seq_mask(seq_bits: u8) -> u32 {
    (1u32 << seq_bits) - 1
}

/// 内部可变状态 (由互斥锁保护)
struct HlcState {
    physical: i64,
    seq: u32,
}

/// 混合逻辑时钟
///
/// 保证在同一进程内生成的 (physical, seq) 严格递增; 时钟回拨时通过递增 seq
/// 或借位 physical 维持单调性. 可选的 `FileStateStore` 提供跨进程/跨重启恢复.
pub struct Hlc {
    now: Box<dyn Fn() -> i64 + Send + Sync>,
    seq_bits: u8,
    store: Option<FileStateStore>,
    state: Mutex<HlcState>,
}

impl Hlc {
    /// 便捷构造: 按节点数推导 seq 位数 (等价于 `HlcBuilder::new().with_node_count(count)?`)
    pub fn new(node_count: u32) -> Result<Hlc, Error> {
        HlcBuilder::new().with_node_count(node_count)?.build()
    }

    /// 推进并返回 (physical, seq), 保证严格递增
    pub fn now(&self) -> Result<(i64, u32), Error> {
        let now = (self.now)();
        let mut state = self.state.lock().map_err(|_| Error::LockPoisoned)?;
        if let Some(store) = &self.store {
            let next = store.next(
                PersistentState {
                    physical: state.physical,
                    seq: state.seq,
                },
                now,
                self.seq_bits,
            )?;
            state.physical = next.physical;
            state.seq = next.seq;
            return Ok((state.physical, state.seq));
        }
        if now > state.physical {
            state.physical = now;
            state.seq = 0;
        } else if state.seq >= seq_mask(self.seq_bits) {
            state.physical += 1;
            state.seq = 0;
        } else {
            state.seq += 1;
        }
        Ok((state.physical, state.seq))
    }

    /// seq 位数
    pub fn seq_bits(&self) -> u8 {
        self.seq_bits
    }

    /// 当前物理时间 (毫秒, 相对 Unix 纪元)
    pub fn timestamp(&self) -> i64 {
        self.state.lock().map(|g| g.physical).unwrap_or(0)
    }

    /// 刷新并释放状态存储 (幂等)
    pub fn close(&self) -> Result<(), Error> {
        let _guard = self.state.lock().map_err(|_| Error::LockPoisoned)?;
        if let Some(store) = &self.store {
            // 即使 Flush 失败也继续释放缓存资源 (mmap/文件句柄)
            return store.close();
        }
        Ok(())
    }
}

/// HLC 构建器 (对应 Go 的 Option 函数式配置)
pub struct HlcBuilder {
    now: Box<dyn Fn() -> i64 + Send + Sync>,
    seed: u16,
    seq_bits: u8,
    sync_every: u32,
    strict: bool,
    store_path: Option<PathBuf>,
}

impl HlcBuilder {
    /// 创建默认构建器
    pub fn new() -> Self {
        HlcBuilder {
            now: Box::new(unix_millis),
            seed: random_u16(),
            seq_bits: DEFAULT_SEQ_BITS,
            sync_every: default_sync_every_value(),
            strict: false,
            store_path: None,
        }
    }

    /// 自定义时钟 (对应 Go WithClock)
    pub fn with_clock<F>(mut self, now: F) -> Self
    where
        F: Fn() -> i64 + Send + Sync + 'static,
    {
        self.now = Box::new(now);
        self
    }

    /// 自定义初始序号种子 (对应 Go WithSeqSeed)
    pub fn with_seq_seed(mut self, seed: u16) -> Self {
        self.seed = seed;
        self
    }

    /// 启用文件状态存储 (对应 Go WithStateFile)
    pub fn with_state_file(mut self, path: impl Into<PathBuf>) -> Self {
        self.store_path = Some(path.into());
        self
    }

    /// 设置落盘间隔 (0 忽略, 对应 Go WithStateSyncEvery)
    pub fn with_state_sync_every(mut self, every: u32) -> Self {
        if every > 0 {
            self.sync_every = every;
        }
        self
    }

    /// 开启严格模式 (对应 Go WithStateStrict)
    pub fn with_state_strict(mut self) -> Self {
        self.strict = true;
        self
    }

    /// 按节点数推导 seq 位数 (对应 Go WithNodeCount)
    pub fn with_node_count(mut self, count: u32) -> Result<Self, Error> {
        let count = if count < 1 { 1 } else { count };
        // bits.Len(uint(count))
        let bit_len = (u32::BITS - count.leading_zeros()) as u8;
        let seq_bits = PAYLOAD_BITS - bit_len;
        if seq_bits < 4 {
            return Err(Error::NodeCountTooLarge(count));
        }
        self.seq_bits = seq_bits;
        Ok(self)
    }

    /// 直接指定 seq 位数, 范围 [4, 21] (对应 Go WithSeqBits)
    pub fn with_seq_bits(mut self, seq_bits: u8) -> Result<Self, Error> {
        if seq_bits < 4 || seq_bits > PAYLOAD_BITS - 1 {
            return Err(Error::InvalidSeqBits(seq_bits));
        }
        self.seq_bits = seq_bits;
        Ok(self)
    }

    /// 构建 HLC (对应 Go NewHLC)
    pub fn build(self) -> Result<Hlc, Error> {
        let store = self
            .store_path
            .map(|path| FileStateStore::new(path, self.sync_every, self.strict));
        let hlc = Hlc {
            now: self.now,
            seq_bits: self.seq_bits,
            store,
            state: Mutex::new(HlcState { physical: 0, seq: 0 }),
        };
        let mut restored: Option<PersistentState> = None;
        if let Some(store) = &hlc.store {
            let (state, ok) = store.load()?;
            if ok {
                restored = Some(state);
            }
        }
        {
            let mut state = hlc.state.lock().map_err(|_| Error::LockPoisoned)?;
            match restored {
                Some(st) => {
                    state.physical = st.physical;
                    state.seq = st.seq;
                }
                None => {
                    state.physical = (hlc.now)();
                    state.seq = (self.seed as u32) & seq_mask(hlc.seq_bits);
                }
            }
        }
        Ok(hlc)
    }
}

impl Default for HlcBuilder {
    fn default() -> Self {
        HlcBuilder::new()
    }
}
