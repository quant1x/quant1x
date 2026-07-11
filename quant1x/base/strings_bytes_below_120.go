//go:build !go1.20

package base

import (
	"reflect"
	"unsafe"
)

// Bytes2String 字节数组转字符串
func Bytes2String(b []byte) string {
	return *(*string)(unsafe.Pointer(&b))
}

// String2Bytes 字符串转字节数组
func String2Bytes(s string) []byte {
	strh := (*reflect.StringHeader)(unsafe.Pointer(&s))
	var sh reflect.SliceHeader
	sh.Data = strh.Data
	sh.Len = strh.Len
	sh.Cap = strh.Len
	return *(*[]byte)(unsafe.Pointer(&sh))
}
