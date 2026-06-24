// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// HKEX 香港交易所规则, 与 Python market_hkex.py 对齐

package ticker_rules

import (
	"math"

	"github.com/quant1x/quant1x/quant1x/data/meta"
)

// HkexRules HKEX 香港交易所规则
func HkexRules() []CodeRule {
	return []CodeRule{
		// 指数
		{meta.HKEX, NewStrPrefix("HSI"), meta.InstrumentTypeIndex, "恒生指数", "香港交易所"},
		{meta.HKEX, NewStrPrefix("HSCEI"), meta.InstrumentTypeIndex, "国企指数", "香港交易所"},
		{meta.HKEX, NewStrPrefix("HSCCI"), meta.InstrumentTypeIndex, "红筹指数", "香港交易所"},
		{meta.HKEX, NewStrPrefix("HSTECH"), meta.InstrumentTypeIndex, "恒生科技指数", "香港交易所"},

		// 00001-09999, 主板及GEM上市证券
		{meta.HKEX, NewRangePrefix("00001", "02799"), meta.InstrumentTypeStock, "主板", ""},
		{meta.HKEX, NewRangePrefix("02800", "02849"), meta.InstrumentTypeFund, "交易所买卖基金", ""},
		{meta.HKEX, NewRangePrefix("02850", "02899"), meta.InstrumentTypeStock, "主板", ""},
		{meta.HKEX, NewRangePrefix("02900", "02999"), meta.InstrumentTypeTemporaryStock, "主板临时柜台", ""},
		{meta.HKEX, NewRangePrefix("03000", "03199"), meta.InstrumentTypeFund, "交易所买卖基金", ""},
		{meta.HKEX, NewRangePrefix("03200", "03399"), meta.InstrumentTypeStock, "主板", ""},
		{meta.HKEX, NewRangePrefix("03400", "03499"), meta.InstrumentTypeFund, "交易所买卖基金", ""},
		{meta.HKEX, NewRangePrefix("03500", "03599"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		{meta.HKEX, NewRangePrefix("03600", "03999"), meta.InstrumentTypeStock, "主板", ""},
		{meta.HKEX, NewRangePrefix("04000", "04199"), meta.InstrumentTypeBond, "外汇基金债券", "香港金融管理局"},
		{meta.HKEX, NewRangePrefix("04200", "04299"), meta.InstrumentTypeBond, "政府债券", "香港特别行政区"},
		{meta.HKEX, NewRangePrefix("04300", "04329"), meta.InstrumentTypeBond, "债券证券", "仅售予专业投资者"},
		{meta.HKEX, NewRangePrefix("04330", "04399"), meta.InstrumentTypeOther, "NASDQA-AMEX实验计划", ""},
		{meta.HKEX, NewRangePrefix("04400", "04599"), meta.InstrumentTypeBond, "债券证券", "仅售予专业投资者"},
		{meta.HKEX, NewRangePrefix("04600", "04699"), meta.InstrumentTypeStock, "优先股", "仅售予专业投资者"},
		{meta.HKEX, NewRangePrefix("04700", "04799"), meta.InstrumentTypeBond, "债务证券", "售予公众"},
		{meta.HKEX, NewRangePrefix("04800", "04999"), meta.InstrumentTypeWarrant, "权证", "SPAC"},
		{meta.HKEX, NewRangePrefix("05000", "06029"), meta.InstrumentTypeBond, "债券证券", "仅售予专业投资者"},
		{meta.HKEX, NewRangePrefix("06030", "06199"), meta.InstrumentTypeStock, "主板", ""},
		{meta.HKEX, NewRangePrefix("06200", "06299"), meta.InstrumentTypeOther, "香港预讬证券", "香港預託證券"},
		{meta.HKEX, NewRangePrefix("06300", "06399"), meta.InstrumentTypeOther, "证券/预讬证券", "被美国联邦证券法界定为受限制(RS)证券"},
		{meta.HKEX, NewRangePrefix("06400", "06599"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		{meta.HKEX, NewRangePrefix("06600", "06749"), meta.InstrumentTypeStock, "主板", ""},
		{meta.HKEX, NewRangePrefix("06750", "06799"), meta.InstrumentTypeBond, "财政部债券", "中华人民共和国"},
		{meta.HKEX, NewRangePrefix("06800", "06999"), meta.InstrumentTypeStock, "主板", ""},
		{meta.HKEX, NewRangePrefix("07000", "07199"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		{meta.HKEX, NewRangePrefix("07200", "07399"), meta.InstrumentTypeOther, "杠杆及反向产品", ""},
		{meta.HKEX, NewRangePrefix("07400", "07499"), meta.InstrumentTypeStock, "主板", ""},
		{meta.HKEX, NewRangePrefix("07500", "07599"), meta.InstrumentTypeOther, "杠杆及反向产品", ""},
		{meta.HKEX, NewRangePrefix("07600", "07699"), meta.InstrumentTypeStock, "主板", ""},
		{meta.HKEX, NewRangePrefix("07700", "07799"), meta.InstrumentTypeOther, "杠杆及反向产品", ""},
		{meta.HKEX, NewRangePrefix("07800", "07999"), meta.InstrumentTypeOther, "股份", "SPAC"},
		{meta.HKEX, NewRangePrefix("08000", "08550"), meta.InstrumentTypeGemMarket, "GEM证券", ""},
		{meta.HKEX, NewRangePrefix("08551", "08600"), meta.InstrumentTypeTemporaryStock, "GEM临时柜台", ""},
		{meta.HKEX, NewRangePrefix("08601", "08999"), meta.InstrumentTypeGemMarket, "GEM证券", ""},
		{meta.HKEX, NewRangePrefix("09000", "09199"), meta.InstrumentTypeFund, "交易所买卖基金", "美元"},
		{meta.HKEX, NewRangePrefix("09200", "09399"), meta.InstrumentTypeOther, "杠杆及反向产品", "美元"},
		{meta.HKEX, NewRangePrefix("09400", "09499"), meta.InstrumentTypeFund, "交易所买卖基金", "美元"},
		{meta.HKEX, NewRangePrefix("09500", "09599"), meta.InstrumentTypeOther, "杠杆及反向产品", "美元"},
		{meta.HKEX, NewRangePrefix("09600", "09699"), meta.InstrumentTypeStock, "主板", ""},
		{meta.HKEX, NewRangePrefix("09700", "09799"), meta.InstrumentTypeOther, "杠杆及反向产品", "美元"},
		{meta.HKEX, NewRangePrefix("09800", "09849"), meta.InstrumentTypeFund, "交易所买卖基金", "美元"},
		{meta.HKEX, NewRangePrefix("09850", "09999"), meta.InstrumentTypeStock, "主板", ""},

		// 10000-29999, 衍生权证
		{meta.HKEX, NewRangePrefix("10000", "10899"), meta.InstrumentTypeWarrant, "衍生权证", "相关资产在香港以外地区上市的衍生权证, 一篮子权证及非标准型权证"},
		{meta.HKEX, NewRangePrefix("10900", "10999"), meta.InstrumentTypeWarrant, "衍生权证", "相关资产在香港以外地区上市的衍生权证(以美元买卖)"},
		{meta.HKEX, NewRangePrefix("11000", "11999"), meta.InstrumentTypeWarrant, "衍生权证", "相关资产在香港以外地区上市的衍生权证, 一篮子权证及非标准型权证"},
		{meta.HKEX, NewRangePrefix("12000", "29999"), meta.InstrumentTypeWarrant, "衍生权证", ""},

		// 30000-39999, 供沪深股通使用
		{meta.HKEX, NewRangePrefix("30000", "39999"), meta.InstrumentTypeOther, "沪深股通", ""},

		// 40000-40999, 仅售于专业投资者的债务证券
		{meta.HKEX, NewRangePrefix("40000", "40999"), meta.InstrumentTypeBond, "债务证券", "仅售于专业投资者"},
		// 41000-46999, 供日后使用
		{meta.HKEX, NewRangePrefix("41000", "46999"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		// 47000-48999, 供日后使用
		{meta.HKEX, NewRangePrefix("47000", "48999"), meta.InstrumentTypeOther, "界内证", "保留"},
		// 49000-49499, 供日后使用
		{meta.HKEX, NewRangePrefix("49000", "49499"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		// 49500-69999, 牛熊证, callable bull and bear contract
		{meta.HKEX, NewRangePrefix("49500", "49999"), meta.InstrumentTypeOption, "牛熊证", "相关资产在香港以外地区上市"},
		{meta.HKEX, NewRangePrefix("50000", "69999"), meta.InstrumentTypeOption, "牛熊证", ""},
		// 70000-79999, 供沪深股通使用
		{meta.HKEX, NewRangePrefix("70000", "79999"), meta.InstrumentTypeOther, "沪深股通", ""},
		// 80000-89999, 以人民币买卖的产品
		{meta.HKEX, NewRangePrefix("80000", "82799"), meta.InstrumentTypeStock, "主板", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("82800", "82849"), meta.InstrumentTypeFund, "交易所买卖基金", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("82850", "82899"), meta.InstrumentTypeStock, "主板", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("82900", "82999"), meta.InstrumentTypeTemporaryStock, "主板临时柜台", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("83000", "83199"), meta.InstrumentTypeFund, "交易所买卖基金", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("83200", "83399"), meta.InstrumentTypeStock, "主板", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("83400", "83499"), meta.InstrumentTypeFund, "交易所买卖基金", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("83500", "83599"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		{meta.HKEX, NewRangePrefix("83600", "83999"), meta.InstrumentTypeStock, "主板", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("84000", "84299"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		{meta.HKEX, NewRangePrefix("84300", "84329"), meta.InstrumentTypeBond, "债券证券", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("84330", "84399"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		{meta.HKEX, NewRangePrefix("84400", "84599"), meta.InstrumentTypeBond, "债务证券", "仅售于专业投资者"},
		{meta.HKEX, NewRangePrefix("84600", "84699"), meta.InstrumentTypeStock, "优先股", "仅售于专业投资者"},
		{meta.HKEX, NewRangePrefix("84700", "84999"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		{meta.HKEX, NewRangePrefix("85000", "85743"), meta.InstrumentTypeBond, "债务证券", "仅售于专业投资者"},
		{meta.HKEX, NewRangePrefix("85744", "85900"), meta.InstrumentTypeBond, "债务证券", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("85901", "86029"), meta.InstrumentTypeBond, "债务证券", "仅售于专业投资者"},
		{meta.HKEX, NewRangePrefix("86030", "86199"), meta.InstrumentTypeStock, "主板", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("86200", "86299"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		{meta.HKEX, NewRangePrefix("86600", "86799"), meta.InstrumentTypeOther, "中华人民共和国财政部债券/主板证券", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("86800", "86999"), meta.InstrumentTypeStock, "主板", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("87000", "87099"), meta.InstrumentTypeFund, "房地产投资信托基金及交易所买卖基金以外的单位信托/互惠基金", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("87100", "87199"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		{meta.HKEX, NewRangePrefix("87200", "87399"), meta.InstrumentTypeOther, "杠杆及反向产品", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("87400", "87499"), meta.InstrumentTypeStock, "主板", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("87500", "87599"), meta.InstrumentTypeOther, "杠杆及反向产品", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("87600", "87699"), meta.InstrumentTypeStock, "主板", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("87700", "87799"), meta.InstrumentTypeOther, "杠杆及反向产品", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("87800", "88999"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		{meta.HKEX, NewRangePrefix("89000", "89099"), meta.InstrumentTypeBond, "中华人民共和国财政部债券", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("89100", "89199"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		{meta.HKEX, NewRangePrefix("89200", "89599"), meta.InstrumentTypeWarrant, "衍生权证", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("89600", "89699"), meta.InstrumentTypeStock, "主板", "以人民币买卖"},
		{meta.HKEX, NewRangePrefix("89700", "89849"), meta.InstrumentTypeOther, "供日后使用", "保留"},
		{meta.HKEX, NewRangePrefix("89850", "89999"), meta.InstrumentTypeStock, "主板", "以人民币买卖"},

		// 90000-99999, 供沪深股通使用
		{meta.HKEX, NewStrPrefix("9"), meta.InstrumentTypeOther, "沪深股通", ""},
	}
}

// priceRange 价格区间: [Low, High)
type priceRange struct {
	Low  float64
	High float64
}

// hkPriceRanges 港股交易最小变动单位(最小价位表) 价格区间
var hkPriceRanges = []priceRange{
	{0.01, 0.25},
	{0.25, 0.50},
	{0.50, 10.00},
	{10.00, 20.00},
	{20.00, 100.00},
	{100.00, 200.00},
	{200.00, 500.00},
	{500.00, 1000.00},
	{1000.00, 2000.00},
	{2000.00, 5000.00},
	{5000.00, 9995.00},
}

// hkMinPriceChanges 最小变动价位, 与 priceRanges 一一对应
var hkMinPriceChanges = []float64{
	0.001,
	0.005,
	0.010,
	0.020,
	0.050,
	0.100,
	0.200,
	0.500,
	1.000,
	2.000,
	5.000,
}

// GetMinPriceChange 根据给定的股价, 返回对应的最小变动价位
// 参数 price: 股票价格
// 返回 minChange: 最小变动价位, ok: 是否在有效范围内
func GetMinPriceChange(price float64) (minChange float64, ok bool) {
	for i, pr := range hkPriceRanges {
		if price >= pr.Low && price < pr.High {
			return hkMinPriceChanges[i], true
		}
	}
	return 0, false
}

// RoundToTick 将价格四舍五入到最小变动单位的整数倍
// 参数 price: 原始价格
// 返回 rounded: 调整后的价格, ok: 是否在有效范围内
func RoundToTick(price float64) (rounded float64, ok bool) {
	minChange, found := GetMinPriceChange(price)
	if !found {
		return 0, false
	}
	ticks := math.Round(price / minChange)
	return ticks * minChange, true
}
