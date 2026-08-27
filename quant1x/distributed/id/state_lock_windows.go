//go:build windows

package id

import (
	"errors"
	"syscall"
)

var (
	kernel32Proc      = syscall.NewLazyDLL("kernel32.dll")
	openProcessProc   = kernel32Proc.NewProc("OpenProcess")
	waitForSingleProc = kernel32Proc.NewProc("WaitForSingleObject")
	closeHandleProc   = kernel32Proc.NewProc("CloseHandle")
	processQueryInfo  = uintptr(0x0400) // PROCESS_QUERY_LIMITED_INFORMATION
	waitObjectZero    = uintptr(0)
	waitAbandoned     = uintptr(0x00000080)
	waitFailed        = uintptr(0xFFFFFFFF)
)

// processAlive 判断进程是否存活。
func processAlive(pid int) bool {
	if pid <= 0 {
		return false
	}
	r1, _, callErr := openProcessProc.Call(processQueryInfo, 0, uintptr(pid))
	if r1 == 0 {
		// 打不开句柄即视为已退出；
		// ERROR_ACCESS_DENIED 表示进程存在但无权查询，视为存活。
		return errors.Is(callErr, syscall.ERROR_ACCESS_DENIED)
	}
	defer func() { closeHandleProc.Call(r1) }()
	status, _, _ := waitForSingleProc.Call(r1, 0)
	switch status {
	case waitObjectZero, waitAbandoned:
		return false // 句柄 signaled：进程已终止
	case waitFailed:
		return true // 查询失败保守视为存活
	default:
		return true // WAIT_TIMEOUT：仍在运行
	}
}
