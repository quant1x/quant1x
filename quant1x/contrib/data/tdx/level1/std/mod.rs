// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// std — 标准行情协议消息
// 按命令字拆分为独立子模块，统一通过 mod.rs 重新导出

pub mod hello;
pub mod heartbeat;
pub mod kline;
pub mod security_count;
pub mod block_meta;
pub mod minute_time;
pub mod security_quote;
pub mod company;

// 重新导出常用类型
pub use hello::{Hello1Request, Hello2Request};
pub use heartbeat::HeartbeatRequest;
pub use kline::KLineType;
pub use security_count::{SecurityCountRequest, fetch_security_count};
pub use block_meta::{BlockMetaRequest, BlockMeta, BLOCK_ZHISHU, BLOCK_FENGGE, BLOCK_GAINIAN, BLOCK_DEFAULT};
pub use minute_time::{HistoryMinuteTimeRequest, MinuteTime, fetch_history_minute_time};
pub use security_quote::{SecurityQuoteRequest, SecurityQuoteData, StockInfo, TradeState};
pub use company::{CompanyCategoryRequest, CompanyContentRequest};
