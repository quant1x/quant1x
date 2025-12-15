package exchange

import (
	"errors"
	"fmt"
)

// ExchangeCode 表示交易所代码/标识
type ExchangeCode string

const (
	ExchangeUnknown ExchangeCode = "unknown" // 未知交易所
	ExchangeSSE     ExchangeCode = "sh"      // 上海证券交易所
	ExchangeSZSE    ExchangeCode = "sz"      // 深圳证券交易所
	ExchangeBJSE    ExchangeCode = "bj"      // 北京证券交易所
	ExchangeHK      ExchangeCode = "hk"      // 香港证券交易所
	ExchangeUS      ExchangeCode = "us"      // 美国交易所
)

// String 返回交易所代码的字符串表示，满足 fmt.Stringer 接口
func (e ExchangeCode) String() string {
	return string(e)
}

// ToExchangeId 将 ExchangeCode 转换为对应的 ExchangeId
//
//	如果无法识别返回错误
func (e ExchangeCode) Id() (ExchangeId, error) {
	switch e {
	case ExchangeSZSE:
		return ExchangeIdShenZhen, nil
	case ExchangeSSE:
		return ExchangeIdShangHai, nil
	case ExchangeBJSE:
		return ExchangeIdBeiJing, nil
	case ExchangeHK:
		return ExchangeIdHongKong, nil
	case ExchangeUS:
		return ExchangeIdUSA, nil
	default:
		return 0, fmt.Errorf("unknown exchange code: %s", e)
	}
}

var (
	// AllExchangeCodes 包含所有已知的交易所代码
	AllExchangeCodes = []string{
		ExchangeSSE.String(),
		ExchangeSZSE.String(),
		ExchangeBJSE.String(),
		ExchangeHK.String(),
		ExchangeUS.String(),
	}
)

// ExchangeId 表示交易所ID
type ExchangeId uint8

const (
	ExchangeIdUnknown  ExchangeId = 255 // 未知交易所
	ExchangeIdShenZhen ExchangeId = 0   // 深圳证券交易所
	ExchangeIdShangHai ExchangeId = 1   // 上海证券交易所
	ExchangeIdBeiJing  ExchangeId = 2   // 北京证券交易所
	ExchangeIdHongKong ExchangeId = 21  // 香港交易所
	ExchangeIdUSA      ExchangeId = 22  // 美国交易所
)

// String 将交易所ID转换为对应的字符串表示
//
//	如果传入未知的交易所ID会触发panic
func (m ExchangeId) String() string {
	switch m {
	case ExchangeIdShenZhen:
		return string(ExchangeSZSE)
	case ExchangeIdShangHai:
		return string(ExchangeSSE)
	case ExchangeIdBeiJing:
		return string(ExchangeBJSE)
	case ExchangeIdHongKong:
		return string(ExchangeHK)
	case ExchangeIdUSA:
		return string(ExchangeUS)
	default:
		panic(fmt.Sprintf("unknown market id: %d", m))
	}
}

// ExchangeInfo 表示交易所信息
type ExchangeInfo struct {
	ID          ExchangeId `yaml:"id"`                    // 市场ID，对应 ExchangeId 枚举
	Code        string     `yaml:"code"`                  // 交易所代码，如 "sh", "sz"
	Name        string     `yaml:"name"`                  // 交易所名称，如 "上海证券交易所"
	Description string     `yaml:"description,omitempty"` // 描述信息，可选
	IsActive    bool       `yaml:"is_active"`             // 是否活跃
}

// String 返回交易所的字符串表示
func (e ExchangeInfo) String() string {
	return fmt.Sprintf("%s(%s)", e.Name, e.Code)
}

// Validate 检查交易所字段的有效性
func (e ExchangeInfo) Validate() error {
	if e.Code == "" {
		return ErrExchangeCodeEmpty
	}
	if e.Name == "" {
		return ErrExchangeNameEmpty
	}
	return nil
}

// NewExchange 创建一个新的 Exchange 实例，带描述信息
func NewExchange(code, name, desc string, id ExchangeId) ExchangeInfo {
	return ExchangeInfo{
		Code:        code,
		Name:        name,
		ID:          id,
		Description: desc,
		IsActive:    true,
	}
}

// SecurityCode 表示证券代码及其所属交易所
type SecurityCode struct {
	Market ExchangeId   // 交易所ID
	Symbol string       // 证券代码
	Type   SecurityType // 证券类型
}

// 包级错误
var (
	ErrExchangeCodeEmpty       = errors.New("exchange code cannot be empty")
	ErrExchangeNameEmpty       = errors.New("exchange name cannot be empty")
	ErrSecurityCodeSymbolEmpty = errors.New("security code symbol cannot be empty")
)

// String 返回证券代码的字符串表示形式，格式为"市场代码+证券代码"
func (c SecurityCode) String() string {
	return fmt.Sprintf("%s%s", c.Market, c.Symbol)
}

// Validate 检查证券代码的有效性
func (c SecurityCode) Validate() error {
	if c.Symbol == "" {
		return ErrSecurityCodeSymbolEmpty
	}
	return nil
}
