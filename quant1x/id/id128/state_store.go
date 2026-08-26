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
	"sync/atomic"
	"time"
)

const persistentStateRecordSize = 18

type persistentState struct {
	Physical int64
	Logical  uint16
	Seq      uint32
}

type stateStore interface {
	Load() (persistentState, bool, error)
	Next(local persistentState, now int64, seed uint16) (persistentState, error)
}

type fileStateStore struct {
	path      string
	lockPath  string
	syncEvery uint32
	unsynced  uint32
}

func newFileStateStore(path string) stateStore {
	return &fileStateStore{
		path:      path,
		lockPath:  path + ".lock",
		syncEvery: 1,
	}
}

func (s *fileStateStore) Load() (persistentState, bool, error) {
	return s.loadLatestState()
}

func (s *fileStateStore) Next(local persistentState, now int64, seed uint16) (state persistentState, err error) {
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

	latest, ok, err := s.loadLatestState()
	if err != nil {
		return persistentState{}, err
	}

	base := local
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

func (s *fileStateStore) appendState(state persistentState) error {
	if err := os.MkdirAll(filepath.Dir(s.path), 0o755); err != nil {
		return fmt.Errorf("id: 创建状态目录失败: %w", err)
	}

	var buf [persistentStateRecordSize]byte
	binary.BigEndian.PutUint64(buf[0:8], uint64(state.Physical))
	binary.BigEndian.PutUint16(buf[8:10], state.Logical)
	binary.BigEndian.PutUint32(buf[10:14], state.Seq)
	binary.BigEndian.PutUint32(buf[14:18], crc32.ChecksumIEEE(buf[:14]))

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

func randomUint16() uint16 {
	var b [2]byte
	if _, err := crand.Read(b[:]); err == nil {
		return binary.BigEndian.Uint16(b[:])
	}

	fallback := uint64(time.Now().UnixNano())
	fallback ^= uint64(os.Getpid()) << 16
	return uint16(fallback)
}
