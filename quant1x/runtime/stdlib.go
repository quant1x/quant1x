package runtime

import (
	"reflect"
	"runtime"
	_ "unsafe"
)

const (
	invalidProgramCounters = 0
	invalidFunctionName    = "<invalid runtimeFunctionForPC>"
	unknownFilename        = "<unknown file>"
	unknownLine            = -1
)

// Caller 获取正在运行的函数名, 文件名以及行号
func Caller() (function, file string, line int) {
	pcs := make([]uintptr, 1)
	runtime.Callers(2, pcs)
	pc := pcs[0]
	return FuncInfo(pc)
}

// 获取pc
func programCounters(v any) uintptr {
	value := reflect.ValueOf(v)
	var pc uintptr
	if value.Kind() == reflect.Func {
		pc = value.Pointer()
	} else if value.Kind() == reflect.Uintptr {
		pc = v.(uintptr)
	} else {
		pc = invalidProgramCounters
	}
	return pc
}

// FuncInfo 获取函数信息
//
//	v 接收func类型和uintptr
func FuncInfo(v any) (nameOfFunction, filename string, lineNumber int) {
	pc := programCounters(v)
	if pc == invalidProgramCounters {
		return invalidFunctionName, unknownFilename, unknownLine
	}
	f := runtime.FuncForPC(pc)
	if f == nil {
		return invalidFunctionName, unknownFilename, unknownLine
	}
	nameOfFunction = f.Name()
	filename, lineNumber = f.FileLine(pc)
	return
}

// FuncName 获取函数名
func FuncName(v any) (nameOfFunction string) {
	pc := programCounters(v)
	if pc == invalidProgramCounters {
		return invalidFunctionName
	}
	f := runtime.FuncForPC(pc)
	if f == nil {
		return invalidFunctionName
	}
	return f.Name()
}
