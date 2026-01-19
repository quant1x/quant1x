package runtime

import (
	"context"
	"sync"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/std/signal"
)

var (
	globalOnce      sync.Once
	globalContext   context.Context    = nil
	globalCancel    context.CancelFunc = nil
	globalWaitGroup sync.WaitGroup
)

// initContext 初始化全局上下文并启动信号监听goroutine
// 该函数会创建全局可取消的context，并监听系统中断信号
// 当收到中断信号时，会触发优雅关闭流程
func initContext() {
	globalContext, globalCancel = context.WithCancel(context.Background())
	// 启动goroutine监听退出信号
	go func() {
		interrupt := signal.NotifyForShutdown()
		<-interrupt
		GracefulShutdown()
	}()
}

// Context 获取全局顶层context
func Context() context.Context {
	globalOnce.Do(initContext)
	return globalContext
}

// CancelContext 取消全局context，通知所有协程退出
func CancelContext() {
	globalOnce.Do(initContext)
	if globalCancel != nil {
		globalCancel()
	}
}

// GetContextWithCancel 返回一个可取消的上下文和对应的取消函数
// 该函数会确保全局上下文只初始化一次，并在返回前增加全局等待组的计数
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
		<-ctx.Done()
		if logger != nil {
			logger.Debugf("runtime/context: stopping %s", name)
		}
		// 执行回调
		cb()
		if logger != nil {
			logger.Debugf("runtime/context: %s stopped", name)
		}
		// cancel 子context
		cancel()
		if logger != nil {
			logger.Debugf("runtime/context: %s finished", name)
		}
		globalWaitGroup.Done()
	}()
	return ctx
}

// GracefulShutdown 优雅关闭应用程序，等待所有hook完成并退出
func GracefulShutdown() {
	CancelContext()
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
			//logger.Infof("application shutdown...")
			break
		case sig := <-interrupt:
			//logger.Infof("interrupt: %s", sig.String())
			_ = sig
			break
		}
	}
	GracefulShutdown()
}
