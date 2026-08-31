package log

// 包级日志入口, 未初始化时静默丢弃, 避免 nil 指针 panic

func Info(v ...any) {
	if l := getSugar(); l != nil {
		l.Info(v...)
	}
}

func Infof(format string, v ...any) {
	if l := getSugar(); l != nil {
		l.Infof(format, v...)
	}
}

func Debug(v ...any) {
	if l := getSugar(); l != nil {
		l.Debug(v...)
	}
}

func Debugf(format string, v ...any) {
	if l := getSugar(); l != nil {
		l.Debugf(format, v...)
	}
}

func Warn(v ...any) {
	if l := getSugar(); l != nil {
		l.Warn(v...)
	}
}

func Warnf(format string, v ...any) {
	if l := getSugar(); l != nil {
		l.Warnf(format, v...)
	}
}

func Error(v ...any) {
	if l := getSugar(); l != nil {
		l.Error(v...)
	}
}

func Errorf(format string, v ...any) {
	if l := getSugar(); l != nil {
		l.Errorf(format, v...)
	}
}

func Fatal(v ...any) {
	if l := getSugar(); l != nil {
		l.Fatal(v...)
	}
}

func Fatalf(format string, v ...any) {
	if l := getSugar(); l != nil {
		l.Fatalf(format, v...)
	}
}
