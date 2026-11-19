// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

pub mod block;
pub mod block_meta;
pub mod company;
pub mod finance_info;
pub mod heartbeat;
pub mod hello;
pub mod kline;
pub mod minute_time;
pub mod security_bars;
pub mod security_count;
pub mod security_list;
pub mod security_quote;
pub mod xdxr_info;

// 重新导出常用类型
pub use block::{BLOCK_CHUNKS_SIZE, BlockFileContext};
pub use block_meta::{
    BLOCK_DEFAULT, BLOCK_FENGGE, BLOCK_GAINIAN, BLOCK_ZHISHU, BlockFileMetaContext, BlockMeta,
};
pub use company::{CompanyCategoryRequest, CompanyInfoContext};
pub use finance_info::{FinanceInfo, FinanceInfoContext, FinanceInfoResponse, fetch_finance_info};
pub use heartbeat::HeartbeatContext;
pub use hello::{StdLoginContext, UpgradeTipContext};
pub use kline::BarFreq;
pub use minute_time::{HistoryMinuteTimeRequest, MinuteTime, fetch_history_minute_time};
pub use security_bars::{SecurityBar, SecurityBarsContext, SecurityBarsResponse};
pub use security_count::{SecurityCountContext, fetch_security_count};
pub use security_list::{
    PRE_REQUEST_MAX as SECURITY_LIST_PRE_REQUEST_MAX, Security, SecurityListContext,
    fetch_security_list,
};
pub use security_quote::{SecurityQuoteContext, SecurityQuoteData, StockInfo, TradeState};
pub use xdxr_info::{XdxrBatchRequest, XdxrInfoContext, fetch_xdxr, fetch_xdxr_batch};
