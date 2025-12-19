// mmap_windows.go
//go:build windows
// +build windows

package cache

import (
	"os"
	"unsafe"

	"golang.org/x/sys/windows"
)

type windowsMmap struct {
	addr        uintptr
	size        uint32
	mapHandle   windows.Handle
	fileHandle  windows.Handle
	inheritFile bool
}

func mmap(size int, f *os.File) (MemObject, error) {
	hFile := windows.Handle(f.Fd())

	protect := uint32(windows.PAGE_READWRITE)
	access := uint32(windows.FILE_MAP_WRITE)

	maxSize := uint32(size)
	hMap, err := windows.CreateFileMapping(
		hFile,
		nil,
		protect,
		0,
		maxSize,
		nil,
	)
	if err != nil {
		return nil, err
	}

	addr, err := windows.MapViewOfFile(
		hMap,
		access,
		0,
		0,
		uintptr(size),
	)
	if err != nil {
		windows.CloseHandle(hMap)
		return nil, err
	}

	return &windowsMmap{
		addr:        addr,
		size:        uint32(size),
		mapHandle:   hMap,
		fileHandle:  hFile,
		inheritFile: true,
	}, nil
}

func (m *windowsMmap) Bytes() []byte {
	return unsafe.Slice((*byte)(unsafe.Pointer(m.addr)), m.size)
}

func (m *windowsMmap) Flush() error {
	if err := windows.FlushViewOfFile(m.addr, uintptr(m.size)); err != nil {
		return err
	}
	if m.fileHandle != windows.InvalidHandle {
		return windows.FlushFileBuffers(m.fileHandle)
	}
	return nil
}

func (m *windowsMmap) Unmap() error {
	if m.addr == 0 {
		return nil
	}

	if err := windows.UnmapViewOfFile(m.addr); err != nil {
		return err
	}
	if err := windows.CloseHandle(m.mapHandle); err != nil {
		return err
	}
	if !m.inheritFile {
		return windows.CloseHandle(m.fileHandle)
	}
	return nil
}
