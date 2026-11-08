// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

pub mod hello;
pub mod heartbeat;
pub mod kline;
pub mod security_count;
pub mod security_list;
pub mod block;
pub mod block_meta;
pub mod minute_time;
pub mod security_quote;
pub mod company;
pub mod security_bars;
pub mod finance_info;
pub mod xdxr_info;

// 重新导出常用类型
pub use hello::{StdLoginContext, UpgradeTipContext};
pub use heartbeat::HeartbeatContext;
pub use kline::BarFreq;
pub use security_count::{SecurityCountContext, fetch_security_count};
pub use security_list::{SecurityListContext, Security, PRE_REQUEST_MAX as SECURITY_LIST_PRE_REQUEST_MAX, fetch_security_list};
pub use block::{BlockFileContext, BLOCK_CHUNKS_SIZE};
pub use block_meta::{BlockFileMetaContext, BlockMeta, BLOCK_ZHISHU, BLOCK_FENGGE, BLOCK_GAINIAN, BLOCK_DEFAULT};
pub use minute_time::{HistoryMinuteTimeRequest, MinuteTime, fetch_history_minute_time};
pub use security_quote::{SecurityQuoteContext, SecurityQuoteData, StockInfo, TradeState};
pub use company::{CompanyCategoryRequest, CompanyInfoContext};
pub use security_bars::{SecurityBarsContext, SecurityBarsResponse, SecurityBar};
pub use finance_info::{FinanceInfoContext, FinanceInfoResponse, FinanceInfo, fetch_finance_info};
pub use xdxr_info::{XdxrInfoContext, XdxrBatchRequest, fetch_xdxr, fetch_xdxr_batch};
