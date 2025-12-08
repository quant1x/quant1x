package log

import (
	"testing"

	"gitee.com/quant1x/quant1x/quant1x/runtime"
)

func TestLogger(t *testing.T) {
	defer runtime.WaitForShutdown(1)
	Info("test")
}
