// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// USA 美国证券交易所规则, 与 Python market_usa.py 对齐

package ticker_rules

import (
	"strings"

	"github.com/quant1x/quant1x/quant1x/data/meta"
)

// UsaRules USA 美国证券交易所规则
func UsaRules() []CodeRule {
	return []CodeRule{
		{meta.OFFSHORE, NewStrPrefix("IXIC"), meta.InstrumentTypeIndex, "指数", "纳斯达克指数"},
		{meta.OFFSHORE, NewStrPrefix("DAX"), meta.InstrumentTypeIndex, "指数", "德国DAX指数"},
		{meta.EXTENDED, NewStrPrefix("US"), meta.InstrumentTypeSector, "指数", "美国板块指数"},
		{meta.USA, NewStrPrefix(""), meta.InstrumentTypeStock, "挂牌公司普通股", ""},
	}
}

// tickerToCodeMapping 美国ticker到协议代码映射
var tickerToCodeMapping = map[string]string{
	"IXIC": "A_IXIC", // 纳斯达克指数
	"DAX":  "B_DAX",  // 德国DAX指数
}

// codeToTickerMapping 协议代码到美国ticker映射(反向)
var codeToTickerMapping = map[string]string{}

func init() {
	for ticker, code := range tickerToCodeMapping {
		codeToTickerMapping[code] = ticker
	}
}

// UsaTickerToCode 将美国股票代码转换为行情标准的代码
// 参数 ticker: 输入的美国股票代码
// 返回: 转换后的标准符号, 如果未找到映射则返回原代码
func UsaTickerToCode(ticker string) string {
	ticker = strings.ToUpper(ticker)
	if code, ok := tickerToCodeMapping[ticker]; ok {
		return code
	}
	return ticker
}

// UsaCodeToTicker 将美国股票协议代码转换为对应的股票代码
// 参数 code: 输入的美国股票协议代码
// 返回: 对应的股票代码, 如果未找到映射则返回空字符串
func UsaCodeToTicker(code string) string {
	if ticker, ok := codeToTickerMapping[code]; ok {
		return ticker
	}
	return ""
}
