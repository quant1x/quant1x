//go:build go1.20

package base

import (
	"unsafe"
)

// Bytes2String 字节数组转字符串
func Bytes2String(b []byte) string {
	return unsafe.String(&b[0], len(b))
}

// String2Bytes 字符串转字节数组
func String2Bytes(s string) []byte {
	return unsafe.Slice(unsafe.StringData(s), len(s))
}
