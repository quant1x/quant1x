// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package base

import "io"

// CloseQuietly 关闭io, 忽略错误
func CloseQuietly(closer io.Closer) {
	if closer != nil {
		_ = closer.Close()
	}
}
