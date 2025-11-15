//go:build !windows

package signal

import (
	"os"
	"syscall"
)

var stopSignals = []os.Signal{syscall.SIGHUP, syscall.SIGINT, syscall.SIGTERM, syscall.SIGQUIT, syscall.SIGSTOP}
