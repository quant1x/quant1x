package signal

import (
	"os"
	stdsigal "os/signal"
)

// NotifyForShutdown 指定默认监控信号
func NotifyForShutdown() chan os.Signal {
	//创建监听退出chan
	sigs := make(chan os.Signal)
	//监听指定信号 ctrl+c kill
	stdsigal.Notify(sigs, stopSignals...)

	return sigs
}
