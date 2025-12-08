package runtime

import (
	"context"
	"sync"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/runtime/signal"
)

var (
	globalOnce      sync.Once
	globalContext   context.Context    = nil
	globalCancel    context.CancelFunc = nil
	globalWaitGroup sync.WaitGroup
)

func initContext() {
	globalContext, globalCancel = context.WithCancel(context.Background())
}

// Context 获取全局顶层context
func Context() context.Context {
	globalOnce.Do(initContext)
	return globalContext
}

// Shutdown 关闭应用程序, 通知所有协程退出
func Shutdown() {
	globalOnce.Do(initContext)
	if globalCancel != nil {
		globalCancel()
	}
}

func GetContextWithCancel() (context.Context, context.CancelFunc) {
	globalOnce.Do(initContext)
	ctx, cancel := context.WithCancel(globalContext)
	globalWaitGroup.Add(1)
	return ctx, cancel
}

// RegisterHook 注册系统退出的hook
func RegisterHook(name string, cb func()) context.Context {
	ctx, cancel := GetContextWithCancel()
	go func() {
		select {
		case <-ctx.Done():
			// 收到退出信号
			//logger.Debug("x/context: stopping %s", name)
			// 执行回调
			cb()
			//logger.Debug("x/context: %s stopped", name)
			// cancel 子context
			cancel()
			//logger.Debug("x/context: %s finished", name)
			globalWaitGroup.Done()
			return
		}
	}()
	_ = name
	return ctx
}

// 执行应用退出前的清理工作
func applicationShutdown() {
	globalCancel()
	globalWaitGroup.Wait()
}

// WaitForShutdown 阻塞等待关闭信号
//
//	如果传入d, 视为等待d秒结束
//	如果没有传值, 则默认为等待信号
func WaitForShutdown(d ...int) {
	globalOnce.Do(initContext)
	interrupt := signal.NotifyForShutdown()
	delay := 0
	if len(d) > 0 {
		delay = d[0]
	}
	if delay > 0 {
		time.Sleep(time.Second * time.Duration(delay))
	} else {
		select {
		case <-globalContext.Done():
			//logger.Info("application shutdown...")
			break
		case sig := <-interrupt:
			//logger.Info("interrupt: %s", sig.String())
			_ = sig
			break
		}
	}
	applicationShutdown()
}
