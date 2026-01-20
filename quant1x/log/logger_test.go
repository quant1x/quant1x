package log

import (
	"testing"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/config"
	"gitee.com/quant1x/quant1x/quant1x/runtime"
	"go.uber.org/zap"
)

func TestLogger(t *testing.T) {
	defer runtime.WaitForShutdown(1)
	InitLogger(config.GetLogsPath(), INFO)
	count := 10
	//logger.Fatal("This is fatal")
	for i := 0; i < count; i++ {
		// 输出日志
		logger.Infof("%d: This is an info message, %+v", i, zap.String("user", "Alice"))
		logger.Errorf("%d: This is an error message, %+v", i, zap.Int("code", 500))
		logger.Debugf("This is a debug message, %d", i)
		logger.Warnf("This is a warn message, %+v", zap.Int("code", 200))
		time.Sleep(1 * time.Second)
	}
}
