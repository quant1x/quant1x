package runtime

// Debug go编译tag debug状态
//
//	运行和编译增加 -tags="dev"
func Debug() bool {
	return tagDebug
}

// SetDebug 重置debug状态
func SetDebug(enable bool) {
	tagDebug = enable
}
