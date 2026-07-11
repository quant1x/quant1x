package base

import (
	"fmt"
)

type Throwable interface {
	error
	Code() int
}

type Exception struct {
	Throwable
	code    int
	message string
}

// New 创建一个新的错误信息, 包含一个状态码和信息
func NewException(code int, message string, a ...any) *Exception {
	return &Exception{
		code:    code,
		message: fmt.Sprintf(message, a...),
	}
}

// 格式化输出错误信息
func (e Exception) Error() string {
	return fmt.Sprintf("#%d, message=%s", e.code, e.message)
}

// Code 返回异常的错误码
func (e Exception) Code() int {
	return e.code
}

// Success 判断异常是否为成功状态(code为0)
func (e Exception) Success() bool {
	return e.code == 0
}
