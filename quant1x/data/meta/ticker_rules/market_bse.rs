// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// BSE 北京证券交易所规则 — 与 Python data/meta/ticker_rules/market_bse.py 对齐

use super::rule::{CodeRule, RulePrefix};
use super::super::exchange::Exchange;
use super::super::instrument::InstrumentType;

/// BSE 北京证券交易所规则
pub fn bse_rules() -> Vec<CodeRule> {
    vec![
        // 指数
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("899"), instrument_type: InstrumentType::INDEX, name: "指数", desc: "证券指数首三位代码为899" },

        // 股票
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("920"), instrument_type: InstrumentType::STOCK, name: "北交所新上市", desc: "2024-04-22 起新上市使用920号段; 已上市公司继续沿用原代码直到统一切换" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("92"), instrument_type: InstrumentType::STOCK, name: "上市公司普通股", desc: "首两位92: 上市公司普通股票; 920号段自2024-04-22起用于新上市公司" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("400"), instrument_type: InstrumentType::STOCK, name: "两网/退市A股", desc: "两网公司及退市公司A股首三位代码为400" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("420"), instrument_type: InstrumentType::BSTOCK, name: "退市B股", desc: "退市公司B股首三位代码为420" },

        // 债券/优先股
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("810"), instrument_type: InstrumentType::BOND, name: "可转换公司债", desc: "向特定对象发行的可转换公司债券首三位代码为810" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("81"), instrument_type: InstrumentType::BOND, name: "优先股(极少)", desc: "其他极少数代码" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("820"), instrument_type: InstrumentType::BOND, name: "优先股", desc: "优先股票首三位代码为820" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("821"), instrument_type: InstrumentType::BOND, name: "优先股", desc: "优先股票首三位代码为820" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("82"), instrument_type: InstrumentType::BOND, name: "优先股(极少)", desc: "其他极少数代码" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("83"), instrument_type: InstrumentType::STOCK, name: "挂牌公司普通股", desc: "挂牌公司普通股票首两位为83" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("840"), instrument_type: InstrumentType::OTHER, name: "要约收购", desc: "要约收购证券代码首三位代码为84" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("841"), instrument_type: InstrumentType::OTHER, name: "要约回购", desc: "要约回购证券代码首三位代码为841" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("87"), instrument_type: InstrumentType::STOCK, name: "挂牌公司普通股", desc: "挂牌公司普通股票首两位为87" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("88"), instrument_type: InstrumentType::STOCK, name: "挂牌公司普通股", desc: "挂牌公司普通股票首两位为88" },
        CodeRule { exchange: Exchange::BSE, prefix: RulePrefix::Str("850"), instrument_type: InstrumentType::OPTION, name: "股权激励期权", desc: "股权激励期权首三位代码为850" },
    ]
}
