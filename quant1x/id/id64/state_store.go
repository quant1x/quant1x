package id64

import (
	crand "crypto/rand"
	"encoding/binary"
	"errors"
	"fmt"
	"hash/crc32"
	"io"
	"os"
	"path/filepath"
	"strconv"
	"sync"
	"sync/atomic"
	"time"
)

// defaultSyncEvery 默认落盘间隔：快速路径下状态记录先在内存批量缓冲中累积，
// 每攒满 N 条才一次性落盘（带跨进程锁 + fsync）。
// 可用环境变量 QUANT1X_ID64_SYNC_EVERY 覆盖（显式 Option.WithStateSyncEvery 优先级最高）。
// 默认 1000：大多数请求不碰磁盘；进程异常退出最多丢失最近 1000 条进度
// （这些 ID 重启后可能重复），优雅退出前调用 HLC.Close() 可零丢失。
const defaultSyncEvery = 1000

func defaultSyncEveryValue() uint32 {
	if v := os.Getenv("QUANT1X_ID64_SYNC_EVERY"); v != "" {
		if n, err := strconv.ParseUint(v, 10, 32); err == nil && n > 0 {
			return uint32(n)
		}
	}
	return defaultSyncEvery
}

// persistentStateRecordSize 状态文件单条记录大小（与 id128 一致）：
// physical(8B) + logical(2B, 恒 0) + seq(4B) + crc32(4B) = 18B。
const persistentStateRecordSize = 18

// persistentState 是持久化高水位状态。
type persistentState struct {
	Physical int64
	Seq      uint32
}

// stateStore 抽象状态存储，支持文件实现与内存实现。
type stateStore interface {
	// Load 返回最近一次持久化状态；文件不存在时 ok=false。
	Load() (persistentState, bool, error)
	// Next 在跨进程锁保护下推进状态。
	Next(local persistentState, now int64, seqBits uint8) (persistentState, error)
}

// fileStateStore 是文件状态存储实现。
// 默认快速路径（strict=false）：构造时恢复一次水位，运行期纯内存推进，
// 状态记录先累积在批量缓冲（pending）中，攒满 syncEvery 条才一次性落盘（带锁 + fsync），
// 热路径零系统调用。适合单写者（含多进程顺序接管 / failover）场景——
// 新写者构造时读到前任写者最近一次落盘的水位，保证跨进程、跨重启不重复。
// 开启严格模式（strict=true）后每次 Next 都读盘取 max，保证多写者活跃共享唯一。
type fileStateStore struct {
	path      string
	lockPath  string
	syncEvery uint32
	unsynced  uint32
	strict    bool
	dirReady  bool
	// pending 是快速路径的批量缓冲：尚未落盘的状态记录。
	// 仅由 HLC 内部锁（h.mu）串行化访问，无需额外同步。
	pending []byte

	// 严格模式句柄缓存（懒打开，首轮发号建立，之后复用）：
	// 把每轮发号的系统调用从 ~8 次（两次 OpenFile/Close + Stat/ReadAt/Write/锁）
	// 降到 5 次（Lock + Stat + ReadAt + Write + Unlock，fsync 每 syncEvery 条一次）。
	// 仅由 HLC 内部锁（h.mu）+ 跨进程锁串行化访问。
	strictLock   *os.File                        // 锁文件句柄
	strictFile   *os.File                        // 状态文件句柄（O_RDWR|O_APPEND）
	strictRecord [persistentStateRecordSize]byte // 复用编码缓冲，避免每轮 make
}

func newFileStateStore(path string) stateStore {
	syncEvery := defaultSyncEveryValue()
	return &fileStateStore{
		path:      path,
		lockPath:  path + ".lock",
		syncEvery: syncEvery,
		// 预分配批量缓冲容量，避免热路径上 append 触发扩容分配
		pending: make([]byte, 0, int(syncEvery)*persistentStateRecordSize),
	}
}

func (s *fileStateStore) Load() (persistentState, bool, error) {
	return s.loadLatestState()
}

func (s *fileStateStore) Next(local persistentState, now int64, seqBits uint8) (persistentState, error) {
	if !s.strict {
		// 快速路径：纯内存推进，记录先入批量缓冲；攒满 syncEvery 条才落盘一次。
		// 进程异常退出最多丢失最近 syncEvery-1 条进度（这些 ID 重启后可能重复），
		// 优雅退出前调用 HLC.Close() 可把缓冲完整刷盘、零丢失。
		next := advancePersistentState(local, now, seqBits)
		if err := s.bufferState(next); err != nil {
			return persistentState{}, err
		}
		return next, nil
	}

	unlock, err := s.lockCached()
	if err != nil {
		return persistentState{}, err
	}
	defer func() { _ = unlock() }()

	// 严格模式：以磁盘最新状态为基准（多写者活跃共享唯一性）。
	f, err := s.stateFileCached()
	if err != nil {
		return persistentState{}, err
	}
	base := local
	latest, ok, err := s.loadLatestFrom(f)
	if err != nil {
		return persistentState{}, err
	}
	if ok && comparePersistentState(latest, base) > 0 {
		base = latest
	}
	next := advancePersistentState(base, now, seqBits)
	if err := s.appendTo(f, next); err != nil {
		return persistentState{}, err
	}
	return next, nil
}

// loadLatestState 从尾部向前读取最近一条有效状态记录：
// 仅 ReadAt 命中最后几条记录，CRC 校验通过即返回，避免全文件扫描。
// 尾部存在坏损记录（断电残写）时截断到最后一个有效记录的末尾。
func (s *fileStateStore) loadLatestState() (persistentState, bool, error) {
	f, err := os.Open(s.path)
	if errors.Is(err, os.ErrNotExist) {
		return persistentState{}, false, nil
	}
	if err != nil {
		return persistentState{}, false, fmt.Errorf("id64: 打开状态文件失败: %w", err)
	}
	defer f.Close()

	info, err := f.Stat()
	if err != nil {
		return persistentState{}, false, fmt.Errorf("id64: 获取状态文件信息失败: %w", err)
	}
	size := info.Size()
	end := size - size%persistentStateRecordSize
	if end == 0 {
		// 不足一条完整记录：视为无状态（下次 append 重建）
		return persistentState{}, false, nil
	}

	var record [persistentStateRecordSize]byte
	for offset := end - persistentStateRecordSize; offset >= 0; offset -= persistentStateRecordSize {
		if _, err := f.ReadAt(record[:], offset); err != nil {
			if errors.Is(err, io.EOF) {
				continue
			}
			return persistentState{}, false, fmt.Errorf("id64: 读取状态记录失败: %w", err)
		}
		st, valid := decodeStateRecord(record[:])
		if !valid {
			continue
		}
		// 尾部存在坏损记录时截断到有效记录末尾
		if size > offset+persistentStateRecordSize {
			if truncateErr := os.Truncate(s.path, offset+persistentStateRecordSize); truncateErr != nil {
				return persistentState{}, false, fmt.Errorf("id64: 截断坏损状态文件失败: %w", truncateErr)
			}
		}
		return st, true, nil
	}
	return persistentState{}, false, nil
}

// bufferState 把一条状态记录写入批量缓冲，攒满 syncEvery 条时一次性落盘。
// 直接编码进预分配切片，热路径零分配。
func (s *fileStateStore) bufferState(state persistentState) error {
	off := len(s.pending)
	s.pending = s.pending[:off+persistentStateRecordSize]
	binary.BigEndian.PutUint64(s.pending[off:off+8], uint64(state.Physical))
	binary.BigEndian.PutUint16(s.pending[off+8:off+10], 0) // logical 恒 0
	binary.BigEndian.PutUint32(s.pending[off+10:off+14], state.Seq)
	binary.BigEndian.PutUint32(s.pending[off+14:off+18], crc32.ChecksumIEEE(s.pending[off:off+14]))
	// 以 syncEvery 为阈值判断（不依赖预分配容量，兼容构造期后调整 syncEvery）
	if len(s.pending) >= int(s.syncEvery)*persistentStateRecordSize {
		if err := s.flushPending(); err != nil {
			return err
		}
	}
	return nil
}

// flushPending 在跨进程锁保护下，把批量缓冲一次性追加到状态文件并 fsync。
func (s *fileStateStore) flushPending() error {
	if len(s.pending) == 0 {
		return nil
	}

	unlock, err := lockProcessFile(s.lockPath)
	if err != nil {
		return err
	}
	defer func() { _ = unlock() }()

	// 目录已就绪时跳过 MkdirAll（避免热路径上的 stat 开销）
	if !s.dirReady {
		dir := filepath.Dir(s.path)
		if err := os.MkdirAll(dir, 0o755); err != nil {
			return fmt.Errorf("id64: 创建状态目录失败: %w", err)
		}
		s.dirReady = true
	}

	f, err := os.OpenFile(s.path, os.O_CREATE|os.O_APPEND|os.O_WRONLY, 0o644)
	if err != nil {
		return fmt.Errorf("id64: 打开状态文件失败: %w", err)
	}
	defer f.Close()

	if _, err := f.Write(s.pending); err != nil {
		return fmt.Errorf("id64: 写入状态文件失败: %w", err)
	}
	if err := f.Sync(); err != nil {
		return fmt.Errorf("id64: 状态文件同步失败: %w", err)
	}

	s.pending = s.pending[:0]
	return nil
}

// Flush 立即把批量缓冲中的记录写入状态文件（带锁 + fsync）。
// 优雅退出前调用，可避免重启后重复最近尚未落盘的 ID。
func (s *fileStateStore) Flush() error {
	return s.flushPending()
}

// Close 释放严格模式缓存的文件句柄（幂等）。
// 通常在 HLC.Close 时调用；句柄未建立或已释放时为空操作。
func (s *fileStateStore) Close() error {
	var firstErr error
	if s.strictFile != nil {
		if err := s.strictFile.Close(); err != nil {
			firstErr = fmt.Errorf("id64: 关闭状态文件失败: %w", err)
		}
		s.strictFile = nil
	}
	if s.strictLock != nil {
		if err := s.strictLock.Close(); err != nil {
			if firstErr == nil {
				firstErr = fmt.Errorf("id64: 关闭锁文件失败: %w", err)
			}
		}
		s.strictLock = nil
	}
	return firstErr
}

// lockCached 获取严格模式的跨进程锁：优先复用缓存句柄。
// 兼容平台（无 flock / LockFileEx）不支持句柄缓存，回退到一次性 lockProcessFile。
func (s *fileStateStore) lockCached() (func() error, error) {
	if !strictLockCacheSupported {
		return lockProcessFile(s.lockPath)
	}
	if s.strictLock == nil {
		f, err := os.OpenFile(s.lockPath, os.O_CREATE|os.O_RDWR, 0o644)
		if err != nil {
			return nil, fmt.Errorf("id64: 打开锁文件失败: %w", err)
		}
		s.strictLock = f
	}
	return lockProcessFileHandle(s.strictLock)
}

// stateFileCached 返回严格模式复用的状态文件句柄（懒打开一次，之后复用）。
// O_APPEND 保证跨进程、跨句柄的追加原子性；O_RDWR 允许 ReadAt 任意偏移读取、
// Truncate 截断坏损尾部。打开失败时保持 nil，下次调用自动重试。
func (s *fileStateStore) stateFileCached() (*os.File, error) {
	if s.strictFile != nil {
		return s.strictFile, nil
	}
	// 目录已就绪时跳过 MkdirAll（避免热路径上的 stat 开销）
	if !s.dirReady {
		if err := os.MkdirAll(filepath.Dir(s.path), 0o755); err != nil {
			return nil, fmt.Errorf("id64: 创建状态目录失败: %w", err)
		}
		s.dirReady = true
	}
	f, err := os.OpenFile(s.path, os.O_CREATE|os.O_RDWR|os.O_APPEND, 0o644)
	if err != nil {
		return nil, fmt.Errorf("id64: 打开状态文件失败: %w", err)
	}
	s.strictFile = f
	return f, nil
}

// loadLatestFrom 从已打开句柄的尾部向前读取最近一条有效状态记录
// （严格模式热路径版本：复用缓存句柄，避免每轮 OpenFile/Close 与 FileInfo 分配）。
// 用 Seek(0, End) 取文件大小而非 Stat（syscall 更轻、无 FileInfo 接口分配）；
// 仅 ReadAt 命中最后几条记录，CRC 校验通过即返回；尾部坏损记录（断电残写）截断。
func (s *fileStateStore) loadLatestFrom(f *os.File) (persistentState, bool, error) {
	size, err := f.Seek(0, io.SeekEnd)
	if err != nil {
		return persistentState{}, false, fmt.Errorf("id64: 获取状态文件大小失败: %w", err)
	}
	end := size - size%persistentStateRecordSize
	if end == 0 {
		// 不足一条完整记录：视为无状态（下次 append 重建）
		return persistentState{}, false, nil
	}

	record := s.strictRecord[:]
	for offset := end - persistentStateRecordSize; offset >= 0; offset -= persistentStateRecordSize {
		if _, err := f.ReadAt(record, offset); err != nil {
			if errors.Is(err, io.EOF) {
				continue
			}
			return persistentState{}, false, fmt.Errorf("id64: 读取状态记录失败: %w", err)
		}
		st, valid := decodeStateRecord(record)
		if !valid {
			continue
		}
		// 尾部存在坏损记录时截断到有效记录末尾
		if size > offset+persistentStateRecordSize {
			if truncateErr := f.Truncate(offset + persistentStateRecordSize); truncateErr != nil {
				return persistentState{}, false, fmt.Errorf("id64: 截断坏损状态文件失败: %w", truncateErr)
			}
		}
		return st, true, nil
	}
	return persistentState{}, false, nil
}

// appendTo 向已打开句柄追加一条状态记录（严格模式热路径版本）。
// 复用 strictRecord 编码缓冲，零分配；每 syncEvery 条 fsync 一次。
func (s *fileStateStore) appendTo(f *os.File, state persistentState) error {
	record := s.strictRecord[:]
	binary.BigEndian.PutUint64(record[0:8], uint64(state.Physical))
	binary.BigEndian.PutUint16(record[8:10], 0) // logical 恒 0
	binary.BigEndian.PutUint32(record[10:14], state.Seq)
	binary.BigEndian.PutUint32(record[14:18], crc32.ChecksumIEEE(record[:14]))

	if _, err := f.Write(record); err != nil {
		return fmt.Errorf("id64: 写入状态文件失败: %w", err)
	}

	count := atomic.AddUint32(&s.unsynced, 1)
	if count >= s.syncEvery {
		if err := f.Sync(); err != nil {
			return fmt.Errorf("id64: 状态文件同步失败: %w", err)
		}
		atomic.StoreUint32(&s.unsynced, 0)
	}
	return nil
}

// decodeStateRecord 解码并校验 18 字节记录。
func decodeStateRecord(record []byte) (persistentState, bool) {
	if len(record) != persistentStateRecordSize {
		return persistentState{}, false
	}
	checksum := binary.BigEndian.Uint32(record[14:18])
	if crc32.ChecksumIEEE(record[:14]) != checksum {
		return persistentState{}, false
	}
	return persistentState{
		Physical: int64(binary.BigEndian.Uint64(record[0:8])),
		Seq:      binary.BigEndian.Uint32(record[10:14]),
	}, true
}

func comparePersistentState(a, b persistentState) int {
	if a.Physical != b.Physical {
		if a.Physical > b.Physical {
			return 1
		}
		return -1
	}
	switch {
	case a.Seq > b.Seq:
		return 1
	case a.Seq < b.Seq:
		return -1
	}
	return 0
}

// randomUint16 返回进程级随机种子（sync.Once 保证每个进程只生成一次）。
// 熵源不可用时退化为 UnixNano 与 PID 混洗（对齐 id128 的退化策略），
// 避免系统熵源阻塞影响后续每次生成。
var (
	randomUint16Once sync.Once
	randomUint16Seed uint16
)

func randomUint16() uint16 {
	randomUint16Once.Do(func() {
		var buf [2]byte
		if _, err := crand.Read(buf[:]); err == nil {
			randomUint16Seed = binary.BigEndian.Uint16(buf[:])
			return
		}
		fallback := uint64(time.Now().UnixNano())
		fallback ^= uint64(os.Getpid()) << 16
		randomUint16Seed = uint16(fallback)
	})
	return randomUint16Seed
}
