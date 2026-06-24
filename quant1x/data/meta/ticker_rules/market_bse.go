// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// BSE 北京证券交易所规则, 与 Python market_bse.py 对齐

package ticker_rules

import "github.com/quant1x/quant1x/quant1x/data/meta"

// BseRules BSE 北京证券交易所规则
func BseRules() []CodeRule {
	return []CodeRule{
		{meta.BSE, NewStrPrefix("899"), meta.InstrumentTypeIndex, "指数", "证券指数首三位代码为899"},

		{meta.BSE, NewStrPrefix("920"), meta.InstrumentTypeStock, "北交所新上市", "2024-04-22 起新上市使用920号段; 已上市公司继续沿用原代码直到统一切换"},
		{meta.BSE, NewStrPrefix("92"), meta.InstrumentTypeStock, "上市公司普通股", "首两位92: 上市公司普通股票; 920号段自2024-04-22起用于新上市公司"},

		{meta.BSE, NewStrPrefix("400"), meta.InstrumentTypeStock, "两网/退市A股", "两网公司及退市公司A股首三位代码为400"},
		{meta.BSE, NewStrPrefix("420"), meta.InstrumentTypeBStock, "退市B股", "退市公司B股首三位代码为420"},

		{meta.BSE, NewStrPrefix("810"), meta.InstrumentTypeBond, "可转换公司债", "向特定对象发行的可转换公司债券首三位代码为810"},
		{meta.BSE, NewStrPrefix("81"), meta.InstrumentTypeBond, "优先股(极少)", "其他极少数代码"},
		{meta.BSE, NewStrPrefix("820"), meta.InstrumentTypeBond, "优先股", "优先股票首三位代码为820"},
		{meta.BSE, NewStrPrefix("821"), meta.InstrumentTypeBond, "优先股", "优先股票首三位代码为820"},
		{meta.BSE, NewStrPrefix("82"), meta.InstrumentTypeBond, "优先股(极少)", "其他极少数代码"},
		{meta.BSE, NewStrPrefix("83"), meta.InstrumentTypeStock, "挂牌公司普通股", "挂牌公司普通股票首两位为83"},
		{meta.BSE, NewStrPrefix("840"), meta.InstrumentTypeOther, "要约收购", "要约收购证券代码首三位代码为84"},
		{meta.BSE, NewStrPrefix("841"), meta.InstrumentTypeOther, "要约回购", "要约回购证券代码首三位代码为841"},
		{meta.BSE, NewStrPrefix("87"), meta.InstrumentTypeStock, "挂牌公司普通股", "挂牌公司普通股票首两位为87"},
		{meta.BSE, NewStrPrefix("88"), meta.InstrumentTypeStock, "挂牌公司普通股", "挂牌公司普通股票首两位为88"},
		{meta.BSE, NewStrPrefix("850"), meta.InstrumentTypeOption, "股权激励期权", "股权激励期权首三位代码为850"},
	}
}
