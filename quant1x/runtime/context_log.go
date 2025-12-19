package runtime

// Logger 日志接口，避免循环引用
type Logger interface {
	Debugf(msg string, v ...any)
	Infof(msg string, v ...any)
	Warnf(msg string, v ...any)
	Errorf(msg string, v ...any)
	Fatalf(msg string, v ...any)
}

var logger Logger = nil

// SetLogger 设置全局logger
func SetLogger(logger_ Logger) {
	logger = logger_
}
