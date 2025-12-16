package exchange

import (
	"regexp"
	"strings"
	"sync/atomic"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/std"
)

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

// CodeRule 表示一条证券代码前缀规则
type CodeRule struct {
	Prefix string       // 前缀，如 "600", "920"
	Type   SecurityType // 类型
	Desc   string       // 描述(用于调试或日志)
}

// ========== 全局规则(跨市场，优先匹配)==========
var globalRules = []CodeRule{
	{"880", SecurityBlock, "板块指数(通达信)"},
	{"881", SecurityBlock, "板块指数(通达信)"},
}

// ========== 上交所规则(SSE)==========
var sseRules = []CodeRule{
	// 指数
	{"000", SecurityIndex, "上证指数"},
	// ETF
	{"51", SecurityETF, "上交所ETF(510-519)"},
	{"588", SecurityETF, "科创板ETF"},
	// 其他基金
	{"50", SecurityFund, "LOF/封闭式基金"},
	{"52", SecurityFund, "其他基金"},
	// A股
	{"600", SecurityStock, "主板A股"},
	{"601", SecurityStock, "主板A股"},
	{"603", SecurityStock, "主板A股"},
	{"605", SecurityStock, "主板A股"},
	// 科创板
	{"688", SecurityStock, "科创板"},
	{"689", SecurityStock, "科创板CDR"},
	// B股
	{"900", SecurityBStock, "B股"},
	// 债券
	{"110", SecurityBond, "债券"},
	{"113", SecurityBond, "可转债"},
	{"118", SecurityBond, "可交换债"},
	{"120", SecurityBond, "公司债"},
	{"123", SecurityBond, "可转债"},
	{"127", SecurityBond, "可转债"},
	{"128", SecurityBond, "可转债"},
	// 新股申购
	{"730", SecurityIPO, "新股申购"},
	{"780", SecurityIPO, "新股申购"},
}

// ========== 深交所规则(SZSE)==========
var szseRules = []CodeRule{
	// 指数
	{"399", SecurityIndex, "深证指数"},
	// ETF
	{"159", SecurityETF, "深交所ETF"},
	// 其他基金
	{"150", SecurityFund, "LOF"},
	{"160", SecurityFund, "LOF"},
	{"161", SecurityFund, "LOF"},
	{"162", SecurityFund, "LOF"},
	{"163", SecurityFund, "LOF"},
	{"164", SecurityFund, "LOF"},
	{"167", SecurityFund, "LOF"},
	{"168", SecurityFund, "LOF"},
	{"169", SecurityFund, "LOF"},
	{"184", SecurityFund, "封闭式基金"},
	// A股(主板 + 创业板)
	{"000", SecurityStock, "主板A股"},
	{"001", SecurityStock, "主板A股"},
	{"002", SecurityStock, "主板A股"},
	{"003", SecurityStock, "主板A股"},
	{"300", SecurityStock, "创业板"},
	{"301", SecurityStock, "创业板"},
	// B股
	{"200", SecurityBStock, "B股"},
	// 债券
	{"110", SecurityBond, "可转债"},
	{"111", SecurityBond, "可转债"},
	{"118", SecurityBond, "可交换债"},
	{"123", SecurityBond, "可转债"},
	{"127", SecurityBond, "可转债"},
	{"128", SecurityBond, "可转债"},
}

// ========== 北交所规则(BJSE)==========
var bjseRules = []CodeRule{
	// 指数
	{"899", SecurityIndex, "北交所指数"},
	// 新上市公司(2024年起使用 920xxx)
	{"920", SecurityStock, "北交所股票(2024年起新上市)"},
	// 存量上市公司(原精选层平移)
	{"83", SecurityStock, "北交所股票(原精选层)"},
	{"87", SecurityStock, "北交所股票(原精选层)"},
	{"88", SecurityStock, "北交所股票(2022-2023年上市)"},
	// 其他(极少)
	{"82", SecurityBond, "优先股"},
	{"89", SecurityBond, "可转债"},
}

// ========== 港交所规则(HKSE)==========
var hkseRules = []CodeRule{
	// 指数
	{"HSI", SecurityIndex, "恒生指数"},
	{"HSCEI", SecurityIndex, "国企指数"},
	{"HSCCI", SecurityIndex, "红筹指数"},
	// ETF
	{"028", SecurityETF, "ETF"},
	{"030", SecurityETF, "ETF"},
	{"031", SecurityETF, "ETF"},
	{"090", SecurityETF, "ETF"},
	{"091", SecurityETF, "ETF"},
	// 股票 (5位数字)
	{"08", SecurityStock, "港股(GEM)"},
	{"0", SecurityStock, "港股"},
	// 权证/牛熊证 (5位数字)
	{"1", SecurityBond, "权证"},
	{"2", SecurityBond, "权证"},
	{"4", SecurityBond, "牛熊证"},
	{"5", SecurityBond, "牛熊证"},
	{"6", SecurityBond, "牛熊证"},
}

// matchRule 在规则列表中匹配最长前缀
func matchRule(code string, rules []CodeRule) (SecurityType, string) {
	bestLen := 0
	var matchedType SecurityType
	var matchedDesc string

	for _, rule := range rules {
		if strings.HasPrefix(code, rule.Prefix) {
			if len(rule.Prefix) > bestLen {
				bestLen = len(rule.Prefix)
				matchedType = rule.Type
				matchedDesc = rule.Desc
			}
		}
	}
	if bestLen > 0 {
		return matchedType, matchedDesc
	}
	return SecurityUnknown, ""
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

// 便捷函数
func GetMarket(code string) ExchangeCode {
	mkt, _, _ := DetectSecurity(code)
	return mkt
}

func GetSecurityType(code string) SecurityType {
	_, typ, _ := DetectSecurity(code)
	return typ
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
