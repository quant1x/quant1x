package signal

import (
	"os"
	"os/signal"
)

// NotifyForShutdown 指定默认监控信号
func NotifyForShutdown() chan os.Signal {
	//创建监听退出chan（使用缓冲以便安全传递给 signal.Notify）
	sigs := make(chan os.Signal, 1)
	//监听指定信号 ctrl+c kill
	signal.Notify(sigs, stopSignals...)

	return sigs
}
