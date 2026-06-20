package std

import (
	"errors"

	"github.com/quant1x/quant1x/quant1x/io"
)

// connectionProvider 由 tdx 包注入, 避免 std → tdx 的循环导入.
var connectionProvider func() (*io.Connection, func(), error)

// SetConnectionProvider 设置连接获取函数, 由 tdx 包在 init 时调用.
func SetConnectionProvider(provider func() (*io.Connection, func(), error)) {
	connectionProvider = provider
}

// GetStdConnection 获取标准连接池中的连接.
// 需先通过 SetConnectionProvider 注入 provider, 否则返回 error.
func GetStdConnection() (*io.Connection, func(), error) {
	if connectionProvider == nil {
		return nil, nil, errors.New("connection provider not initialized, call SetConnectionProvider first")
	}
	return connectionProvider()
}
