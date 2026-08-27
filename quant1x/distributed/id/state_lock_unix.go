//go:build unix

package id

import "syscall"

// processAlive 判断进程是否存活。
// kill(pid, 0) 只做权限与存在性检查、不发送信号：
// ESRCH 表示进程不存在，EPERM 表示存在但无权发信号（仍存活）。
func processAlive(pid int) bool {
	if pid <= 0 {
		return false
	}
	err := syscall.Kill(pid, 0)
	return err == nil || err == syscall.EPERM
}
