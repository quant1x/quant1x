package markets

import (
	"fmt"
	"strings"
	_ "unsafe"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
)

var (
	// AShareIndexList A股指数列表
	AShareIndexList = []exchange.InstrumentInfo{
		{Exchange: exchange.ExchangeSSE, Ticker: "000001", Type: exchange.SecurityTypeIndex},  // 上证指数
		{Exchange: exchange.ExchangeSSE, Ticker: "000002", Type: exchange.SecurityTypeIndex},  // 上证A股指数
		{Exchange: exchange.ExchangeSSE, Ticker: "000300", Type: exchange.SecurityTypeIndex},  // 沪深300指数
		{Exchange: exchange.ExchangeSSE, Ticker: "000688", Type: exchange.SecurityTypeIndex},  // 科创50指数
		{Exchange: exchange.ExchangeSSE, Ticker: "000905", Type: exchange.SecurityTypeIndex},  // 中证500指数
		{Exchange: exchange.ExchangeSZSE, Ticker: "399001", Type: exchange.SecurityTypeIndex}, // 深证成份指数
		{Exchange: exchange.ExchangeSZSE, Ticker: "399006", Type: exchange.SecurityTypeIndex}, // 创业板指
		{Exchange: exchange.ExchangeSZSE, Ticker: "399107", Type: exchange.SecurityTypeIndex}, // 深证A指
		{Exchange: exchange.ExchangeBSE, Ticker: "899050", Type: exchange.SecurityTypeIndex},  // 北证50指数
		{Exchange: exchange.ExchangeSSE, Ticker: "880005", Type: exchange.SecurityTypeBlock},  // 通达信板块-涨跌家数
		{Exchange: exchange.ExchangeSSE, Ticker: "510050", Type: exchange.SecurityTypeETF},    // 上证50ETF
		{Exchange: exchange.ExchangeSSE, Ticker: "510300", Type: exchange.SecurityTypeETF},    // 沪深300ETF
		{Exchange: exchange.ExchangeSSE, Ticker: "510900", Type: exchange.SecurityTypeETF},    // H股ETF
	}

	// 需要检查的关键字列表
	ignoredKeywords = []string{"ST", "退", "摘牌"}
)

// IsNeedIgnore 证券代码是否需要忽略, 这是一个不参与数据和策略处理的开关
func IsNeedIgnore(code string) bool {
	p := GetSecurityInfo(code)
	if p == nil {
		// 没找到直接忽略
		return true
	}

	// 转换名称为大写
	upperName := strings.ToUpper(p.Name)

	// 检查是否存在任意关键字
	for _, keyword := range ignoredKeywords {
		if strings.Contains(upperName, keyword) {
			return true
		}
	}
	return false
}

// GetStockCodeList 返回所有A股股票代码列表，包括：
//   - 上海证券交易所（600000-609999）
//   - 科创板（688000-689999）
//   - 深圳主板（000000-000999）
//   - 中小板（001000-009999）
//   - 创业板（300000-309999）
//   - 北交所（920000-920999）
//
// 返回的列表会过滤掉需要忽略的股票代码
func GetStockCodeList() []exchange.InstrumentInfo {
	var allCodes []exchange.InstrumentInfo

	// 上海证券交易所 (sh600000-sh609999)
	for i := 600000; i <= 609999; i++ {
		sc := exchange.InstrumentInfo{
			Exchange: exchange.ExchangeSSE,
			Ticker:   fmt.Sprintf("%06d", i),
			Type:     exchange.SecurityTypeStock,
		}
		if !IsNeedIgnore(sc.Symbol()) {
			allCodes = append(allCodes, sc)

		}
	}

	// 科创板 (sh688000-sh689999)
	for i := 688000; i <= 689999; i++ {
		sc := exchange.InstrumentInfo{
			Exchange: exchange.ExchangeSSE,
			Ticker:   fmt.Sprintf("%06d", i),
			Type:     exchange.SecurityTypeStock,
		}
		if !IsNeedIgnore(sc.Symbol()) {
			allCodes = append(allCodes, sc)
		}
	}

	// 深圳主板 (sz000000-sz000999)
	for i := 0; i <= 999; i++ {
		sc := exchange.InstrumentInfo{
			Exchange: exchange.ExchangeSZSE,
			Ticker:   fmt.Sprintf("%06d", i),
			Type:     exchange.SecurityTypeStock,
		}
		if !IsNeedIgnore(sc.Symbol()) {
			allCodes = append(allCodes, sc)
		}
	}

	// 中小板 (sz001000-sz009999)
	for i := 1000; i <= 9999; i++ {
		sc := exchange.InstrumentInfo{
			Exchange: exchange.ExchangeSZSE,
			Ticker:   fmt.Sprintf("%06d", i),
			Type:     exchange.SecurityTypeStock,
		}
		if !IsNeedIgnore(sc.Symbol()) {
			allCodes = append(allCodes, sc)
		}
	}

	// 创业板 (sz300000-sz300999)
	for i := 300000; i <= 309999; i++ {
		sc := exchange.InstrumentInfo{
			Exchange: exchange.ExchangeSZSE,
			Ticker:   fmt.Sprintf("%06d", i),
			Type:     exchange.SecurityTypeStock,
		}
		if !IsNeedIgnore(sc.Symbol()) {
			allCodes = append(allCodes, sc)
		}
	}

	// 北交所 (bj920000-bj920999)
	for i := 920000; i <= 920999; i++ {
		sc := exchange.InstrumentInfo{
			Exchange: exchange.ExchangeBSE,
			Ticker:   fmt.Sprintf("%06d", i),
			Type:     exchange.SecurityTypeStock,
		}
		if !IsNeedIgnore(sc.Symbol()) {
			allCodes = append(allCodes, sc)
		}
	}

	return allCodes
}

// GetCodeList 加载全部指数、板块和个股的代码
func GetCodeList() []exchange.InstrumentInfo {
	var list []exchange.InstrumentInfo
	// 1. 指数
	list = append(list, AShareIndexList...)

	// 2. 板块
	sectors := BlockList()
	for _, v := range sectors {
		symbol_ := v.Code[2:]
		sc := exchange.InstrumentInfo{
			Exchange: exchange.ExchangeSSE,
			Type:     exchange.SecurityTypeBlock,
			Ticker:   symbol_,
		}
		list = append(list, sc)
	}

	// 3. 个股, 包括场内开放式ETF基金
	stockCodeList := GetStockCodeList()
	list = append(list, stockCodeList...)

	return list
}
