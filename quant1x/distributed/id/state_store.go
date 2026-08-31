package id

import (
	crand "crypto/rand"
	"encoding/binary"
	"errors"
	"fmt"
	"hash/crc32"
	"io"
	"os"
	"strconv"
	"sync"
	"time"

	"github.com/quant1x/quant1x/quant1x/base/cache"
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

const (
	legacyRecordSize          = 18
	persistentStateRecordSize = legacyRecordSize
	checkpointSlotSize        = 32
	checkpointSlotCount       = 2
	checkpointAreaSize        = checkpointSlotSize * checkpointSlotCount // 64B 双槽区
	stateFileSize             = 128                                      // 槽区 + mmap 锁字 + 预留，定长不再增长
)

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
// 状态与跨进程锁均落在同一块共享映射上：
//   - 偏移 0..63 为双槽 checkpoint（generation + CRC 选最新有效槽）；
//   - 偏移 64..71 为 8 字节锁字（pid + stamp，CAS 加解锁，
//     持有者进程死亡或超时由等待方抢占）。
//
// 默认快速路径（strict=false）：构造时恢复一次水位，运行期纯内存推进，
// 攒满 syncEvery 条才 checkpoint（msync）一次，热路径零系统调用。
// 适合单写者（含多进程顺序接管 / failover）场景。
// 开启严格模式（strict=true）后每次 Next 都持锁读取共享映射取 max，
// 保证多写者活跃共享唯一。不再使用文件锁（flock / LockFileEx）与锁文件。
type fileStateStore struct {
	path       string
	syncEvery  uint32
	unsynced   uint32
	strict     bool
	mapped     *cache.MappedFile
	generation uint64
	latest     persistentState
}

func newFileStateStore(path string) stateStore {
	return &fileStateStore{
		path:      path,
		syncEvery: defaultSyncEveryValue(),
	}
}

func (s *fileStateStore) Load() (persistentState, bool, error) {
	var legacy persistentState
	var ok bool
	if info, err := os.Stat(s.path); err == nil && info.Size() != stateFileSize {
		legacy, ok, err = s.loadLatestState()
		if err != nil {
			return persistentState{}, false, err
		}
	} else if err != nil && !errors.Is(err, os.ErrNotExist) {
		return persistentState{}, false, err
	}
	if err := s.openMapped(); err != nil {
		return persistentState{}, false, err
	}
	mappedState, mappedOK := s.loadCheckpoint()
	if mappedOK && (!ok || comparePersistentState(mappedState, legacy) > 0) {
		legacy, ok = mappedState, true
	}
	if ok {
		s.latest = legacy
	}
	return legacy, ok, nil
}

func (s *fileStateStore) openMapped() error {
	if s.mapped != nil {
		return nil
	}
	mapped, err := cache.OpenMappedFile(s.path, stateFileSize)
	if err != nil {
		return fmt.Errorf("distributed/id: 打开 mmap 状态文件失败: %w", err)
	}
	s.mapped = mapped
	return nil
}

func (s *fileStateStore) loadCheckpoint() (persistentState, bool) {
	if s.mapped == nil {
		return persistentState{}, false
	}
	data := s.mapped.Bytes()
	var best persistentState
	var bestGeneration uint64
	valid := false
	for slot := 0; slot < 2; slot++ {
		record := data[slot*checkpointSlotSize : (slot+1)*checkpointSlotSize]
		generation := binary.BigEndian.Uint64(record[0:8])
		if generation == 0 || crc32.ChecksumIEEE(record[:20]) != binary.BigEndian.Uint32(record[20:24]) {
			continue
		}
		if !valid || generation > bestGeneration {
			bestGeneration = generation
			best = persistentState{
				Physical: int64(binary.BigEndian.Uint64(record[8:16])),
				Seq:      binary.BigEndian.Uint32(record[16:20]),
			}
			valid = true
		}
	}
	if valid {
		s.generation = bestGeneration
		s.latest = best
	}
	return best, valid
}

func (s *fileStateStore) checkpoint(state persistentState, flush bool) error {
	if err := s.openMapped(); err != nil {
		return err
	}
	s.generation++
	record := s.mapped.Bytes()[(s.generation%2)*checkpointSlotSize:]
	for i := range record[:checkpointSlotSize] {
		record[i] = 0
	}
	binary.BigEndian.PutUint64(record[0:8], s.generation)
	binary.BigEndian.PutUint64(record[8:16], uint64(state.Physical))
	binary.BigEndian.PutUint32(record[16:20], state.Seq)
	binary.BigEndian.PutUint32(record[20:24], crc32.ChecksumIEEE(record[:20]))
	// 写入成功后同步内存水位：否则 Flush()/Close() 会用旧水位的 latest 覆盖
	// 刚写入的新 checkpoint，造成水位回退（严格模式重启可能重复 ID）。
	// 该缺陷在 Rust 版同样存在，已按 Python（Spec 锚点）的修正方式统一处理。
	s.latest = state
	if flush {
		return s.mapped.Flush()
	}
	return nil
}

func (s *fileStateStore) Next(local persistentState, now int64, seqBits uint8) (persistentState, error) {
	if !s.strict {
		// 快速路径：纯内存推进；攒满 syncEvery 条才 checkpoint 一次。
		// 进程异常退出最多丢失最近 syncEvery-1 条进度（这些 ID 重启后可能重复），
		// 优雅退出前调用 HLC.Close() 可把缓冲完整刷盘、零丢失。
		next := advancePersistentState(local, now, seqBits)
		s.latest = next
		s.unsynced++
		if s.unsynced >= s.syncEvery {
			if err := s.checkpoint(next, true); err != nil {
				return persistentState{}, err
			}
			s.unsynced = 0
		}
		return next, nil
	}

	unlock, err := s.lockMapped()
	if err != nil {
		return persistentState{}, err
	}
	defer unlock()

	// 严格模式：以共享映射中的最新状态为基准（多写者活跃共享唯一性）。
	base := local
	latest, ok := s.loadCheckpoint()
	if ok && comparePersistentState(latest, base) > 0 {
		base = latest
	}
	next := advancePersistentState(base, now, seqBits)
	if err := s.checkpoint(next, s.unsynced+1 >= s.syncEvery); err != nil {
		return persistentState{}, err
	}
	s.unsynced++
	if s.unsynced >= s.syncEvery {
		s.unsynced = 0
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
		return persistentState{}, false, fmt.Errorf("distributed/id: 打开状态文件失败: %w", err)
	}
	defer f.Close()

	info, err := f.Stat()
	if err != nil {
		return persistentState{}, false, fmt.Errorf("distributed/id: 获取状态文件信息失败: %w", err)
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
			return persistentState{}, false, fmt.Errorf("distributed/id: 读取状态记录失败: %w", err)
		}
		st, valid := decodeStateRecord(record[:])
		if !valid {
			continue
		}
		// 尾部存在坏损记录时截断到有效记录末尾
		if size > offset+persistentStateRecordSize {
			if truncateErr := os.Truncate(s.path, offset+persistentStateRecordSize); truncateErr != nil {
				return persistentState{}, false, fmt.Errorf("distributed/id: 截断坏损状态文件失败: %w", truncateErr)
			}
		}
		return st, true, nil
	}
	return persistentState{}, false, nil
}

// Flush 立即把尚未 checkpoint 的水位写入映射并 msync。
// 优雅退出前调用，可避免重启后重复最近尚未落盘的 ID。
// 在 mmap 锁字保护下写入：Close 刷盘可能与另一进程严格模式的 checkpoint 并发，
// 无锁并发写同一 slot 会破坏双槽冗余（极端情况下两槽 CRC 同时失效）。
func (s *fileStateStore) Flush() error {
	if s.unsynced == 0 {
		return nil
	}
	unlock, err := s.lockMapped()
	if err != nil {
		return err
	}
	defer unlock()
	if err := s.checkpoint(s.latest, true); err != nil {
		return err
	}
	s.unsynced = 0
	return nil
}

// Close 刷新未落盘的水位并释放共享映射（幂等）。
// 通常在 HLC.Close 时调用；映射未打开时仅跳过释放。
func (s *fileStateStore) Close() error {
	var firstErr error
	if err := s.Flush(); err != nil {
		firstErr = err
	}
	if s.mapped != nil {
		if err := s.mapped.Close(); err != nil && firstErr == nil {
			firstErr = err
		}
		s.mapped = nil
	}
	return firstErr
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
