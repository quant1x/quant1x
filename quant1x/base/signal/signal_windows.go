// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//go:build windows

package signal

import (
	"os"
	"syscall"
)

var stopSignals = []os.Signal{syscall.SIGHUP, syscall.SIGINT, syscall.SIGTERM, syscall.SIGQUIT}
