package runtime

import (
	"fmt"
	"runtime/debug"
)

// Deprecated: 废弃 [wangfeng on 2024/2/27 07:52]
func sprintf(v ...any) string {
	n := len(v)
	switch n {
	case 0:
		// 无参数返回长度0的字符串
		return ""
	default:
		// 如果第1个元素是string, 则按照fmt.Sprintf来处理, 否则, 统一fmt.Sprint
		// 这样做的目的为了捕获异常时, 可以允许关注特别信息
		switch tv := v[0].(type) {
		case string:
			return fmt.Sprintf(tv, v[1:]...)
		default:
			return fmt.Sprint(v...)
		}
	}
}

// 捕获异常, 是否忽略异常
func catchException(err any, ignore bool, format string, a ...any) {
	if err != nil {
		//warning := sprintf(v...)
		warning := fmt.Sprintf(format, a...)
		stack := string(debug.Stack())
		loggerFunc := logger.Fatalf
		if ignore {
			loggerFunc = logger.Errorf
		} else {
			fmt.Printf("\nerr=%v, stack=%s\n", err, stack)
		}
		loggerFunc("%s exception: warning=%s, error=%+v, stack=%s", ApplicationName(), warning, err, stack)
	}
}

// CatchPanic 捕获panic
func CatchPanic(format string, a ...any) {
	err := recover()
	catchException(err, false, format, a...)
}

// IgnorePanic 通用捕获panic, 忽略异常, 继续执行
func IgnorePanic(format string, a ...any) {
	err := recover()
	catchException(err, true, format, a...)
}

// IgnorePanicAndNoLog 忽略panic且不记录日志
func IgnorePanicAndNoLog() {
	_ = recover()
}
