package runtime

import (
	"fmt"
	"reflect"
	"testing"
)

func test1(cb func()) {
	v := reflect.ValueOf(cb)
	fmt.Println(v)
	//p := unsafe.Pointer(&cb)
	//pc := uintptr(p)
	pc := v.Pointer()
	funcName := FuncName(pc)
	fmt.Println(funcName)
}

func foo() {
	fmt.Println(1)
}

func TestFunction(t *testing.T) {
	fmt.Println(Caller())
	test1(foo)
	fmt.Println(FuncInfo(foo))
}
