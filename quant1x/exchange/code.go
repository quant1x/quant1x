package exchange

import (
	"errors"
	"fmt"
	"regexp"
	"strings"

	"gitee.com/quant1x/quant1x/quant1x/std"
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
func (e ExchangeCode) Id() ExchangeId {
	switch e {
	case ExchangeSZSE:
		return ExchangeIdShenZhen
	case ExchangeSSE:
		return ExchangeIdShangHai
	case ExchangeBJSE:
		return ExchangeIdBeiJing
	case ExchangeHK:
		return ExchangeIdHongKong
	case ExchangeUS:
		return ExchangeIdUSA
	default:
		return ExchangeIdUnknown
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
func (e ExchangeId) String() string {
	switch e {
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
		//panic(fmt.Sprintf("unknown market id: %d", e))
		return ExchangeUnknown.String()
	}
}

func (e *ExchangeId) UnmarshalCSV(val string) error {
	text := strings.TrimSpace(val)
	exchangeCode := ExchangeCode(text)
	*e = exchangeCode.Id()
	if *e == ExchangeIdUnknown {
		return fmt.Errorf("invalid exchange code: %s", text)
	}
	return nil
}

// 包级错误
var (
	ErrExchangeCodeEmpty       = errors.New("exchange code cannot be empty")
	ErrExchangeNameEmpty       = errors.New("exchange name cannot be empty")
	ErrSecurityCodeSymbolEmpty = errors.New("security code symbol cannot be empty")
)

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

// String 返回证券代码的字符串表示形式，格式为"市场代码+证券代码"
func (sc SecurityCode) String() string {
	return fmt.Sprintf("%s%s", sc.Market, sc.Symbol)
}

// Validate 检查证券代码的有效性
func (sc SecurityCode) Validate() error {
	if sc.Symbol == "" {
		return ErrSecurityCodeSymbolEmpty
	}
	return nil
}

func (sc *SecurityCode) MarshalCSV() (string, error) {
	if sc == nil {
		return "", nil
	}

	return fmt.Sprintf("%s%s", sc.Market, sc.Symbol), nil
}

func (sc *SecurityCode) UnmarshalCSV(val string) error {
	text := strings.TrimSpace(val)
	if len(text) < 2 {
		return ErrSecurityCodeSymbolEmpty
	}

	marketCode := text[:2]
	exchangeCode := ExchangeCode(marketCode)
	exchangeId := exchangeCode.Id()
	if exchangeId == ExchangeIdUnknown {
		return fmt.Errorf("invalid exchange code: %s", marketCode)
	}
	symbol := text[2:]

	tmp := DetectWithExchangeId(exchangeId, symbol)

	sc.Market = tmp.Market
	sc.Symbol = tmp.Symbol
	sc.Type = tmp.Type
	return nil
}

// Detect 根据输入的证券代码字符串解析出市场、代码和类型信息
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
func Detect(input string) SecurityCode {
	raw := strings.TrimSpace(input)
	if raw == "" {
		return SecurityCode{Market: ExchangeIdShangHai, Symbol: "", Type: SecurityUnknown}
	}
	pureCode := strings.ToLower(raw)
	symbol := ""                    // 纯代码部分
	exchangeCode := ExchangeUnknown // 默认未知市场
	exchangeId := ExchangeIdUnknown // 默认未知市场
	typ := SecurityUnknown          // 默认未知类型
	if std.StartsWith(pureCode, AllExchangeCodes) {
		// 前缀形式: sh600000, hk00700, usappl
		symbol = pureCode[2:]
		exchangeCode = ExchangeCode(pureCode[:2])
		exchangeId = exchangeCode.Id()
	} else if std.EndsWith(pureCode, AllExchangeCodes) && len(pureCode) >= 3 && pureCode[len(pureCode)-3] == '.' {
		// 后缀形式: 600000.sh, 00700.hk, APPL.us
		suffixLength := 3 // 包含点号
		symbol = pureCode[:len(pureCode)-suffixLength]
		exchangeCode = ExchangeCode(pureCode[len(pureCode)-2:])
		exchangeId = exchangeCode.Id()
	} else {
		// 纯形式或字母: 600000, 00700, APPL
		codeLength := len(pureCode)
		switch codeLength {
		case 4: // 可能为美股代码（4位字母），否则视为未知
			// 仅当全部为字母时认定为美股代码
			if regexp.MustCompile(`^[a-z]{4}$`).MatchString(pureCode) {
				exchangeCode = ExchangeUS
				exchangeId = ExchangeIdUSA
				symbol = pureCode
				typ = SecurityStock
			} else {
				// 未识别的 4 位代码，标记为未知
				exchangeCode = ExchangeUnknown
				exchangeId = ExchangeIdUnknown
				symbol = ""
				typ = SecurityUnknown
			}
		case 5: // 港股代码，5位数字
			exchangeCode = ExchangeHK
			exchangeId = ExchangeIdHongKong
			symbol = pureCode
		case 6: // A股代码，6位数
			// 1. 全局规则优先(如板块指数)
			if typ_, desc := matchRule(pureCode, globalRules); typ_ != SecurityUnknown {
				_ = desc
				symbol = pureCode
				exchangeCode = ExchangeSSE
				exchangeId = ExchangeIdShangHai
				return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ_}
			}
			// 2. 按市场匹配规则
			// 2.1 深交所
			if typ_, desc := matchRule(pureCode, szseRules); typ_ != SecurityUnknown {
				_ = desc
				symbol = pureCode
				exchangeCode = ExchangeSZSE
				exchangeId = ExchangeIdShenZhen
				return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ_}
			}
			// 2.2 北交所
			if typ_, desc := matchRule(pureCode, bjseRules); typ_ != SecurityUnknown {
				_ = desc
				symbol = pureCode
				exchangeCode = ExchangeBJSE
				exchangeId = ExchangeIdBeiJing
				return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ_}
			}
			// 2.3 上交所
			if typ_, desc := matchRule(pureCode, sseRules); typ_ != SecurityUnknown {
				_ = desc
				symbol = pureCode
				exchangeCode = ExchangeSSE
				exchangeId = ExchangeIdShangHai
				return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ_}
			}
		}
	}

	if exchangeId == ExchangeIdUnknown {
		// 无法识别市场
		return SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}
	}

	if typ == SecurityUnknown {
		// 基于市场规则解析类型
		var rules []CodeRule
		switch exchangeId {
		case ExchangeIdShangHai:
			rules = sseRules
		case ExchangeIdShenZhen:
			rules = szseRules
		case ExchangeIdBeiJing:
			rules = bjseRules
		case ExchangeIdHongKong:
			rules = hkseRules
		case ExchangeIdUSA:
			typ = SecurityStock // 美股默认股票
			return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ}
		default:
			return SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}
		}
		if typ_, _ := matchRule(symbol, rules); typ_ != SecurityUnknown {
			typ = typ_
			return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ}
		} else {
			return SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}
		}
	} else {
		// 已识别类型，直接返回, 不进行规则匹配
		// 适用于美股等市场
		return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ}
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
func DetectWithExchangeId(exchangeId ExchangeId, symbol string) SecurityCode {
	// 基于市场规则解析类型
	var rules []CodeRule
	switch exchangeId {
	case ExchangeIdShangHai:
		rules = sseRules
	case ExchangeIdShenZhen:
		rules = szseRules
	case ExchangeIdBeiJing:
		rules = bjseRules
	case ExchangeIdHongKong:
		rules = hkseRules
	case ExchangeIdUSA:
		typ := SecurityStock // 美股默认股票
		return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ}
	default:
		return SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}
	}
	if typ, _ := matchRule(symbol, rules); typ != SecurityUnknown {
		return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ}
	} else {
		return SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}
	}
}
