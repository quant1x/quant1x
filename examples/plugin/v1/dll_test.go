package main

import (
	"fmt"
	"syscall"
	"testing"
)

func TestV1Dll(t *testing.T) {
	// 加载 DLL, 测试dll是否能正常工作
	dllPath := "../../"
	dllName := "mydll"
	dll := syscall.MustLoadDLL(dllPath + dllName + ".so")
	defer dll.Release()

	// 获取 Add 函数
	addProc := dll.MustFindProc("Add")

	// 调用 Add 函数
	a, b := 5, 3
	ret, _, _ := addProc.Call(uintptr(a), uintptr(b))
	fmt.Printf("%d + %d = %d\n", a, b, int(ret))
}
