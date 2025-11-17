package exchange

import "fmt"

// Exchange 表示交易所信息
type Exchange struct {
	Code        string   `json:"code"`                  // 交易所代码，如 "sh", "sz"
	Name        string   `json:"name"`                  // 交易所名称，如 "上海证券交易所"
	ID          MarketID `json:"id"`                    // 市场ID，对应 MarketID 枚举
	Description string   `json:"description,omitempty"` // 描述信息，可选
	IsActive    bool     `json:"is_active"`             // 是否活跃
}

// String 返回交易所的字符串表示
func (e Exchange) String() string {
	return fmt.Sprintf("%s(%s)", e.Name, e.Code)
}

// Validate 检查交易所字段的有效性
func (e Exchange) Validate() error {
	if e.Code == "" {
		return fmt.Errorf("exchange code cannot be empty")
	}
	if e.Name == "" {
		return fmt.Errorf("exchange name cannot be empty")
	}
	return nil
}

// NewExchange 创建一个新的 Exchange 实例
func NewExchange(code, name string, id MarketID) Exchange {
	return Exchange{
		Code:     code,
		Name:     name,
		ID:       id,
		IsActive: true,
	}
}

// SecurityCode 表示证券代码及其所属市场
type SecurityCode struct {
	Market MarketID // 市场ID
	Symbol string   // 证券代码
}

// String 返回证券代码的字符串表示形式，格式为"市场代码+证券代码"
func (c SecurityCode) String() string {
	return fmt.Sprintf("%s%s", c.Market, c.Symbol)
}

// Validate 检查证券代码的有效性
func (c SecurityCode) Validate() error {
	if c.Symbol == "" {
		return fmt.Errorf("security code symbol cannot be empty")
	}
	return nil
}
