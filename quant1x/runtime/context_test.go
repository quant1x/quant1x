// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package runtime

import (
	"testing"
)

func TestContext(t *testing.T) {
	_ = Context()
	CancelContext()
}
