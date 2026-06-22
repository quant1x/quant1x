// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// SZSE 深圳证券交易所规则, 与 Python market_szse.py 对齐

package ticker_rules

import "github.com/quant1x/quant1x/quant1x/data/meta"

// SzseRules SZSE 深圳证券交易所规则
func SzseRules() []CodeRule {
	return []CodeRule{
		{meta.SZSE, NewStrPrefix("395"), meta.InstrumentTypeIndex, "成交量统计指数", ""},
		{meta.SZSE, NewStrPrefix("399"), meta.InstrumentTypeIndex, "深证指数", ""},

		{meta.SZSE, NewStrPrefix("000"), meta.InstrumentTypeStock, "主板A股", ""},
		{meta.SZSE, NewStrPrefix("001"), meta.InstrumentTypeStock, "主板A股", ""},
		{meta.SZSE, NewStrPrefix("002"), meta.InstrumentTypeStock, "主板A股", ""},
		{meta.SZSE, NewStrPrefix("003"), meta.InstrumentTypeStock, "主板A股", ""},
		{meta.SZSE, NewStrPrefix("030"), meta.InstrumentTypeWarrant, "权证", ""},
		{meta.SZSE, NewStrPrefix("031"), meta.InstrumentTypeWarrant, "权证", ""},
		{meta.SZSE, NewStrPrefix("032"), meta.InstrumentTypeWarrant, "权证", ""},
		{meta.SZSE, NewStrPrefix("036"), meta.InstrumentTypeWarrant, "创业板股权激励计划涉及的员工认股权", ""},
		{meta.SZSE, NewStrPrefix("0370"), meta.InstrumentTypeWarrant, "主板A股股权激励计划涉及的员工认股权", ""},
		{meta.SZSE, NewStrPrefix("0371"), meta.InstrumentTypeWarrant, "主板A股股权激励计划涉及的员工认股权", ""},
		{meta.SZSE, NewStrPrefix("0372"), meta.InstrumentTypeWarrant, "创业板股权激励计划审计的员工认股权", ""},
		{meta.SZSE, NewStrPrefix("0373"), meta.InstrumentTypeWarrant, "主板A股股权激励计划涉及的员工认股权", ""},
		{meta.SZSE, NewStrPrefix("0374"), meta.InstrumentTypeWarrant, "主板A股股权激励计划涉及的员工认股权", ""},
		{meta.SZSE, NewStrPrefix("0375"), meta.InstrumentTypeWarrant, "中小企业板股权激励计划涉及的员工认股权", ""},
		{meta.SZSE, NewStrPrefix("0376"), meta.InstrumentTypeWarrant, "中小企业板股权激励计划涉及的员工认股权", ""},
		{meta.SZSE, NewStrPrefix("0377"), meta.InstrumentTypeWarrant, "中小企业板股权激励计划涉及的员工认股权", ""},
		{meta.SZSE, NewStrPrefix("0378"), meta.InstrumentTypeWarrant, "中小企业板股权激励计划涉及的员工认股权", ""},
		{meta.SZSE, NewStrPrefix("0379"), meta.InstrumentTypeWarrant, "中小企业板股权激励计划涉及的员工认股权", ""},
		{meta.SZSE, NewStrPrefix("038"), meta.InstrumentTypeWarrant, "主板A股及中小企业股票认沽权证", ""},
		{meta.SZSE, NewStrPrefix("039"), meta.InstrumentTypeWarrant, "主板A股及中小企业股票认沽权证", ""},
		{meta.SZSE, NewStrPrefix("070"), meta.InstrumentTypeWarrant, "主板A股增发/可转债申购", ""},
		{meta.SZSE, NewStrPrefix("071"), meta.InstrumentTypeWarrant, "主板A股增发/可转债申购", ""},
		{meta.SZSE, NewStrPrefix("072"), meta.InstrumentTypeWarrant, "中小企业板增发/可转债申购", ""},
		{meta.SZSE, NewStrPrefix("073"), meta.InstrumentTypeWarrant, "中小企业板增发/可转债申购", ""},
		{meta.SZSE, NewStrPrefix("074"), meta.InstrumentTypeWarrant, "中小企业板增发/可转债申购", ""},
		{meta.SZSE, NewStrPrefix("080"), meta.InstrumentTypeWarrant, "A股配股", ""},

		{meta.SZSE, NewStrPrefix("0"), meta.InstrumentTypeStock, "股票", ""},

		{meta.SZSE, NewStrPrefix("10"), meta.InstrumentTypeBond, "国债", ""},
		{meta.SZSE, NewStrPrefix("11"), meta.InstrumentTypeBond, "企业债", ""},
		{meta.SZSE, NewStrPrefix("120"), meta.InstrumentTypeBond, "企业债券", ""},
		{meta.SZSE, NewStrPrefix("123"), meta.InstrumentTypeBond, "可转债", ""},
		{meta.SZSE, NewStrPrefix("127"), meta.InstrumentTypeBond, "可转债", ""},
		{meta.SZSE, NewStrPrefix("128"), meta.InstrumentTypeBond, "可转债", ""},
		{meta.SZSE, NewStrPrefix("13"), meta.InstrumentTypeBond, "债券回购", ""},
		{meta.SZSE, NewStrPrefix("159"), meta.InstrumentTypeETF, "深交所ETF", ""},
		{meta.SZSE, NewStrPrefix("15"), meta.InstrumentTypeFund, "ETF", ""},
		{meta.SZSE, NewStrPrefix("16"), meta.InstrumentTypeFund, "LOF", ""},
		{meta.SZSE, NewStrPrefix("17"), meta.InstrumentTypeFund, "传统投资基金", ""},
		{meta.SZSE, NewStrPrefix("184"), meta.InstrumentTypeFund, "封闭式基金", ""},
		{meta.SZSE, NewStrPrefix("18"), meta.InstrumentTypeFund, "封闭式基金", ""},

		{meta.SZSE, NewStrPrefix("1"), meta.InstrumentTypeBond, "债券", ""},

		{meta.SZSE, NewStrPrefix("200"), meta.InstrumentTypeBStock, "B股", ""},
		{meta.SZSE, NewStrPrefix("238"), meta.InstrumentTypeOther, "B股现金选择权", ""},
		{meta.SZSE, NewStrPrefix("28"), meta.InstrumentTypeOther, "B股配股优先权", ""},

		{meta.SZSE, NewStrPrefix("2"), meta.InstrumentTypeBStock, "B股", ""},

		{meta.SZSE, NewStrPrefix("300"), meta.InstrumentTypeChiNextMarket, "创业板", ""},
		{meta.SZSE, NewStrPrefix("301"), meta.InstrumentTypeChiNextMarket, "创业板注册制", ""},
		{meta.SZSE, NewStrPrefix("30"), meta.InstrumentTypeChiNextMarket, "创业板", ""},
		{meta.SZSE, NewStrPrefix("36"), meta.InstrumentTypeOther, "投票", ""},
		{meta.SZSE, NewStrPrefix("37"), meta.InstrumentTypeOther, "增发/可转债申购", ""},
		{meta.SZSE, NewStrPrefix("38"), meta.InstrumentTypeOther, "配股/可转债优先权", ""},

		{meta.SZSE, NewStrPrefix("50"), meta.InstrumentTypeBond, "资产支持证券ABS", ""},
		{meta.SZSE, NewStrPrefix("56"), meta.InstrumentTypeBond, "资产支持证券ABS", ""},

		{meta.SZSE, NewStrPrefix("5"), meta.InstrumentTypeBond, "资产支持证券ABS", ""},

		{meta.SZSE, NewStrPrefix("700"), meta.InstrumentTypeWarrant, "B股增发", ""},
		{meta.SZSE, NewStrPrefix("730"), meta.InstrumentTypeWarrant, "跨市场申购", ""},
	}
}
