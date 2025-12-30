package data

import (
	"errors"
	"fmt"
	"regexp"
	"strings"
	"sync"
	"sync/atomic"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
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

var (
	onceFirstMarketDate sync.Once
	firstMarketDate     exchange.Timestamp
)

func initFirstMarketDate() {
	// 与 C++ 严格保持一致：解析常量并取盘前时间。
	ts, err := exchange.NewTimestampFromString(exchange.MarketCnFirstListTime)
	if err != nil {
		panic(fmt.Sprintf("datasets: failed to parse MarketCnFirstListTime: %v", err))
	}
	firstMarketDate = ts.PreMarketTime()
}

// GetFirstMarketDate 返回指定交易所的第一个市场交易日时间戳
// 该函数是线程安全的，首次调用时会初始化数据
func GetFirstMarketDate(exchange ExchangeId) exchange.Timestamp {
	onceFirstMarketDate.Do(initFirstMarketDate)
	_ = exchange
	return firstMarketDate
}

// SecurityType 表示证券类型（枚举，基于 uint8）
type SecurityType uint8

const (
	SecurityUnknown   SecurityType = iota // 未知类型
	SecurityStock                         // A股/普通股票
	SecurityETF                           // 交易所交易基金
	SecurityFund                          // 各类基金（如 LOF、封闭式基金等）
	SecurityBond                          // 债券（公司债、可转债、可交换债等）
	SecurityBStock                        // B股
	SecurityIPO                           // 新股申购/新股
	SecurityIndex                         // 指数（如上证指数、深证指数等）
	SecurityBlock                         // 板块/板块指数
	SecurityOption                        // 期权
	SecurityFuture                        // 期货
	SecurityWarrant                       // 权证
	SecurityForex                         // 外汇
	SecurityCommodity                     // 商品
	SecurityOther     = 255               // 其他类型
)

// String 返回证券类型的字符串表示，满足 fmt.Stringer 接口
func (s SecurityType) String() string {
	return s.StringWithLocale(getLocale())
}

// locale handling: stored as string "en" or "zh"
var locale atomic.Value

const (
	localeEN = "en"
	localeZH = "zh"
)

func init() {
	_, offset := time.Now().Zone()
	if offset == 8*3600 {
		locale.Store(localeZH)
	} else {
		locale.Store(localeEN)
	}
}

// SetLocale 设置全局 locale（"zh" 或 "en"），对外可调用以覆盖自动检测
func SetLocale(l string) {
	l = strings.ToLower(strings.TrimSpace(l))
	if l == "zh" || l == "zh-cn" || l == "cn" {
		locale.Store(localeZH)
		return
	}
	locale.Store(localeEN)
}

func getLocale() string {
	v := locale.Load()
	if v == nil {
		return localeEN
	}
	if s, ok := v.(string); ok {
		return s
	}
	return localeEN
}

// 标签表，便于维护和扩展
var labels = map[SecurityType]map[string]string{
	SecurityStock:     {localeEN: "Stock", localeZH: "股票"},
	SecurityETF:       {localeEN: "ETF", localeZH: "ETF"},
	SecurityFund:      {localeEN: "Fund", localeZH: "基金"},
	SecurityBond:      {localeEN: "Bond", localeZH: "债券"},
	SecurityBStock:    {localeEN: "BStock", localeZH: "B股"},
	SecurityIPO:       {localeEN: "IPO", localeZH: "新股"},
	SecurityIndex:     {localeEN: "Index", localeZH: "指数"},
	SecurityBlock:     {localeEN: "Block", localeZH: "板块"},
	SecurityOption:    {localeEN: "Option", localeZH: "期权"},
	SecurityFuture:    {localeEN: "Future", localeZH: "期货"},
	SecurityWarrant:   {localeEN: "Warrant", localeZH: "权证"},
	SecurityForex:     {localeEN: "Forex", localeZH: "外汇"},
	SecurityCommodity: {localeEN: "Commodity", localeZH: "商品"},
	SecurityOther:     {localeEN: "Other", localeZH: "其他"},
	SecurityUnknown:   {localeEN: "Unknown", localeZH: "未知"},
}

// StringWithLocale 返回指定 locale 的文本表示（不修改全局状态）
func (s SecurityType) StringWithLocale(loc string) string {
	if m, ok := labels[s]; ok {
		if v, ok2 := m[loc]; ok2 {
			return v
		}
		// fallback to en
		if v, ok2 := m[localeEN]; ok2 {
			return v
		}
	}
	// 最后兜底
	if m, ok := labels[SecurityUnknown]; ok {
		if v, ok2 := m[loc]; ok2 {
			return v
		}
		if v, ok2 := m[localeEN]; ok2 {
			return v
		}
	}
	return ""
}

func (s *SecurityType) MarshalCSV() (string, error) {
	return s.StringWithLocale(getLocale()), nil
}

func (s *SecurityType) UnmarshalCSV(val string) error {
	// Normalize input
	v := strings.TrimSpace(val)
	if v == "" {
		*s = SecurityUnknown
		return nil
	}
	lv := strings.ToLower(v)

	// 1) Try matching against localized labels map (both zh/en)
	for typ, m := range labels {
		for _, label := range m {
			if strings.ToLower(label) == lv {
				*s = typ
				return nil
			}
		}
	}

	// 2) Fallback: compact synonym map for common CSV values
	synonyms := map[string]SecurityType{
		"stock":     SecurityStock,
		"a股":        SecurityStock,
		"普通股票":      SecurityStock,
		"a股股票":      SecurityStock,
		"a股普通股票":    SecurityStock,
		"etf":       SecurityETF,
		"etf基金":     SecurityETF,
		"交易所交易基金":   SecurityETF,
		"fund":      SecurityFund,
		"基金":        SecurityFund,
		"各类基金":      SecurityFund,
		"封闭式基金":     SecurityFund,
		"lof基金":     SecurityFund,
		"bond":      SecurityBond,
		"债券":        SecurityBond,
		"公司债":       SecurityBond,
		"可转债":       SecurityBond,
		"可交换债":      SecurityBond,
		"bstock":    SecurityBStock,
		"b股":        SecurityBStock,
		"ipo":       SecurityIPO,
		"新股申购":      SecurityIPO,
		"新股":        SecurityIPO,
		"index":     SecurityIndex,
		"指数":        SecurityIndex,
		"上证指数":      SecurityIndex,
		"深证指数":      SecurityIndex,
		"block":     SecurityBlock,
		"板块":        SecurityBlock,
		"板块指数":      SecurityBlock,
		"option":    SecurityOption,
		"期权":        SecurityOption,
		"future":    SecurityFuture,
		"期货":        SecurityFuture,
		"warrant":   SecurityWarrant,
		"权证":        SecurityWarrant,
		"forex":     SecurityForex,
		"外汇":        SecurityForex,
		"commodity": SecurityCommodity,
		"商品":        SecurityCommodity,
		"other":     SecurityOther,
		"其他":        SecurityOther,
	}

	if t, ok := synonyms[lv]; ok {
		*s = t
		return nil
	}

	// 3) Not matched -> Unknown
	*s = SecurityUnknown
	return nil
}

// 包级错误
var (
	ErrExchangeCodeEmpty       = errors.New("exchange code cannot be empty")
	ErrExchangeNameEmpty       = errors.New("exchange name cannot be empty")
	ErrSecurityCodeSymbolEmpty = errors.New("security code symbol cannot be empty")
)

// SecurityCode 表示证券代码及其所属交易所
type SecurityCode struct {
	Exchange ExchangeId   // 交易所ID
	Symbol   string       // 证券代码
	Type     SecurityType // 证券类型
}

// String 返回证券代码的字符串表示形式，格式为"市场代码+证券代码"
func (sc SecurityCode) String() string {
	return fmt.Sprintf("%s%s", sc.Exchange, sc.Symbol)
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

	return fmt.Sprintf("%s%s", sc.Exchange, sc.Symbol), nil
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

	sc.Exchange = tmp.Exchange
	sc.Symbol = tmp.Symbol
	sc.Type = tmp.Type
	return nil
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
func DetectSymbol(input string) SecurityCode {
	raw := strings.TrimSpace(input)
	if raw == "" {
		return SecurityCode{Exchange: ExchangeIdShangHai, Symbol: "", Type: SecurityUnknown}
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
				return SecurityCode{Exchange: exchangeId, Symbol: symbol, Type: typ_}
			}
			// 2. 按市场匹配规则
			// 2.1 深交所
			if typ_, desc := matchRule(pureCode, szseRules); typ_ != SecurityUnknown {
				_ = desc
				symbol = pureCode
				exchangeCode = ExchangeSZSE
				exchangeId = ExchangeIdShenZhen
				return SecurityCode{Exchange: exchangeId, Symbol: symbol, Type: typ_}
			}
			// 2.2 北交所
			if typ_, desc := matchRule(pureCode, bjseRules); typ_ != SecurityUnknown {
				_ = desc
				symbol = pureCode
				exchangeCode = ExchangeBJSE
				exchangeId = ExchangeIdBeiJing
				return SecurityCode{Exchange: exchangeId, Symbol: symbol, Type: typ_}
			}
			// 2.3 上交所
			if typ_, desc := matchRule(pureCode, sseRules); typ_ != SecurityUnknown {
				_ = desc
				symbol = pureCode
				exchangeCode = ExchangeSSE
				exchangeId = ExchangeIdShangHai
				return SecurityCode{Exchange: exchangeId, Symbol: symbol, Type: typ_}
			}
		}
	}

	if exchangeId == ExchangeIdUnknown {
		// 无法识别市场
		return SecurityCode{Exchange: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}
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
			return SecurityCode{Exchange: exchangeId, Symbol: symbol, Type: typ}
		default:
			return SecurityCode{Exchange: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}
		}
		if typ_, _ := matchRule(symbol, rules); typ_ != SecurityUnknown {
			typ = typ_
			return SecurityCode{Exchange: exchangeId, Symbol: symbol, Type: typ}
		} else {
			return SecurityCode{Exchange: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}
		}
	} else {
		// 已识别类型，直接返回, 不进行规则匹配
		// 适用于美股等市场
		return SecurityCode{Exchange: exchangeId, Symbol: symbol, Type: typ}
	}
}

// DetectWithExchangeId 根据交易所ID和代码检测证券类型
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
		return SecurityCode{Exchange: exchangeId, Symbol: symbol, Type: typ}
	default:
		return SecurityCode{Exchange: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}
	}
	if typ, _ := matchRule(symbol, rules); typ != SecurityUnknown {
		return SecurityCode{Exchange: exchangeId, Symbol: symbol, Type: typ}
	} else {
		return SecurityCode{Exchange: ExchangeIdUnknown, Symbol: "", Type: SecurityUnknown}
	}
}

// DetectSecurity 解析证券代码，返回(市场, 类型, 描述)
func DetectSecurity(input string) (ExchangeCode, SecurityType, string) {
	// 标准化：去除空格、点，转小写
	s := strings.ToLower(strings.ReplaceAll(strings.TrimSpace(input), ".", ""))

	var market ExchangeCode
	var code string

	// 1. 尝试解析显式市场标识(前缀或后缀)
	if len(s) >= 7 {
		if strings.HasPrefix(s, "sh") || strings.HasPrefix(s, "sz") || strings.HasPrefix(s, "bj") || strings.HasPrefix(s, "hk") {
			market = ExchangeCode(s[:2])
			code = s[2:]
		} else if strings.HasSuffix(s, "sh") || strings.HasSuffix(s, "sz") || strings.HasSuffix(s, "bj") || strings.HasSuffix(s, "hk") {
			market = ExchangeCode(s[len(s)-2:])
			code = s[:len(s)-2]
		}
	}

	// 2. 若无市场标识，自动推断市场
	if market == "" {
		if regexp.MustCompile(`^\d{6}$`).MatchString(s) {
			code = s
			switch {
			case strings.HasPrefix(code, "6") || strings.HasPrefix(code, "5") ||
				strings.HasPrefix(code, "9") || strings.HasPrefix(code, "7") ||
				strings.HasPrefix(code, "000"):
				market = ExchangeSSE
			case strings.HasPrefix(code, "0") || strings.HasPrefix(code, "3") ||
				strings.HasPrefix(code, "1") || strings.HasPrefix(code, "2"):
				market = ExchangeSZSE
			case strings.HasPrefix(code, "8") || strings.HasPrefix(code, "92"):
				market = ExchangeBJSE
			default:
				return "", SecurityUnknown, "无法识别市场"
			}
		} else if regexp.MustCompile(`^\d{5}$`).MatchString(s) {
			code = s
			market = ExchangeHK
		} else {
			code = s
		}
	} else if code == "" {
		code = s
	}

	// 3. 验证 code 为5或6位纯数字
	if !regexp.MustCompile(`^\d{5,6}$`).MatchString(code) {
		return "", SecurityUnknown, "代码格式错误(应为5或6位数字)"
	}

	// 4. 全局规则优先(如板块指数)
	if typ, desc := matchRule(code, globalRules); typ != SecurityUnknown {
		return ExchangeSSE, typ, desc // 板块指数归属上证体系
	}

	// 5. 按市场匹配规则
	var rules []CodeRule
	switch market {
	case ExchangeSSE:
		rules = sseRules
	case ExchangeSZSE:
		rules = szseRules
	case ExchangeBJSE:
		rules = bjseRules
	case ExchangeHK:
		rules = hkseRules
	default:
		return market, SecurityUnknown, "不支持的市场"
	}

	if typ, desc := matchRule(code, rules); typ != SecurityUnknown {
		return market, typ, desc
	}

	return market, SecurityUnknown, "未匹配到规则"
}
