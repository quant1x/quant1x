package pool

import "errors"

var (
	//ErrMaxActiveConnReached 连接池超限
	ErrMaxActiveConnReached = errors.New("max active conn reached")
	// ErrClosed 连接已关闭
	ErrClosed = errors.New("pool is closed")
	// ErrIsNil 连接无效
	ErrIsNil = errors.New("connection is nil. rejecting")
)

// Pool 基本方法
type Pool interface {
	// Get 获取一个连接
	Get() (any, error)
	// Put 归还一个连接
	Put(any) error
	// Close 关闭连接
	Close(any) error
	// CloseAll 关闭全部连接
	CloseAll()
	// Release 释放连接池
	Release()
	// Len 连接数
	Len() int
}
