package id128

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

const persistentStateRecordSize = 18

// defaultSyncEvery 默认落盘间隔：快速路径下状态记录先在内存批量缓冲中累积，
// 每攒满 N 条才一次性落盘（带跨进程锁 + fsync）。
// 可用环境变量 QUANT1X_ID128_SYNC_EVERY 覆盖（显式 Option.WithStateSyncEvery 优先级最高）。
// 默认 1000：大多数请求不碰磁盘；进程异常退出最多丢失最近 1000 条进度
// （对应 ID 在重启后可能重复），优雅退出前调用 HLC.Close() 可零丢失。
const defaultSyncEvery = 1000

func defaultSyncEveryValue() uint32 {
	if v := os.Getenv("QUANT1X_ID128_SYNC_EVERY"); v != "" {
		if n, err := strconv.ParseUint(v, 10, 32); err == nil && n > 0 {
			return uint32(n)
		}
	}
	return defaultSyncEvery
}

type persistentState struct {
	Physical int64
	Logical  uint16
	Seq      uint32
}

type stateStore interface {
	Load() (persistentState, bool, error)
	Next(local persistentState, now int64, seed uint16) (persistentState, error)
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

func (s *fileStateStore) Next(local persistentState, now int64, seed uint16) (persistentState, error) {
	if !s.strict {
		// 快速路径：纯内存推进，记录先入批量缓冲；攒满 syncEvery 条才落盘一次。
		// 进程异常退出最多丢失最近 syncEvery-1 条进度（这些 ID 重启后可能重复），
		// 优雅退出前调用 HLC.Close() 可把缓冲完整刷盘、零丢失。
		next := advancePersistentState(local, now, seed)
		if err := s.bufferState(next); err != nil {
			return persistentState{}, err
		}
		return next, nil
	}

	unlock, err := lockProcessFile(s.lockPath)
	if err != nil {
		return persistentState{}, fmt.Errorf("id: 获取进程锁失败: %w", err)
	}
	defer func() {
		unlockErr := unlock()
		if err == nil && unlockErr != nil {
			err = unlockErr
		}
	}()

	// 严格模式：以磁盘最新状态为基准（多写者活跃共享唯一性）。
	base := local
	latest, ok, err := s.loadLatestState()
	if err != nil {
		return persistentState{}, err
	}
	if ok && comparePersistentState(latest, base) > 0 {
		base = latest
	}

	next := advancePersistentState(base, now, seed)
	if err := s.appendState(next); err != nil {
		return persistentState{}, err
	}

	return next, nil
}

func (s *fileStateStore) loadLatestState() (persistentState, bool, error) {
	file, err := os.Open(s.path)
	if err != nil {
		if os.IsNotExist(err) {
			return persistentState{}, false, nil
		}
		return persistentState{}, false, fmt.Errorf("id: 读取状态文件失败: %w", err)
	}
	defer file.Close()

	info, err := file.Stat()
	if err != nil {
		return persistentState{}, false, fmt.Errorf("id: 获取状态文件信息失败: %w", err)
	}

	size := info.Size()
	if size < persistentStateRecordSize {
		return persistentState{}, false, fmt.Errorf("id: 状态文件长度非法: %d", size)
	}

	end := size - (size % persistentStateRecordSize)
	if end == 0 {
		return persistentState{}, false, fmt.Errorf("id: 状态文件长度非法: %d", size)
	}

	var record [persistentStateRecordSize]byte
	for offset := end - persistentStateRecordSize; offset >= 0; offset -= persistentStateRecordSize {
		if _, err := file.ReadAt(record[:], offset); err != nil {
			if errors.Is(err, io.EOF) {
				continue
			}
			return persistentState{}, false, fmt.Errorf("id: 读取状态记录失败: %w", err)
		}

		checksum := binary.BigEndian.Uint32(record[14:18])
		if crc32.ChecksumIEEE(record[:14]) != checksum {
			continue
		}

		state := persistentState{
			Physical: int64(binary.BigEndian.Uint64(record[0:8])),
			Logical:  binary.BigEndian.Uint16(record[8:10]),
			Seq:      binary.BigEndian.Uint32(record[10:14]),
		}
		return state, true, nil
	}

	return persistentState{}, false, fmt.Errorf("id: 状态文件中没有有效记录")
}

// encodeState 把持久化状态编码为 18 字节记录（含 CRC32 校验）。
func encodeState(state persistentState) [persistentStateRecordSize]byte {
	var buf [persistentStateRecordSize]byte
	binary.BigEndian.PutUint64(buf[0:8], uint64(state.Physical))
	binary.BigEndian.PutUint16(buf[8:10], state.Logical)
	binary.BigEndian.PutUint32(buf[10:14], state.Seq)
	binary.BigEndian.PutUint32(buf[14:18], crc32.ChecksumIEEE(buf[:14]))
	return buf
}

// bufferState 把一条状态记录写入批量缓冲，攒满 syncEvery 条时一次性落盘。
// 直接编码进预分配切片，热路径零分配。
func (s *fileStateStore) bufferState(state persistentState) error {
	off := len(s.pending)
	s.pending = s.pending[:off+persistentStateRecordSize]
	binary.BigEndian.PutUint64(s.pending[off:off+8], uint64(state.Physical))
	binary.BigEndian.PutUint16(s.pending[off+8:off+10], state.Logical)
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
		return fmt.Errorf("id: 获取进程锁失败: %w", err)
	}
	defer func() {
		unlockErr := unlock()
		if err == nil && unlockErr != nil {
			err = unlockErr
		}
	}()

	// 目录已就绪时跳过 MkdirAll（避免热路径上的 stat 开销）
	if !s.dirReady {
		if err := os.MkdirAll(filepath.Dir(s.path), 0o755); err != nil {
			return fmt.Errorf("id: 创建状态目录失败: %w", err)
		}
		s.dirReady = true
	}

	file, err := os.OpenFile(s.path, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0o644)
	if err != nil {
		return fmt.Errorf("id: 打开状态文件失败: %w", err)
	}
	defer file.Close()

	if _, err := file.Write(s.pending); err != nil {
		return fmt.Errorf("id: 追加状态记录失败: %w", err)
	}
	if err := file.Sync(); err != nil {
		return fmt.Errorf("id: 刷新状态文件失败: %w", err)
	}

	s.pending = s.pending[:0]
	return nil
}

// Flush 立即把批量缓冲中的记录写入状态文件（带锁 + fsync）。
// 优雅退出前调用，可避免重启后重复最近尚未落盘的 ID。
func (s *fileStateStore) Flush() error {
	return s.flushPending()
}

// appendState 立即追加一条状态记录并落盘，供严格模式使用。
func (s *fileStateStore) appendState(state persistentState) error {
	// 目录已就绪时跳过 MkdirAll（避免热路径上的 stat 开销）
	if !s.dirReady {
		if err := os.MkdirAll(filepath.Dir(s.path), 0o755); err != nil {
			return fmt.Errorf("id: 创建状态目录失败: %w", err)
		}
		s.dirReady = true
	}

	buf := encodeState(state)

	file, err := os.OpenFile(s.path, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0o644)
	if err != nil {
		return fmt.Errorf("id: 打开状态文件失败: %w", err)
	}
	defer file.Close()

	if _, err := file.Write(buf[:]); err != nil {
		return fmt.Errorf("id: 追加状态记录失败: %w", err)
	}

	// TODO: s.syncEvery 当前仅在构造期写入（单线程），若未来支持运行时动态调整，
	// 需改为 atomic 读写以避免 data race。
	syncEvery := s.syncEvery
	if syncEvery == 0 {
		syncEvery = 1
	}
	if atomic.AddUint32(&s.unsynced, 1) >= syncEvery {
		if err := file.Sync(); err != nil {
			return fmt.Errorf("id: 刷新状态文件失败: %w", err)
		}
		atomic.StoreUint32(&s.unsynced, 0)
	}

	return nil
}

func comparePersistentState(left, right persistentState) int {
	if left.Physical < right.Physical {
		return -1
	}
	if left.Physical > right.Physical {
		return 1
	}
	if left.Logical < right.Logical {
		return -1
	}
	if left.Logical > right.Logical {
		return 1
	}
	if left.Seq < right.Seq {
		return -1
	}
	if left.Seq > right.Seq {
		return 1
	}
	return 0
}

// randomUint16 返回进程级随机种子（sync.Once 保证每个进程只生成一次）。
// 熵源不可用时退化为 UnixNano 与 PID 混洗，避免系统熵源阻塞影响后续每次生成。
var (
	randomUint16Once sync.Once
	randomUint16Seed uint16
)

func randomUint16() uint16 {
	randomUint16Once.Do(func() {
		var b [2]byte
		if _, err := crand.Read(b[:]); err == nil {
			randomUint16Seed = binary.BigEndian.Uint16(b[:])
			return
		}

		fallback := uint64(time.Now().UnixNano())
		fallback ^= uint64(os.Getpid()) << 16
		randomUint16Seed = uint16(fallback)
	})
	return randomUint16Seed
}
