// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package base

import (
	"github.com/petermattis/goid"
)

// GoID 获取协程id
func GoID() int64 {
	return goid.Get()
}
