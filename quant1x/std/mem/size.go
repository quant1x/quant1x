package mem

import (
	"reflect"
	"unsafe"
)

// 使用反射判断泛型类型的尺寸
func v1TypeSize[T any]() uintptr {
	typ := reflect.TypeOf((*T)(nil)).Elem()
	return typ.Size()
}

// TypeSize 返回泛型类型的尺寸
func TypeSize[T any]() uintptr {
	var e T
	return unsafe.Sizeof(e)
}
