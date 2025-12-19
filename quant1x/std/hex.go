package std

import "encoding/hex"

// ToHexString 字节数组转16进制字符串
func ToHexString(data []byte) string {
	return hex.EncodeToString(data)
}

// FromHexString 16进制字符串转字节数组
func FromHexString(s string) ([]byte, error) {
	return hex.DecodeString(s)
}
