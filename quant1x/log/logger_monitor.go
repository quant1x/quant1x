package log

import (
	"fmt"
)

// 等待进程结束信号
func waitForStop() {
	for _, bw := range bws {
		err := bw.Stop()
		if err != nil {
			Errorf("zapcore.BufferedWriteSyncer stop error: %v", err)
		}
	}
	Infof("exit sign")
	fmt.Println("exit")
	if logger != nil {
		_ = logger.Desugar().Sync()
	}
}
