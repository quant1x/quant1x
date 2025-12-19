// mmap_posix.go (Linux/MacOS实现)
//go:build !windows
// +build !windows

package cache

import (
	"os"
	"unsafe"

	"golang.org/x/sys/unix"
)

type posixMmap struct {
	data []byte
	file *os.File
}

func mmap(size int, f *os.File) (MemObject, error) {
	data, err := unix.Mmap(int(f.Fd()), 0, size,
		unix.PROT_READ|unix.PROT_WRITE, unix.MAP_SHARED)
	if err != nil {
		return nil, err
	}
	return &posixMmap{
		data: data,
		file: f,
	}, nil
}

func (m *posixMmap) Flush() error {
	_, _, errno := unix.Syscall(
		unix.SYS_MSYNC,
		uintptr(unsafe.Pointer(&m.data[0])),
		uintptr(len(m.data)),
		unix.MS_SYNC,
	)
	if errno != 0 {
		return errno
	}
	return nil
}

func (m *posixMmap) Unmap() error {
	return unix.Munmap(m.data)
}
