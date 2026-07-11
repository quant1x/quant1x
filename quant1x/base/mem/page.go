package mem

import (
	"os"
)

const (
	_4KB  = 4096
	_16KB = 16384
)

var (
	pageSize = os.Getpagesize() // 预计算页大小
	pageMask = ^(pageSize - 1)  // 预计算页掩码
)

func safeAlign(size int) int {
	return (size + _4KB - 1) &^ (_4KB - 1)
}

func Align(n int) int {
	return (n + (pageSize - 1)) & pageMask
}
