package log

import (
	"testing"

	"github.com/quant1x/quant1x/quant1x/config"
	"github.com/quant1x/quant1x/quant1x/runtime"
)

func TestLogger(t *testing.T) {
	defer runtime.WaitForShutdown(1)
	if err := InitLogger(config.GetLogsPath(), INFO); err != nil {
		t.Fatalf("init logger failed: %v", err)
	}
	count := 10
	for i := 0; i < count; i++ {
		// 通过包级入口输出, 验证 caller 与各级别写入
		Infof("%d: This is an info message, user=%s", i, "Alice")
		Errorf("%d: This is an error message, code=%d", i, 500)
		Debugf("This is a debug message, %d", i)
		Warnf("This is a warn message, code=%d", i, 200)
	}
}
