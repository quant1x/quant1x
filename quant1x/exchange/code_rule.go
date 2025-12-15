package exchange

import (
	"regexp"
	"strings"

	"gitee.com/quant1x/quant1x/quant1x/std"
)

// SecurityType 表示证券类型（枚举，基于 int8）
type SecurityType int8

const (
	TypeUnknown SecurityType = iota // 未知类型
	TypeStock                       // A股/普通股票
	TypeETF                         // 交易所交易基金
	TypeFund                        // 各类基金（如 LOF、封闭式基金等）
	TypeBond                        // 债券（公司债、可转债、可交换债等）
	TypeBStock                      // B股
	TypeIPO                         // 新股申购/新股
	TypeIndex                       // 指数（如上证指数、深证指数等）
	TypeBlock                       // 板块/板块指数
)

// String 返回 SecurityType 的字符串表示
func (t SecurityType) String() string {
	switch t {
	case TypeStock:
		return "stock"
	case TypeETF:
		return "etf"
	case TypeFund:
		return "fund"
	case TypeBond:
		return "bond"
	case TypeBStock:
		return "b_stock"
	case TypeIPO:
		return "ipo"
	case TypeIndex:
		return "index"
	case TypeBlock:
		return "block"
	default:
		return "unknown"
	}
}

// CodeRule 表示一条证券代码前缀规则
type CodeRule struct {
	Prefix string       // 前缀，如 "600", "920"
	Type   SecurityType // 类型
	Desc   string       // 描述(用于调试或日志)
}

// ========== 全局规则(跨市场，优先匹配)==========
var globalRules = []CodeRule{
	{"880", TypeBlock, "板块指数(通达信)"},
	{"881", TypeBlock, "板块指数(通达信)"},
}

// ========== 上交所规则(SSE)==========
var sseRules = []CodeRule{
	// 指数
	{"000", TypeIndex, "上证指数"},
	// ETF
	{"51", TypeETF, "上交所ETF(510-519)"},
	{"588", TypeETF, "科创板ETF"},
	// 其他基金
	{"50", TypeFund, "LOF/封闭式基金"},
	{"52", TypeFund, "其他基金"},
	// A股
	{"600", TypeStock, "主板A股"},
	{"601", TypeStock, "主板A股"},
	{"603", TypeStock, "主板A股"},
	{"605", TypeStock, "主板A股"},
	// 科创板
	{"688", TypeStock, "科创板"},
	{"689", TypeStock, "科创板CDR"},
	// B股
	{"900", TypeBStock, "B股"},
	// 债券
	{"110", TypeBond, "债券"},
	{"113", TypeBond, "可转债"},
	{"118", TypeBond, "可交换债"},
	{"120", TypeBond, "公司债"},
	{"123", TypeBond, "可转债"},
	{"127", TypeBond, "可转债"},
	{"128", TypeBond, "可转债"},
	// 新股申购
	{"730", TypeIPO, "新股申购"},
	{"780", TypeIPO, "新股申购"},
}

// ========== 深交所规则(SZSE)==========
var szseRules = []CodeRule{
	// 指数
	{"399", TypeIndex, "深证指数"},
	// ETF
	{"159", TypeETF, "深交所ETF"},
	// 其他基金
	{"150", TypeFund, "LOF"},
	{"160", TypeFund, "LOF"},
	{"161", TypeFund, "LOF"},
	{"162", TypeFund, "LOF"},
	{"163", TypeFund, "LOF"},
	{"164", TypeFund, "LOF"},
	{"167", TypeFund, "LOF"},
	{"168", TypeFund, "LOF"},
	{"169", TypeFund, "LOF"},
	{"184", TypeFund, "封闭式基金"},
	// A股(主板 + 创业板)
	{"000", TypeStock, "主板A股"},
	{"001", TypeStock, "主板A股"},
	{"002", TypeStock, "主板A股"},
	{"003", TypeStock, "主板A股"},
	{"300", TypeStock, "创业板"},
	{"301", TypeStock, "创业板"},
	// B股
	{"200", TypeBStock, "B股"},
	// 债券
	{"110", TypeBond, "可转债"},
	{"111", TypeBond, "可转债"},
	{"118", TypeBond, "可交换债"},
	{"123", TypeBond, "可转债"},
	{"127", TypeBond, "可转债"},
	{"128", TypeBond, "可转债"},
}

// ========== 北交所规则(BJSE)==========
var bjseRules = []CodeRule{
	// 新上市公司(2024年起使用 920xxx)
	{"920", TypeStock, "北交所股票(2024年起新上市)"},
	// 存量上市公司(原精选层平移)
	{"83", TypeStock, "北交所股票(原精选层)"},
	{"87", TypeStock, "北交所股票(原精选层)"},
	{"88", TypeStock, "北交所股票(2022-2023年上市)"},
	// 其他(极少)
	{"82", TypeBond, "优先股"},
	{"89", TypeBond, "可转债"},
}

// ========== 港交所规则(HKSE)==========
var hkseRules = []CodeRule{
	// 指数
	{"HSI", TypeIndex, "恒生指数"},
	{"HSCEI", TypeIndex, "国企指数"},
	{"HSCCI", TypeIndex, "红筹指数"},
	// ETF
	{"028", TypeETF, "ETF"},
	{"030", TypeETF, "ETF"},
	{"031", TypeETF, "ETF"},
	{"090", TypeETF, "ETF"},
	{"091", TypeETF, "ETF"},
	// 股票 (5位数字)
	{"0", TypeStock, "港股"},
	{"08", TypeStock, "港股(GEM)"},
	// 权证/牛熊证 (5位数字)
	{"1", TypeBond, "权证"},
	{"2", TypeBond, "权证"},
	{"4", TypeBond, "牛熊证"},
	{"5", TypeBond, "牛熊证"},
	{"6", TypeBond, "牛熊证"},
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
	return TypeUnknown, ""
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
				return "", TypeUnknown, "无法识别市场"
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
		return "", TypeUnknown, "代码格式错误(应为5或6位数字)"
	}

	// 4. 全局规则优先(如板块指数)
	if typ, desc := matchRule(code, globalRules); typ != TypeUnknown {
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
		return market, TypeUnknown, "不支持的市场"
	}

	if typ, desc := matchRule(code, rules); typ != TypeUnknown {
		return market, typ, desc
	}

	return market, TypeUnknown, "未匹配到规则"
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

// Detect 解析证券代码并返回 `SecurityCode`（包含 Market 和 Symbol）
// 要求：对输入仅做一次检测（仅调用一次 regexp FindStringSubmatch），然后基于规则表解析类型。
// detectRegexp 仅用于一次性提取输入的组成部分：
// - 前缀形式: <flag><code> (例如 sh600600, hk00700)
// - 后缀形式: <code>.<flag> (例如 600600.sh, 00700.hk)
// - 纯形式: <code> (例如 600600 或 00700)
// - 字母后缀: <alpha>.<flag> (例如 APPL.US)
func Detect(input string) SecurityCode {
	raw := strings.TrimSpace(input)
	if raw == "" {
		return SecurityCode{Market: ExchangeIdShangHai, Symbol: "", Type: TypeUnknown}
	}
	pureCode := strings.ToLower(raw)
	symbol := ""                    // 纯代码部分
	exchangeCode := ExchangeUnknown // 默认未知市场
	exchangeId := ExchangeIdUnknown // 默认未知市场
	typ := TypeUnknown              // 默认未知类型
	if std.StartsWith(pureCode, AllExchangeCodes) {
		// 前缀形式: sh600000, hk00700, usappl
		symbol = pureCode[2:]
		exchangeCode = ExchangeCode(pureCode[:2])
		exchangeId, _ = exchangeCode.Id()
	} else if std.EndsWith(pureCode, AllExchangeCodes) && len(pureCode) >= 3 && pureCode[len(pureCode)-3] == '.' {
		// 后缀形式: 600000.sh, 00700.hk, APPL.us
		suffixLength := 3 // 包含点号
		symbol = pureCode[:len(pureCode)-suffixLength]
		exchangeCode = ExchangeCode(pureCode[len(pureCode)-2:])
		exchangeId, _ = exchangeCode.Id()
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
				typ = TypeStock
			} else {
				// 未识别的 4 位代码，标记为未知
				exchangeCode = ExchangeUnknown
				exchangeId = ExchangeIdUnknown
				symbol = ""
				typ = TypeUnknown
			}
		case 5: // 港股代码，5位数字
			exchangeCode = ExchangeHK
			exchangeId = ExchangeIdHongKong
			symbol = pureCode
		case 6: // A股代码，6位数
			// 1. 全局规则优先(如板块指数)
			if typ_, desc := matchRule(pureCode, globalRules); typ_ != TypeUnknown {
				_ = desc
				symbol = pureCode
				exchangeCode = ExchangeSSE
				exchangeId = ExchangeIdShangHai
				return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ_}
			}
			// 2. 按市场匹配规则
			// 2.1 深交所
			if typ_, desc := matchRule(pureCode, szseRules); typ_ != TypeUnknown {
				_ = desc
				symbol = pureCode
				exchangeCode = ExchangeSZSE
				exchangeId = ExchangeIdShenZhen
				return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ_}
			}
			// 2.2 北交所
			if typ_, desc := matchRule(pureCode, bjseRules); typ_ != TypeUnknown {
				_ = desc
				symbol = pureCode
				exchangeCode = ExchangeBJSE
				exchangeId = ExchangeIdBeiJing
				return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ_}
			}
			// 2.3 上交所
			if typ_, desc := matchRule(pureCode, sseRules); typ_ != TypeUnknown {
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
		return SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: TypeUnknown}
	}

	if typ == TypeUnknown {
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
			typ = TypeStock // 美股默认股票
			return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ}
		default:
			return SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: TypeUnknown}
		}
		if typ_, _ := matchRule(symbol, rules); typ_ != TypeUnknown {
			typ = typ_
			return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ}
		} else {
			return SecurityCode{Market: ExchangeIdUnknown, Symbol: "", Type: TypeUnknown}
		}
	} else {
		// 已识别类型，直接返回, 不进行规则匹配
		// 适用于美股等市场
		return SecurityCode{Market: exchangeId, Symbol: symbol, Type: typ}
	}
}
