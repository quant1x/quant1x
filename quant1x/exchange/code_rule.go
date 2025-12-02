package exchange

import (
	"regexp"
	"strings"
)

// Market 表示交易所
type Market string

const (
	MarketSSE  Market = "sh" // 上海证券交易所
	MarketSZSE Market = "sz" // 深圳证券交易所
	MarketBJSE Market = "bj" // 北京证券交易所
	MarketHK   Market = "hk" // 香港证券交易所
)

// SecurityType 表示证券类型
type SecurityType string

const (
	TypeStock   SecurityType = "stock"
	TypeETF     SecurityType = "etf"
	TypeFund    SecurityType = "fund" // LOF、封闭式基金等
	TypeBond    SecurityType = "bond"
	TypeBStock  SecurityType = "b_stock"
	TypeIPO     SecurityType = "ipo"
	TypeIndex   SecurityType = "index"
	TypeBlock   SecurityType = "block"
	TypeUnknown SecurityType = "unknown"
)

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
func DetectSecurity(input string) (Market, SecurityType, string) {
	// 标准化：去除空格、点，转小写
	s := strings.ToLower(strings.ReplaceAll(strings.TrimSpace(input), ".", ""))

	var market Market
	var code string

	// 1. 尝试解析显式市场标识(前缀或后缀)
	if len(s) >= 7 {
		if strings.HasPrefix(s, "sh") || strings.HasPrefix(s, "sz") || strings.HasPrefix(s, "bj") || strings.HasPrefix(s, "hk") {
			market = Market(s[:2])
			code = s[2:]
		} else if strings.HasSuffix(s, "sh") || strings.HasSuffix(s, "sz") || strings.HasSuffix(s, "bj") || strings.HasSuffix(s, "hk") {
			market = Market(s[len(s)-2:])
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
				market = MarketSSE
			case strings.HasPrefix(code, "0") || strings.HasPrefix(code, "3") ||
				strings.HasPrefix(code, "1") || strings.HasPrefix(code, "2"):
				market = MarketSZSE
			case strings.HasPrefix(code, "8") || strings.HasPrefix(code, "92"):
				market = MarketBJSE
			default:
				return "", TypeUnknown, "无法识别市场"
			}
		} else if regexp.MustCompile(`^\d{5}$`).MatchString(s) {
			code = s
			market = MarketHK
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
		return MarketSSE, typ, desc // 板块指数归属上证体系
	}

	// 5. 按市场匹配规则
	var rules []CodeRule
	switch market {
	case MarketSSE:
		rules = sseRules
	case MarketSZSE:
		rules = szseRules
	case MarketBJSE:
		rules = bjseRules
	case MarketHK:
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
func GetMarket(code string) Market {
	mkt, _, _ := DetectSecurity(code)
	return mkt
}

func GetSecurityType(code string) SecurityType {
	_, typ, _ := DetectSecurity(code)
	return typ
}
