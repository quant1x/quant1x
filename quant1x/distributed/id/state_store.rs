//! 文件状态存储 (对应 Go fileStateStore)
//!
//! 状态与跨进程锁均落在同一块共享 mmap 上:
//!   - 偏移 0..63 为双槽 checkpoint (generation + CRC 选最新有效槽);
//!   - 偏移 64..71 为 8 字节锁字 (pid + stamp, CAS 加解锁,
//!     持有者进程死亡或超时由等待方抢占)。
//!
//! 默认快速路径 (strict=false): 构造时恢复一次水位, 运行期纯内存推进,
//! 攒满 syncEvery 条才 checkpoint (msync) 一次, 热路径零系统调用。
//! 开启严格模式 (strict=true) 后每次 Next 都持锁读取共享映射取 max,
//! 保证多写者活跃共享唯一。

use std::fs::File;
use std::io::{Seek, SeekFrom};
use std::path::PathBuf;
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Mutex, MutexGuard};
use std::time::{Duration, SystemTime, UNIX_EPOCH};

use memmap2::MmapMut;

use super::crc32::crc32_ieee;
use super::Error;

/// 默认落盘间隔 (对应 Go defaultSyncEvery)
pub(crate) const DEFAULT_SYNC_EVERY: u32 = 1000;

/// 环境变量: 覆盖默认落盘间隔
pub(crate) const ENV_SYNC_EVERY: &str = "QUANT1X_ID64_SYNC_EVERY";

pub(crate) const LEGACY_RECORD_SIZE: usize = 18;
pub(crate) const PERSISTENT_STATE_RECORD_SIZE: usize = LEGACY_RECORD_SIZE;
pub(crate) const CHECKPOINT_SLOT_SIZE: usize = 32;
pub(crate) const CHECKPOINT_SLOT_COUNT: usize = 2;
/// 64B 双槽区
pub(crate) const CHECKPOINT_AREA_SIZE: usize = CHECKPOINT_SLOT_SIZE * CHECKPOINT_SLOT_COUNT;
/// 槽区 + mmap 锁字 + 预留, 定长不再增长
pub(crate) const STATE_FILE_SIZE: usize = 128;
/// 锁字偏移 (跨进程互斥锁)
pub(crate) const STATE_LOCK_OFFSET: usize = CHECKPOINT_AREA_SIZE;
/// 锁字占用: 高 32 位 pid, 低 32 位秒级时间戳
pub(crate) const LOCK_TAKEOVER_AFTER_SECONDS: u32 = 30;
pub(crate) const LOCK_BACKOFF_MAX_SLEEP_US: u64 = 1024;

/// 持久化高水位状态
#[derive(Clone, Copy, PartialEq, Eq, Debug, Default)]
pub struct PersistentState {
    pub physical: i64,
    pub seq: u32,
}

/// 比较两个持久化状态 (先比物理时间, 再比序号)
pub(crate) fn compare_persistent_state(a: PersistentState, b: PersistentState) -> i32 {
    if a.physical != b.physical {
        return if a.physical > b.physical { 1 } else { -1 };
    }
    if a.seq > b.seq {
        1
    } else if a.seq < b.seq {
        -1
    } else {
        0
    }
}

/// 在给定水位上推进状态 (对应 Go advancePersistentState)
pub(crate) fn advance_persistent_state(state: PersistentState, now: i64, seq_bits: u8) -> PersistentState {
    if now > state.physical {
        return PersistentState { physical: now, seq: 0 };
    }
    let mask = (1u32 << seq_bits) - 1;
    if state.seq >= mask {
        PersistentState {
            physical: state.physical + 1,
            seq: 0,
        }
    } else {
        PersistentState {
            physical: state.physical,
            seq: state.seq + 1,
        }
    }
}

/// 默认落盘间隔 (环境变量 QUANT1X_ID64_SYNC_EVERY 可覆盖)
pub(crate) fn default_sync_every_value() -> u32 {
    if let Ok(value) = std::env::var(ENV_SYNC_EVERY) {
        if let Ok(n) = value.parse::<u32>() {
            if n > 0 {
                return n;
            }
        }
    }
    DEFAULT_SYNC_EVERY
}

/// 文件状态存储
pub struct FileStateStore {
    path: PathBuf,
    sync_every: u32,
    strict: bool,
    inner: Mutex<StoreInner>,
}

struct StoreInner {
    mapped: Option<MmapMut>,
    generation: u64,
    latest: PersistentState,
    unsynced: u32,
}

impl FileStateStore {
    pub(crate) fn new(path: impl Into<PathBuf>, sync_every: u32, strict: bool) -> FileStateStore {
        FileStateStore {
            path: path.into(),
            sync_every,
            strict,
            inner: Mutex::new(StoreInner {
                mapped: None,
                generation: 0,
                latest: PersistentState::default(),
                unsynced: 0,
            }),
        }
    }

    fn lock(&self) -> Result<MutexGuard<'_, StoreInner>, Error> {
        self.inner.lock().map_err(|_| Error::LockPoisoned)
    }

    /// 打开 (必要时创建) 定长状态文件并映射到内存
    fn open_mapped(&self) -> Result<(), Error> {
        let mut inner = self.lock()?;
        if inner.mapped.is_some() {
            return Ok(());
        }
        if let Some(dir) = self.path.parent() {
            if !dir.as_os_str().is_empty() {
                std::fs::create_dir_all(dir)
                    .map_err(|e| Error::StateFile(format!("create dir {:?} failed: {e}", dir)))?;
            }
        }
        let file = File::options()
            .read(true)
            .write(true)
            .create(true)
            .truncate(false)
            .open(&self.path)
            .map_err(|e| Error::StateFile(format!("open state file {:?} failed: {e}", self.path)))?;
        file.set_len(STATE_FILE_SIZE as u64)
            .map_err(|e| Error::StateFile(format!("resize state file {:?} failed: {e}", self.path)))?;
        // 状态文件由本包进程组独占维护 (锁字保证跨进程互斥), 映射期间无外部并发修改风险
        // SAFETY: 映射目标文件在本模块内以定长读写方式维护, 不会出现并发 truncate 导致 SIGBUS
        let map = unsafe { MmapMut::map_mut(&file) }
            .map_err(|e| Error::StateFile(format!("mmap state file {:?} failed: {e}", self.path)))?;
        inner.mapped = Some(map);
        Ok(())
    }

    /// 取锁字原子引用 (偏移 64, 8 字节对齐)
    fn lock_word(&self) -> Result<&'static AtomicU64, Error> {
        let inner = self.lock()?;
        let map = inner
            .mapped
            .as_ref()
            .ok_or_else(|| Error::StateFile("mapping not open".to_string()))?;
        // SAFETY: 锁字位于映射内固定偏移, 8 字节对齐; 映射生命周期与 store 一致,
        // AtomicU64::from_ptr 保证原子访问共享映射内存 (跨进程锁的基础)
        Ok(unsafe { AtomicU64::from_ptr(map.as_ptr().add(STATE_LOCK_OFFSET) as *mut u64) })
    }

    /// 加载最近一次持久化状态; 文件不存在时返回 ok=false
    pub fn load(&self) -> Result<(PersistentState, bool), Error> {
        let mut legacy = PersistentState::default();
        let mut ok = false;
        match std::fs::metadata(&self.path) {
            Ok(info) => {
                if info.len() != STATE_FILE_SIZE as u64 {
                    let (st, found, err) = self.load_latest_state()?;
                    legacy = st;
                    ok = found;
                    let _ = err;
                }
            }
            Err(e) if e.kind() == std::io::ErrorKind::NotFound => {}
            Err(e) => {
                return Err(Error::StateFile(format!(
                    "stat state file {:?} failed: {e}",
                    self.path
                )))
            }
        }
        self.open_mapped()?;
        let (mapped_state, mapped_ok) = self.load_checkpoint();
        if mapped_ok && (!ok || compare_persistent_state(mapped_state, legacy) > 0) {
            legacy = mapped_state;
            ok = true;
        }
        if ok {
            self.lock()?.latest = legacy;
        }
        Ok((legacy, ok))
    }

    /// 扫描双槽 checkpoint, 返回 generation 最大的有效槽
    fn load_checkpoint(&self) -> (PersistentState, bool) {
        // 读槽阶段放在块作用域内: 块结束即释放 store.inner 锁,
        // 避免后续 self.lock() 对同一 Mutex 递归加锁造成死锁
        let (best, valid, best_generation) = {
            let inner = match self.lock() {
                Ok(guard) => guard,
                Err(_) => return (PersistentState::default(), false),
            };
            let Some(map) = inner.mapped.as_ref() else {
                return (PersistentState::default(), false);
            };
            let data = map.as_ptr();
            let mut best = PersistentState::default();
            let mut best_generation: u64 = 0;
            let mut valid = false;
            for slot in 0..CHECKPOINT_SLOT_COUNT {
                let base = slot * CHECKPOINT_SLOT_SIZE;
                // SAFETY: 映射长度为 STATE_FILE_SIZE, base+32 不超过映射范围
                let record =
                    unsafe { std::slice::from_raw_parts(data.add(base), CHECKPOINT_SLOT_SIZE) };
                let mut gen_bytes = [0u8; 8];
                gen_bytes.copy_from_slice(&record[0..8]);
                let generation = u64::from_be_bytes(gen_bytes);
                if generation == 0 {
                    continue;
                }
                let mut crc_bytes = [0u8; 4];
                crc_bytes.copy_from_slice(&record[20..24]);
                let checksum = u32::from_be_bytes(crc_bytes);
                if crc32_ieee(&record[0..20]) != checksum {
                    continue;
                }
                let mut phy_bytes = [0u8; 8];
                phy_bytes.copy_from_slice(&record[8..16]);
                let physical = i64::from_be_bytes(phy_bytes);
                let mut seq_bytes = [0u8; 4];
                seq_bytes.copy_from_slice(&record[16..20]);
                let seq = u32::from_be_bytes(seq_bytes);
                if !valid || generation > best_generation {
                    best_generation = generation;
                    best = PersistentState { physical, seq };
                    valid = true;
                }
            }
            (best, valid, best_generation)
        };
        if valid {
            let mut inner = match self.lock() {
                Ok(guard) => guard,
                Err(_) => return (best, valid),
            };
            inner.generation = best_generation;
            inner.latest = best;
        }
        (best, valid)
    }

    /// 将水位写入指定槽并 (可选) msync
    fn checkpoint(&self, state: PersistentState, flush: bool) -> Result<(), Error> {
        let mut inner = self.lock()?;
        inner.generation += 1;
        let generation = inner.generation;
        let base = ((generation % CHECKPOINT_SLOT_COUNT as u64) as usize) * CHECKPOINT_SLOT_SIZE;
        let mut record = [0u8; CHECKPOINT_SLOT_SIZE];
        record[0..8].copy_from_slice(&generation.to_be_bytes());
        record[8..16].copy_from_slice(&(state.physical as u64).to_be_bytes());
        record[16..20].copy_from_slice(&state.seq.to_be_bytes());
        let checksum = crc32_ieee(&record[0..20]);
        record[20..24].copy_from_slice(&checksum.to_be_bytes());
        let map = inner
            .mapped
            .as_mut()
            .ok_or_else(|| Error::StateFile("mapping not open".to_string()))?;
        // SAFETY: 写共享映射, 映射由本包进程组独占维护 (锁字保证互斥), 写操作与 &self 借用不冲突
        unsafe {
            let dst = map.as_mut_ptr().add(base);
            std::ptr::copy_nonoverlapping(record.as_ptr(), dst, CHECKPOINT_SLOT_SIZE);
        }
        if flush {
            map.flush()
                .map_err(|e| Error::StateFile(format!("msync state file failed: {e}")))?;
        }
        Ok(())
    }

    /// 从尾部向前读取最近一条有效状态记录 (legacy 18 字节格式)
    fn load_latest_state(&self) -> Result<(PersistentState, bool, Option<Error>), Error> {
        let file = match File::open(&self.path) {
            Ok(f) => f,
            Err(e) if e.kind() == std::io::ErrorKind::NotFound => {
                return Ok((PersistentState::default(), false, None))
            }
            Err(e) => {
                return Err(Error::StateFile(format!(
                    "open state file {:?} failed: {e}",
                    self.path
                )))
            }
        };
        let size = file
            .metadata()
            .map_err(|e| Error::StateFile(format!("stat state file failed: {e}")))?
            .len();
        let end = size - size % PERSISTENT_STATE_RECORD_SIZE as u64;
        if end == 0 {
            // 不足一条完整记录: 视为无状态 (下次 append 重建)
            return Ok((PersistentState::default(), false, None));
        }
        let mut offset = end - PERSISTENT_STATE_RECORD_SIZE as u64;
        loop {
            let mut record = [0u8; LEGACY_RECORD_SIZE];
            let mut f = &file;
            f.seek(SeekFrom::Start(offset))
                .map_err(|e| Error::StateFile(format!("seek state file failed: {e}")))?;
            use std::io::Read;
            f.read_exact(&mut record)
                .map_err(|e| Error::StateFile(format!("read state record failed: {e}")))?;
            if let Some(st) = decode_state_record(&record) {
                if size > offset + PERSISTENT_STATE_RECORD_SIZE as u64 {
                    file.set_len(offset + PERSISTENT_STATE_RECORD_SIZE as u64).map_err(|e| {
                        Error::StateFile(format!("truncate state file failed: {e}"))
                    })?;
                }
                return Ok((st, true, None));
            }
            if offset == 0 {
                break;
            }
            offset -= PERSISTENT_STATE_RECORD_SIZE as u64;
        }
        Ok((PersistentState::default(), false, None))
    }

    /// 推进状态 (对应 Go Next)
    pub(crate) fn next(&self, local: PersistentState, now: i64, seq_bits: u8) -> Result<PersistentState, Error> {
        if !self.strict {
            // 快速路径: 纯内存推进; 攒满 syncEvery 条才 checkpoint 一次
            let next = advance_persistent_state(local, now, seq_bits);
            let should_sync = {
                let mut inner = self.lock()?;
                inner.latest = next;
                inner.unsynced += 1;
                inner.unsynced >= self.sync_every
            };
            if should_sync {
                self.checkpoint(next, true)?;
                self.lock()?.unsynced = 0;
            }
            return Ok(next);
        }

        // 严格模式: 以共享映射中的最新状态为基准 (多写者活跃共享唯一性)
        let _guard = self.lock_mapped()?;
        let mut base = local;
        let (latest, ok) = self.load_checkpoint();
        if ok && compare_persistent_state(latest, base) > 0 {
            base = latest;
        }
        let next = advance_persistent_state(base, now, seq_bits);
        let should_sync = {
            let inner = self.lock()?;
            inner.unsynced + 1 >= self.sync_every
        };
        self.checkpoint(next, should_sync)?;
        let mut inner = self.lock()?;
        inner.unsynced += 1;
        if inner.unsynced >= self.sync_every {
            inner.unsynced = 0;
        }
        Ok(next)
    }

    /// 立即把尚未 checkpoint 的水位写入映射并 msync
    pub fn flush(&self) -> Result<(), Error> {
        if self.lock()?.unsynced == 0 {
            return Ok(());
        }
        let _guard = self.lock_mapped()?;
        let latest = self.lock()?.latest;
        self.checkpoint(latest, true)?;
        self.lock()?.unsynced = 0;
        Ok(())
    }

    /// 刷新未落盘水位并释放共享映射 (幂等)
    pub fn close(&self) -> Result<(), Error> {
        let mut first_err = self.flush().err();
        let mut inner = self.lock()?;
        if let Some(map) = inner.mapped.take() {
            if let Err(e) = map.flush() {
                if first_err.is_none() {
                    first_err = Some(Error::StateFile(format!("msync on close failed: {e}")));
                }
            }
            drop(map); // 解除映射
        }
        match first_err {
            Some(e) => Err(e),
            None => Ok(()),
        }
    }

    /// 获取跨进程锁 (返回释放守卫)
    fn lock_mapped(&self) -> Result<LockGuard, Error> {
        self.open_mapped()?;
        let word = self.lock_word()?;
        let self_pid = std::process::id();
        let mine = encode_lock_word(self_pid, lock_stamp_now());
        let mut retries: u32 = 0;
        loop {
            let current = word.load(Ordering::Acquire);
            if current == 0 {
                match word.compare_exchange(0, mine, Ordering::SeqCst, Ordering::Acquire) {
                    Ok(_) => return Ok(LockGuard { word, mine }),
                    Err(_) => continue,
                }
            }
            let (pid, stamp) = decode_lock_word(current);
            if !lock_holder_stale(pid, stamp, self_pid) {
                lock_backoff(&mut retries);
                continue;
            }
            match word.compare_exchange(current, mine, Ordering::SeqCst, Ordering::Acquire) {
                Ok(_) => return Ok(LockGuard { word, mine }),
                Err(_) => {
                    retries = 0;
                    continue;
                }
            }
        }
    }
}

/// 跨进程锁释放守卫: Drop 时归还锁字
struct LockGuard {
    word: &'static AtomicU64,
    mine: u64,
}

impl Drop for LockGuard {
    fn drop(&mut self) {
        // 若期间被抢占 (他人判定本进程已死后接管), CAS 失败即放弃, 不会误清他人的锁
        let _ = self
            .word
            .compare_exchange(self.mine, 0, Ordering::SeqCst, Ordering::Relaxed);
    }
}

/// 编码锁字: 高 32 位 pid, 低 32 位秒级时间戳
fn encode_lock_word(pid: u32, stamp: u32) -> u64 {
    ((pid as u64) << 32) | (stamp as u64)
}

fn decode_lock_word(word: u64) -> (u32, u32) {
    ((word >> 32) as u32, word as u32)
}

/// 当前 Unix 秒 (截断为 u32, 与 Go uint32(time.Now().Unix()) 一致)
fn lock_stamp_now() -> u32 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_secs() as u32)
        .unwrap_or(0)
}

/// 锁持有者是否可被抢占: 同进程不抢占; 进程死亡或锁龄超时可抢占
fn lock_holder_stale(pid: u32, stamp: u32, self_pid: u32) -> bool {
    if pid == self_pid {
        return false;
    }
    if !process_alive(pid) {
        return true;
    }
    let elapsed = lock_stamp_now().wrapping_sub(stamp);
    elapsed >= LOCK_TAKEOVER_AFTER_SECONDS
}

/// 进程是否存活
#[cfg(unix)]
fn process_alive(pid: u32) -> bool {
    if pid == 0 {
        return false;
    }
    // SAFETY: kill(pid, 0) 不发送信号, 仅探测进程存在性, 无内存副作用
    let ret = unsafe { libc::kill(pid as i32, 0) };
    if ret == 0 {
        return true;
    }
    // EPERM: 进程存在但无权限信号, 视为存活; ESRCH: 进程不存在
    std::io::Error::last_os_error().raw_os_error() == Some(libc::EPERM)
}

/// 进程是否存活 (Windows)
#[cfg(windows)]
fn process_alive(pid: u32) -> bool {
    if pid == 0 {
        return false;
    }
    use winapi::shared::minwindef::DWORD;
    use winapi::um::handleapi::CloseHandle;
    use winapi::um::processthreadsapi::OpenProcess;
    use winapi::um::synchapi::WaitForSingleObject;
    use winapi::um::winnt::{PROCESS_QUERY_LIMITED_INFORMATION, HANDLE};

    const WAIT_OBJECT_0: DWORD = 0;
    const WAIT_ABANDONED: DWORD = 0x0000_0080;
    const ERROR_ACCESS_DENIED: DWORD = 5;

    // SAFETY: OpenProcess 仅查询句柄, 无内存副作用
    let handle: HANDLE = unsafe { OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid) };
    if handle.is_null() {
        // 无权限访问通常表示进程存在 (系统进程/其他用户)
        return std::io::Error::last_os_error().raw_os_error() == Some(ERROR_ACCESS_DENIED as i32);
    }
    // SAFETY: WaitForSingleObject 轮询进程退出状态, 0 超时立即返回
    let status: DWORD = unsafe { WaitForSingleObject(handle, 0) };
    // SAFETY: 关闭句柄
    unsafe {
        CloseHandle(handle);
    }
    // 已退出 (WAIT_OBJECT_0 / WAIT_ABANDONED) 视为死亡, 其余 (超时/失败) 保守视为存活
    status != WAIT_OBJECT_0 && status != WAIT_ABANDONED
}

/// 锁等待退避: 先自旋, 再让出, 最后短睡
fn lock_backoff(retries: &mut u32) {
    match *retries {
        0..=3 => {}
        4..=11 => std::thread::yield_now(),
        _ => {
            let mut sleep_us = 1u64 << (*retries - 12);
            if sleep_us > LOCK_BACKOFF_MAX_SLEEP_US {
                sleep_us = LOCK_BACKOFF_MAX_SLEEP_US;
            }
            std::thread::sleep(Duration::from_micros(sleep_us));
        }
    }
    *retries = retries.saturating_add(1);
}

/// 解码并校验 18 字节 legacy 记录
fn decode_state_record(record: &[u8]) -> Option<PersistentState> {
    if record.len() != PERSISTENT_STATE_RECORD_SIZE {
        return None;
    }
    let mut checksum_bytes = [0u8; 4];
    checksum_bytes.copy_from_slice(&record[14..18]);
    let checksum = u32::from_be_bytes(checksum_bytes);
    if crc32_ieee(&record[0..14]) != checksum {
        return None;
    }
    let mut phy_bytes = [0u8; 8];
    phy_bytes.copy_from_slice(&record[0..8]);
    let physical = i64::from_be_bytes(phy_bytes);
    let mut seq_bytes = [0u8; 4];
    seq_bytes.copy_from_slice(&record[10..14]);
    let seq = u32::from_be_bytes(seq_bytes);
    Some(PersistentState { physical, seq })
}

// 仅用于文档说明的编译期断言: 锁字与槽区互不重叠
const _: () = assert!(STATE_LOCK_OFFSET == CHECKPOINT_AREA_SIZE);
const _: () = assert!(STATE_FILE_SIZE > STATE_LOCK_OFFSET + 8);
