package log

import (
	"fmt"

	"go.uber.org/zap/zapcore"
)

// waitForStop 等待进程结束信号, 刷新并关闭日志
func waitForStop() {
	// 先输出退出日志, 再关闭缓冲写入器, 避免退出日志丢失
	Infof("exit sign")
	mu.Lock()
	syncers := append([]*zapcore.BufferedWriteSyncer(nil), bws...)
	mu.Unlock()
	for _, bw := range syncers {
		if err := bw.Stop(); err != nil {
			Errorf("zapcore.BufferedWriteSyncer stop error: %v", err)
		}
	}
	if l := getSugar(); l != nil {
		_ = l.Desugar().Sync()
	}
	fmt.Println("exit")
}
