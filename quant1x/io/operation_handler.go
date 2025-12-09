package net

import (
	stdnet "net"
	"time"
)

// NetworkOperationHandler 定义了TCP连接的握手和心跳检查
type NetworkOperationHandler interface {
	// Timeout 超时时间
	Timeout() time.Duration
	// Handshake 执行TCP连接握手验证，返回验证结果和可能的错误
	// conn: 要验证的TCP连接
	// 返回值: (验证是否成功, 错误信息)
	Handshake(conn *stdnet.TCPConn) (bool, error)
	// Keepalive 检查并保持TCP连接的活动状态
	// 参数:
	//   conn: 要检查的TCP连接
	// 返回值:
	//   bool: 连接是否仍然活跃
	//   error: 操作过程中遇到的错误
	Keepalive(conn *stdnet.TCPConn) (bool, error)
	// CheckInterval 检查心跳的间隔时间
	CheckInterval() time.Duration
}
