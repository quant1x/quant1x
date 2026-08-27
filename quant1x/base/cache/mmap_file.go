package cache

import (
	"fmt"
	"math"
	"os"
	"path/filepath"
)

// MappedFile is a fixed-size shared memory mapping backed by a file.
type MappedFile struct {
	file *os.File
	data MemObject
}

// OpenMappedFile opens or creates a fixed-size memory-mapped file.
func OpenMappedFile(name string, size int64) (*MappedFile, error) {
	if size <= 0 {
		return nil, fmt.Errorf("invalid mapping size %d", size)
	}
	if size > int64(math.MaxInt) || size > int64(math.MaxUint32) {
		return nil, fmt.Errorf("mapping size %d is too large", size)
	}
	if err := os.MkdirAll(filepath.Dir(name), 0o755); err != nil {
		return nil, fmt.Errorf("create mapping directory: %w", err)
	}
	file, err := os.OpenFile(name, os.O_CREATE|os.O_RDWR, 0o644)
	if err != nil {
		return nil, fmt.Errorf("open mapping file: %w", err)
	}
	if err := file.Truncate(size); err != nil {
		_ = file.Close()
		return nil, fmt.Errorf("resize mapping file: %w", err)
	}
	data, err := mmap(int(size), file)
	if err != nil {
		_ = file.Close()
		return nil, fmt.Errorf("map file: %w", err)
	}
	return &MappedFile{file: file, data: data}, nil
}

// Bytes returns the mapped file contents.
func (m *MappedFile) Bytes() []byte { return m.data.Bytes() }

// Flush writes dirty mapped pages and the file metadata to stable storage.
func (m *MappedFile) Flush() error { return m.data.Flush() }

// Close flushes and releases the mapping and file handle.
func (m *MappedFile) Close() error {
	if m == nil || m.file == nil {
		return nil
	}
	var firstErr error
	if err := m.Flush(); err != nil {
		firstErr = err
	}
	if err := m.data.Unmap(); err != nil && firstErr == nil {
		firstErr = err
	}
	if err := m.file.Close(); err != nil && firstErr == nil {
		firstErr = err
	}
	m.file = nil
	return firstErr
}
