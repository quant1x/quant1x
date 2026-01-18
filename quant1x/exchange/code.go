package exchange

import (
	"errors"
	"fmt"
	"regexp"
	"strings"

	"gitee.com/quant1x/quant1x/quant1x/std"
)

const MarketCnFirstListTime = "1990-12-19"

// 包级错误
var (
	ErrExchangeCodeEmpty       = errors.New("exchange code cannot be empty")
	ErrExchangeNameEmpty       = errors.New("exchange name cannot be empty")
	ErrSecurityCodeSymbolEmpty = errors.New("security code symbol cannot be empty")
)

// ExchangeInfo 表示交易所信息
type ExchangeInfo struct {
	ID          Exchange `yaml:"id"`                    // 市场ID，对应 ExchangeId 枚举
	Code        string   `yaml:"code"`                  // 交易所代码，如 "sh", "sz"
	Name        string   `yaml:"name"`                  // 交易所名称，如 "上海证券交易所"
	Description string   `yaml:"description,omitempty"` // 描述信息，可选
	IsActive    bool     `yaml:"is_active"`             // 是否活跃
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
func NewExchange(id Exchange, code, name, desc string) ExchangeInfo {
	return ExchangeInfo{
		ID:          id,
		Code:        code,
		Name:        name,
		Description: desc,
		IsActive:    true,
	}
}

// DetectSymbol 根据输入的证券代码字符串解析出市场、代码和类型信息
//
// 支持多种格式的证券代码输入：
//  1. 前缀形式：sh600000, hk00700, usappl
//  2. 后缀形式：600000.sh, 00700.hk, APPL.us
//  3. 纯数字/字母形式：600000, 00700, APPL
//
// 参数：
//
//	input - 待解析的证券代码字符串
//
// 返回值：
//
//	SecurityCode 结构体，包含市场ID、证券代码和证券类型
//
// 处理逻辑：
//  1. 首先去除输入字符串的空格
//  2. 根据不同的代码格式（前缀/后缀/纯形式）进行解析
//  3. 对于纯数字/字母形式，根据长度和规则匹配市场
//  4. 对于A股代码，会进一步匹配各交易所的特定规则
//  5. 返回解析后的SecurityCode结构体，包含市场、代码和类型信息
func DetectSymbol(input string) InstrumentInfo {
	raw := strings.TrimSpace(input)
	if raw == "" {
		return InstrumentInfo{Exchange: ExchangeSSE, Ticker: "", Type: SecurityTypeUnknown}
	}
	pureCode := strings.ToLower(raw)
	ticker := ""                // 纯代码部分
	exchange := ExchangeUnknown // 默认未知市场
	typ := SecurityTypeUnknown  // 默认未知类型
	if std.StartsWith(pureCode, AllExchangeCodes) {
		// 前缀形式: sh600000, hk00700, usappl
		ticker = pureCode[2:]
		exchange = ParseExchangeCode(pureCode[:2])
	} else if std.EndsWith(pureCode, AllExchangeCodes) && len(pureCode) >= 3 && pureCode[len(pureCode)-3] == '.' {
		// 后缀形式: 600000.sh, 00700.hk, APPL.us
		suffixLength := 3 // 包含点号
		ticker = pureCode[:len(pureCode)-suffixLength]
		exchange = ParseExchangeCode(pureCode[len(pureCode)-2:])
	} else {
		// 纯形式或字母: 600000, 00700, APPL
		codeLength := len(pureCode)
		switch codeLength {
		case 4: // 可能为美股代码（4位字母），否则视为未知
			// 仅当全部为字母时认定为美股代码
			if regexp.MustCompile(`^[a-z]{4}$`).MatchString(pureCode) {
				exchange = ExchangeUS
				ticker = pureCode
				typ = SecurityTypeStock
			} else {
				// 未识别的 4 位代码，标记为未知
				exchange = ExchangeUnknown
				ticker = ""
				typ = SecurityTypeUnknown
			}
		case 5: // 港股代码，5位数字
			exchange = ExchangeHKEX
			ticker = pureCode
		case 6: // A股代码，6位数
			// 1. 全局规则优先(如板块指数)
			if typ_, desc := matchRule(pureCode, globalRules); typ_ != SecurityTypeUnknown {
				_ = desc
				ticker = pureCode
				exchange = ExchangeSSE
				return InstrumentInfo{Exchange: exchange, Ticker: ticker, Type: typ_}
			}
			// 2. 按市场匹配规则
			// 2.1 深交所
			if typ_, desc := matchRule(pureCode, szseRules); typ_ != SecurityTypeUnknown {
				_ = desc
				ticker = pureCode
				exchange = ExchangeSZSE
				return InstrumentInfo{Exchange: exchange, Ticker: ticker, Type: typ_}
			}
			// 2.2 北交所
			if typ_, desc := matchRule(pureCode, bseRules); typ_ != SecurityTypeUnknown {
				_ = desc
				ticker = pureCode
				exchange = ExchangeBSE
				return InstrumentInfo{Exchange: exchange, Ticker: ticker, Type: typ_}
			}
			// 2.3 上交所
			if typ_, desc := matchRule(pureCode, sseRules); typ_ != SecurityTypeUnknown {
				_ = desc
				ticker = pureCode
				exchange = ExchangeSSE
				return InstrumentInfo{Exchange: exchange, Ticker: ticker, Type: typ_}
			}
		}
	}

	if exchange == ExchangeUnknown {
		// 无法识别市场
		return InstrumentInfo{Exchange: ExchangeUnknown, Ticker: "", Type: SecurityTypeUnknown}
	}

	if typ == SecurityTypeUnknown {
		// 基于市场规则解析类型
		var rules []CodeRule
		switch exchange {
		case ExchangeSSE:
			rules = sseRules
		case ExchangeSZSE:
			rules = szseRules
		case ExchangeBSE:
			rules = bseRules
		case ExchangeHKEX:
			rules = hkexRules
		case ExchangeUS:
			typ = SecurityTypeStock // 美股默认股票
			return InstrumentInfo{Exchange: exchange, Ticker: ticker, Type: typ}
		default:
			return InstrumentInfo{Exchange: ExchangeUnknown, Ticker: "", Type: SecurityTypeUnknown}
		}
		if typ_, _ := matchRule(ticker, rules); typ_ != SecurityTypeUnknown {
			typ = typ_
			return InstrumentInfo{Exchange: exchange, Ticker: ticker, Type: typ}
		} else {
			return InstrumentInfo{Exchange: ExchangeUnknown, Ticker: "", Type: SecurityTypeUnknown}
		}
	} else {
		// 已识别类型，直接返回, 不进行规则匹配
		// 适用于美股等市场
		return InstrumentInfo{Exchange: exchange, Ticker: ticker, Type: typ}
	}
}

// DetectWithExchange 根据交易所ID和代码检测证券类型
//
// 参数:
//
//	exchangeId - 交易所标识符
//	symbol - 证券代码
//
// 返回值:
//
//	SecurityCode - 包含市场、代码和类型的证券信息结构体
//
// 支持以下交易所:
//   - 上海证券交易所 (SSE)
//   - 深圳证券交易所 (SZSE)
//   - 北京证券交易所 (BJSE)
//   - 香港交易所 (HKSE)
//   - 美国市场 (USA)
//
// 对于不支持的交易所返回未知类型
func DetectWithExchange(exchange Exchange, symbol string) InstrumentInfo {
	// 基于市场规则解析类型
	var rules []CodeRule
	switch exchange {
	case ExchangeSSE:
		rules = sseRules
	case ExchangeSZSE:
		rules = szseRules
	case ExchangeBSE:
		rules = bseRules
	case ExchangeHKEX:
		rules = hkexRules
	case ExchangeUS:
		typ := SecurityTypeStock // 美股默认股票
		return InstrumentInfo{Exchange: exchange, Ticker: symbol, Type: typ}
	default:
		return InstrumentInfo{Exchange: ExchangeUnknown, Ticker: "", Type: SecurityTypeUnknown}
	}
	if typ, _ := matchRule(symbol, rules); typ != SecurityTypeUnknown {
		return InstrumentInfo{Exchange: exchange, Ticker: symbol, Type: typ}
	} else {
		return InstrumentInfo{Exchange: ExchangeUnknown, Ticker: "", Type: SecurityTypeUnknown}
	}
}
