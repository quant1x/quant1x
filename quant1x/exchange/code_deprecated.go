package exchange

import (
	"fmt"
	"regexp"
	"strings"
)

// DetectMarket 根据证券代码检测所属市场
//
// 参数:
//
//	symbol: 证券代码字符串
//
// 返回值:
//
//	ExchangeId: 交易所ID
//	string: 市场代码
//	string: 纯数字证券代码
//	error: 错误信息
func DetectMarket(symbol string) (ExchangeId, string, string, error) {
	trimmed := strings.TrimSpace(symbol)
	if trimmed == "" {
		return ExchangeIdShangHai, string(ExchangeSSE), "", fmt.Errorf("empty security code")
	}

	lowered := strings.ToLower(trimmed)
	marketCode, pureCode := splitMarketFlag(lowered)
	pureCode = sanitizeDigits(pureCode)

	if marketCode == "" {
		if len(pureCode) == 6 && digitsRegexp.MatchString(pureCode) {
			market := GetMarket(pureCode)
			marketCode = string(market)
		} else {
			marketCode = string(ExchangeSSE)
		}
	}

	switch marketCode {
	case string(ExchangeSZSE):
		return ExchangeIdShenZhen, marketCode, pureCode, nil
	case string(ExchangeBJSE):
		return ExchangeIdBeiJing, marketCode, pureCode, nil
	case "hk":
		return ExchangeIdHongKong, marketCode, pureCode, nil
	case "us":
		return ExchangeIdUSA, marketCode, pureCode, nil
	default:
		return ExchangeIdShangHai, string(ExchangeSSE), pureCode, nil
	}
}

// AssertIndexByMarketAndCode 根据交易所ID和代码判断是否为指数代码
//
// 参数:
//
//	m: 交易所ID
//	code: 证券代码
//
// 返回值:
//
//	bool: 如果是该交易所的指数代码返回true，否则返回false
func AssertIndexByMarketAndCode(m ExchangeId, code string) bool {
	symbol := strings.TrimSpace(code)
	switch m {
	case ExchangeIdShangHai:
		return strings.HasPrefix(symbol, "000") || strings.HasPrefix(symbol, "880") || strings.HasPrefix(symbol, "881")
	case ExchangeIdShenZhen:
		return strings.HasPrefix(symbol, "399")
	case ExchangeIdBeiJing:
		return strings.HasPrefix(symbol, "899")
	default:
		return false
	}
}

var digitsRegexp = regexp.MustCompile(`^\d+$`)

func splitMarketFlag(symbol string) (string, string) {
	for _, flag := range []string{"sh", "sz", "bj", "hk", "us"} {
		if strings.HasPrefix(symbol, flag) {
			return flag, symbol[len(flag):]
		}
		if strings.HasSuffix(symbol, flag) {
			return flag, symbol[:len(symbol)-len(flag)]
		}
	}
	return "", symbol
}

func sanitizeDigits(s string) string {
	replaced := strings.ReplaceAll(s, ".", "")
	replaced = strings.ReplaceAll(replaced, "-", "")
	return replaced
}

// CorrectSecurityCode 将输入的证券代码转换为标准格式，返回市场标志+证券代码的组合字符串
func CorrectSecurityCode(input string) string {
	_, marketFlag, symbol, err := DetectMarket(input)
	if err != nil {
		panic(err)
	}
	return fmt.Sprintf("%s%s", marketFlag, symbol)
}

// GetSecurityCode 根据市场和证券代码生成完整的证券代码字符串
//
// 参数:
//
//	market: 交易所ID (ExchangeIdUSA/ExchangeIdHongKong/ExchangeIdBeiJing/ExchangeIdShenZhen等)
//	symbol: 证券代码
//
// 返回值:
//
//	格式化后的完整证券代码字符串，包含交易所前缀
func GetSecurityCode(market ExchangeId, symbol string) string {
	switch market {
	case ExchangeIdUSA:
		return string(ExchangeUS) + symbol
	case ExchangeIdHongKong:
		if len(symbol) >= 5 {
			return string(ExchangeHK) + symbol[:5]
		}
		return string(ExchangeHK) + symbol
	case ExchangeIdBeiJing:
		if len(symbol) >= 6 {
			return string(ExchangeBJSE) + symbol[:6]
		}
		return string(ExchangeBJSE) + symbol
	case ExchangeIdShenZhen:
		if len(symbol) >= 6 {
			return string(ExchangeSZSE) + symbol[:6]
		}
		return string(ExchangeSZSE) + symbol
	default:
		if len(symbol) >= 6 {
			return string(ExchangeSSE) + symbol[:6]
		}
		return string(ExchangeSSE) + symbol
	}
}

// GetMarketId 根据证券代码获取市场ID
//
// 参数:
//
//	symbol - 证券代码字符串
//
// 返回值:
//
//	ExchangeId - 对应的市场ID
//
// 注意:
//
//	如果无法识别市场会触发panic
func GetMarketId(symbol string) ExchangeId {
	mid, _, _, err := DetectMarket(symbol)
	if err != nil {
		panic(err)
	}
	return mid
}

// GetMarketFlag 根据市场ID返回市场标识
func GetMarketFlag(m ExchangeId) string {
	return m.String()
}

// AssertIndexBySecurityCode 判断是否为指数代码（通过完整证券代码）
func AssertIndexBySecurityCode(securityCode string) bool {
	mid, _, code, err := DetectMarket(securityCode)
	if err != nil {
		panic(err)
	}
	return AssertIndexByMarketAndCode(mid, code)
}

// AssertBlockBySecurityCode 判断并修正板块代码（会修改传入字符串）
func AssertBlockBySecurityCode(securityCode *string) bool {
	if securityCode == nil || *securityCode == "" {
		return false
	}
	mid, flag, code, err := DetectMarket(*securityCode)
	if err != nil {
		panic(err)
	}
	if mid != ExchangeIdShangHai {
		return false
	}
	if strings.HasPrefix(code, "880") || strings.HasPrefix(code, "881") {
		*securityCode = fmt.Sprintf("%s%s", flag, code)
		return true
	}
	return false
}

// AssertETFByMarketAndCode 判断是否为ETF（通过市场ID和纯代码）
func AssertETFByMarketAndCode(mid ExchangeId, symbol string) bool {
	if mid == ExchangeIdShangHai && strings.HasPrefix(symbol, "510") {
		return true
	}
	if mid == ExchangeIdShenZhen && strings.HasPrefix(symbol, "159") {
		return true
	}
	return false
}

// AssertStockByMarketAndCode 判断是否为个股（通过市场ID和纯代码）
func AssertStockByMarketAndCode(mid ExchangeId, symbol string) bool {
	if mid == ExchangeIdShangHai && (strings.HasPrefix(symbol, "60") || strings.HasPrefix(symbol, "68") || strings.HasPrefix(symbol, "510")) {
		return true
	}
	if mid == ExchangeIdShenZhen && (strings.HasPrefix(symbol, "00") || strings.HasPrefix(symbol, "30")) {
		return true
	}
	if mid == ExchangeIdBeiJing && (strings.HasPrefix(symbol, "40") || strings.HasPrefix(symbol, "43") || strings.HasPrefix(symbol, "83") || strings.HasPrefix(symbol, "87") || strings.HasPrefix(symbol, "88") || strings.HasPrefix(symbol, "420") || strings.HasPrefix(symbol, "820") || strings.HasPrefix(symbol, "920")) {
		return true
	}
	return false
}

// AssertStockBySecurityCode 判断是否为个股（通过完整证券代码）
func AssertStockBySecurityCode(securityCode string) bool {
	mid, _, code, err := DetectMarket(securityCode)
	if err != nil {
		panic(err)
	}
	return AssertStockByMarketAndCode(mid, code)
}

// AssertCode 判断证券代码类型
func AssertCode(securityCode string) SecurityType {
	mid, _, code, err := DetectMarket(securityCode)
	if err != nil {
		panic(err)
	}
	if mid == ExchangeIdShangHai {
		if strings.HasPrefix(code, "880") || strings.HasPrefix(code, "881") {
			return SecurityBlock
		}
		if strings.HasPrefix(code, "000") {
			return SecurityIndex
		}
		if strings.HasPrefix(code, "5") {
			return SecurityETF
		}
	}
	if mid == ExchangeIdShenZhen {
		if strings.HasPrefix(code, "399") {
			return SecurityIndex
		}
		if strings.HasPrefix(code, "159") {
			return SecurityETF
		}
	}
	if mid == ExchangeIdBeiJing && strings.HasPrefix(code, "899") {
		return SecurityIndex
	}
	return SecurityStock
}

// CheckIndexAndStock 检查指数和个股
func CheckIndexAndStock(securityCode string) bool {
	if AssertIndexBySecurityCode(securityCode) {
		return true
	}
	if AssertStockBySecurityCode(securityCode) {
		return true
	}
	return false
}
