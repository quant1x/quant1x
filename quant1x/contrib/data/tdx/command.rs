// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// TDX command — 行情指令定义
// 对应 Python quant1x/contrib/data/tdx/command.py

// ============================================================
// 行情类型
// ============================================================

/// 行情类型
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum QuoteType {
    /// 标准行情 L1
    Standard,
    /// 扩展行情 L1
    Extension,
    /// 二级行情 L2
    Level2,
}

impl QuoteType {
    /// 行情级别标识
    pub fn level(&self) -> &'static str {
        match self {
            QuoteType::Standard => "L1",
            QuoteType::Extension => "L1",
            QuoteType::Level2 => "L2",
        }
    }

    /// 标识符
    pub fn identifier(&self) -> &'static str {
        match self {
            QuoteType::Standard => "standard",
            QuoteType::Extension => "extension",
            QuoteType::Level2 => "level2",
        }
    }

    /// 中文描述
    pub fn desc(&self) -> &'static str {
        match self {
            QuoteType::Standard => "标准",
            QuoteType::Extension => "扩展",
            QuoteType::Level2 => "二级",
        }
    }
}

// ============================================================
// 命令定义
// ============================================================

/// 行情指令
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct Command {
    pub quote_type: QuoteType,
    pub value: u16,
    pub desc: &'static str,
}

impl Command {
    pub const fn new(quote_type: QuoteType, value: u16, desc: &'static str) -> Self {
        Self { quote_type, value, desc }
    }

    /// 从数值查找已定义的 Command(仅限标准+扩展命令)
    pub fn from_value(value: u16) -> Option<&'static Command> {
        ALL_COMMANDS.iter().find(|c| c.value == value).copied()
    }

    /// 创建未注册的临时 Command(不加入全局表)
    pub const fn adhoc(quote_type: QuoteType, value: u16, desc: &'static str) -> Self {
        Self { quote_type, value, desc }
    }
}

// ============================================================
// 标准行情命令
// ============================================================

pub const CMD_UNKNOWN: Command                      = Command::new(QuoteType::Standard, 0x0000, "未知");
pub const STD_SYNCHRONIZE1: Command                 = Command::new(QuoteType::Standard, 0x000d, "标准行情协议握手1");
pub const STD_SYNCHRONIZE2: Command                 = Command::new(QuoteType::Standard, 0x0fdb, "标准行情协议握手2");
pub const STD_HEARTBEAT: Command                    = Command::new(QuoteType::Standard, 0x0004, "心跳维持");
pub const STD_XDXR_INFO: Command                    = Command::new(QuoteType::Standard, 0x000f, "除权除息信息");
pub const STD_FINANCE_INFO: Command                 = Command::new(QuoteType::Standard, 0x0010, "财务信息");
pub const STD_PING: Command                         = Command::new(QuoteType::Standard, 0x0015, "测试连接");
pub const STD_COMPANY_CATEGORY: Command             = Command::new(QuoteType::Standard, 0x02cf, "公司信息分类");
pub const STD_COMPANY_CONTENT: Command              = Command::new(QuoteType::Standard, 0x02d0, "公司信息数据");
pub const STD_SECURITY_COUNT: Command               = Command::new(QuoteType::Standard, 0x044e, "证券数量");
pub const STD_SECURITY_LIST: Command                = Command::new(QuoteType::Standard, 0x044d, "证券列表");
pub const STD_OLD_SECURITY_LIST: Command            = Command::new(QuoteType::Standard, 0x0450, "证券列表(已废弃)");
pub const STD_SECURITY_BARS: Command                = Command::new(QuoteType::Standard, 0x052d, "K线");
pub const STD_SECURITY_QUOTES_OLD: Command          = Command::new(QuoteType::Standard, 0x053e, "旧版行情信息");
pub const STD_SECURITY_QUOTES_NEW: Command          = Command::new(QuoteType::Standard, 0x054c, "新版行情信息");
pub const STD_MINUTE_TIME_DATA: Command             = Command::new(QuoteType::Standard, 0x051d, "分时数据");
pub const STD_BLOCK_META: Command                   = Command::new(QuoteType::Standard, 0x02c5, "板块文件信息");
pub const STD_BLOCK_DATA: Command                   = Command::new(QuoteType::Standard, 0x06b9, "板块文件数据");
pub const STD_TRANSACTION_DATA: Command             = Command::new(QuoteType::Standard, 0x0fc5, "分笔成交信息");
pub const STD_HISTORY_MINUTE_DATA: Command          = Command::new(QuoteType::Standard, 0x0fb4, "历史分时信息");
pub const STD_HISTORY_TRANSACTION_DATA: Command     = Command::new(QuoteType::Standard, 0x0fb5, "历史分笔成交信息");
pub const STD_AUCTION_INFO: Command                 = Command::new(QuoteType::Standard, 0x056a, "集合竞价信息");
pub const STD_FUND_FLOW: Command                    = Command::new(QuoteType::Standard, 0x1218, "资金流向信息");

// ============================================================
// 扩展行情命令
// ============================================================

pub const EXT_SYNCHRONIZE: Command                  = Command::new(QuoteType::Extension, 0x2454, "扩展行情协议握手");
pub const EXT_SYNCHRONIZE2: Command                 = Command::new(QuoteType::Extension, 0x2455, "心跳维持");
pub const EXT_INSTRUMENT_COUNT: Command             = Command::new(QuoteType::Extension, 0x23f0, "证券数量");
pub const EXT_MARKET_LIST: Command                  = Command::new(QuoteType::Extension, 0x23f4, "市场列表");
pub const EXT_INSTRUMENT_INFO: Command              = Command::new(QuoteType::Extension, 0x23f5, "证券列表");
pub const EXT_INSTRUMENT_QUOTE_X1: Command          = Command::new(QuoteType::Extension, 0x23fa, "即时行情1");
pub const EXT_INSTRUMENT_QUOTE_X2: Command          = Command::new(QuoteType::Extension, 0x23fb, "即时行情2");
pub const EXT_TRANSACTION_DATA: Command             = Command::new(QuoteType::Extension, 0x23fc, "分笔成交");
pub const EXT_DAILY_TRANSACTION_DATA: Command       = Command::new(QuoteType::Extension, 0x2406, "分笔成交-某日");
pub const EXT_INSTRUMENT_BARS: Command              = Command::new(QuoteType::Extension, 0x23ff, "K线");
pub const EXT_TODO_2458: Command                    = Command::new(QuoteType::Extension, 0x2458, "除权除息信息");
pub const EXT_TODO_2459: Command                    = Command::new(QuoteType::Extension, 0x2459, "除权除息信息");
pub const EXT_XDXR_INFO: Command                    = Command::new(QuoteType::Extension, 0x2488, "除权除息信息");
pub const EXT_TODO_2489: Command                    = Command::new(QuoteType::Extension, 0x2489, "K线-含抛空量");
pub const EXT_FUTURES_QUOTES: Command               = Command::new(QuoteType::Extension, 0x248a, "期货行情");
pub const EXT_COMPANY_INFO_CATEGORIES: Command      = Command::new(QuoteType::Extension, 0x24b8, "公司信息分类");
pub const EXT_COMPANY_INFO_CONTENT: Command         = Command::new(QuoteType::Extension, 0x24b9, "公司信息数据");
pub const EXT_INTRADAY_CHART_SAMPLING: Command      = Command::new(QuoteType::Extension, 0x254d, "图形采样");

// ============================================================
// L2 行情命令
// ============================================================

#[allow(non_upper_case_globals)]
pub const L2_0x0547: Command                        = Command::new(QuoteType::Level2, 0x0547, "L2-即时行情");

// ============================================================
// 全局命令表(用于 from_value 查找)
// ============================================================

static ALL_COMMANDS: &[&Command] = &[
    &CMD_UNKNOWN,
    &STD_SYNCHRONIZE1,
    &STD_SYNCHRONIZE2,
    &STD_HEARTBEAT,
    &STD_XDXR_INFO,
    &STD_FINANCE_INFO,
    &STD_PING,
    &STD_COMPANY_CATEGORY,
    &STD_COMPANY_CONTENT,
    &STD_SECURITY_COUNT,
    &STD_SECURITY_LIST,
    &STD_OLD_SECURITY_LIST,
    &STD_SECURITY_BARS,
    &STD_SECURITY_QUOTES_OLD,
    &STD_SECURITY_QUOTES_NEW,
    &STD_MINUTE_TIME_DATA,
    &STD_BLOCK_META,
    &STD_BLOCK_DATA,
    &STD_TRANSACTION_DATA,
    &STD_HISTORY_MINUTE_DATA,
    &STD_HISTORY_TRANSACTION_DATA,
    &STD_AUCTION_INFO,
    &STD_FUND_FLOW,
    &EXT_SYNCHRONIZE,
    &EXT_SYNCHRONIZE2,
    &EXT_INSTRUMENT_COUNT,
    &EXT_MARKET_LIST,
    &EXT_INSTRUMENT_INFO,
    &EXT_INSTRUMENT_QUOTE_X1,
    &EXT_INSTRUMENT_QUOTE_X2,
    &EXT_TRANSACTION_DATA,
    &EXT_DAILY_TRANSACTION_DATA,
    &EXT_INSTRUMENT_BARS,
    &EXT_TODO_2458,
    &EXT_TODO_2459,
    &EXT_XDXR_INFO,
    &EXT_TODO_2489,
    &EXT_FUTURES_QUOTES,
    &EXT_COMPANY_INFO_CATEGORIES,
    &EXT_COMPANY_INFO_CONTENT,
    &EXT_INTRADAY_CHART_SAMPLING,
    &L2_0x0547,
];

// ============================================================
// 协议标志位
// ============================================================

/// 帧类型标志
pub const FLAG_ZIP: u8           = 0x10;
/// 未帧类型标志
pub const FLAG_UNCOMPRESSED: u8  = 0x0C;
/// 帧类型标志(ZIP | UNCOMPRESSED)
pub const FLAG_ZIPPED: u8        = FLAG_ZIP | FLAG_UNCOMPRESSED;
/// 一般性标志
pub const FLAG_GENERIC: u8       = 0x01;

// ============================================================
// 测试
// ============================================================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_quote_type() {
        assert_eq!(QuoteType::Standard.level(), "L1");
        assert_eq!(QuoteType::Extension.identifier(), "extension");
        assert_eq!(QuoteType::Level2.desc(), "二级");
    }

    #[test]
    fn test_command_from_value() {
        assert_eq!(Command::from_value(0x0004), Some(&STD_HEARTBEAT));
        assert_eq!(Command::from_value(0x2454), Some(&EXT_SYNCHRONIZE));
        assert_eq!(Command::from_value(0x0547), Some(&L2_0x0547));
        assert_eq!(Command::from_value(0xFFFF), None);
    }

    #[test]
    fn test_command_values() {
        assert_eq!(STD_SECURITY_LIST.value, 0x044d);
        assert_eq!(STD_SECURITY_BARS.value, 0x052d);
        assert_eq!(STD_TRANSACTION_DATA.value, 0x0fc5);
        assert_eq!(STD_BLOCK_META.value, 0x02c5);
        assert_eq!(STD_BLOCK_DATA.value, 0x06b9);
        assert_eq!(STD_XDXR_INFO.value, 0x000f);
        assert_eq!(EXT_INSTRUMENT_BARS.value, 0x23ff);
    }

    #[test]
    fn test_flags() {
        assert_eq!(FLAG_ZIPPED, 0x1C);
        assert_eq!(FLAG_GENERIC, 0x01);
    }
}
