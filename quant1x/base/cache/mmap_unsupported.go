//go:build !windows && !aix && !darwin && !dragonfly && !freebsd && !linux && !netbsd && !openbsd && !solaris
// +build !windows,!aix,!darwin,!dragonfly,!freebsd,!linux,!netbsd,!openbsd,!solaris

package cache

import (
	"fmt"
	"os"
)

func mmap(_ int, _ *os.File) (MemObject, error) {
	return nil, fmt.Errorf("memory mapping is unsupported on this platform")
}
