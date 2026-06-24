// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// xdxr — 除权除息信息 (STD_XDXR_INFO, 0x000f)
// 对应 Python level1/std/xdxr.py
// 对应 C++   level1/xdxr_info.h

use crate::std::BinaryStream;
use crate::data::schema::{XdxrInfo, XdxrEntry};
use crate::data::meta::Exchange;

use super::super::super::command::*;
use super::super::super::helpers::{get_datetime_from_u32, int_to_float64, exchange_to_market, market_to_exchange};
use super::super::super::protocol::{BaseFrame, RequestHeader, ResponseHeader};

// ============================================================
// XdxrInfoContext — 单只证券除权除息查询
// 对应 Python class Xdxr
// ============================================================

/// 单只证券除权除息请求/响应
#[derive(Debug, Clone)]
pub struct XdxrInfoContext {
    req_header: RequestHeader,
    resp_header: ResponseHeader,
    /// TDX 市场编号 (0:深圳, 1:上海)
    pub market: u8,
    /// 证券代码 (6字节 ASCII)
    pub ticker: String,
    /// 返回的记录条数
    pub count: u16,
    /// 除权除息记录列表
    pub list: Vec<XdxrInfo>,
}

impl XdxrInfoContext {
    pub fn new(exchange: Exchange, ticker: &str) -> Self {
        let market_id = exchange_to_market(exchange.code())
            .unwrap_or(0) as u8;

        Self {
            req_header: RequestHeader::new(STD_XDXR_INFO, FLAG_UNCOMPRESSED),
            resp_header: ResponseHeader::new(),
            market: market_id,
            ticker: ticker.to_string(),
            count: 0,
            list: Vec::new(),
        }
    }
}

impl BaseFrame for XdxrInfoContext {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        let mut bs = BinaryStream::new();
        // padding: bytes.fromhex('0100') = [0x01, 0x00]
        bs.push_u8(0x01);
        bs.push_u8(0x00);
        // market: u8
        bs.push_u8(self.market);
        // code: 6 bytes ASCII, pad with \0 if shorter
        let mut code_bytes = [0u8; 6];
        let ticker_bytes = self.ticker.as_bytes();
        let len = ticker_bytes.len().min(6);
        code_bytes[..len].copy_from_slice(&ticker_bytes[..len]);
        bs.push_byte_array(&code_bytes);
        bs.data().clone()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.list.clear();
        if data.len() < 11 {
            // 9 bytes unknown + 2 bytes count = 11 minimum
            return Ok(());
        }

        let mut bs = BinaryStream::from_vec(data.to_vec());
        let data_len = data.len();

        // skip 9 bytes unknown header
        bs.skip(9);

        // count: u16
        let remaining = data_len.saturating_sub(bs.position());
        if remaining < 2 {
            return Ok(());
        }
        self.count = bs.get_u16()?;

        for _ in 0..self.count {
            let remaining = data_len.saturating_sub(bs.position());
            if remaining < 29 {
                log::warn!("Insufficient data when parsing XDXR_INFO payload");
                break;
            }

            // market: u8 (skip)
            bs.skip(1);

            // code: 6 bytes ASCII (skip)
            bs.skip(6);

            // unknown: u8 (skip)
            bs.skip(1);

            // date: u32
            let date_int = bs.get_u32()?;

            // category: u8
            let category = bs.get_u8()? as i32;

            // record_data: 16 bytes
            let mut record_data = [0u8; 16];
            bs.get_byte_array(&mut record_data)?;

            // parse date
            let (year, month, day, _, _) = get_datetime_from_u32(9, date_int, 0);

            let mut info = XdxrInfo {
                date: format!("{:04}-{:02}-{:02}", year, month, day),
                category,
                name: crate::data::schema::XdxrCategory::to_string(category),
                ..Default::default()
            };

            // parse record_data based on category
            let mut tmp = BinaryStream::from_vec(record_data.to_vec());
            match category {
                1 => {
                    // 除权除息
                    info.fen_hong = tmp.get_f32()? as f64;
                    info.pei_gu_jia = tmp.get_f32()? as f64;
                    info.song_zhuan_gu = tmp.get_f32()? as f64;
                    info.pei_gu = tmp.get_f32()? as f64;
                }
                11 | 12 => {
                    // 扩缩股 / 非流通股缩股
                    tmp.skip(8);
                    info.suo_gu = tmp.get_f32()? as f64;
                }
                13 | 14 => {
                    // 送认购权证 / 送认沽权证
                    info.xing_quan_jia = tmp.get_f32()? as f64;
                    tmp.skip(8);
                    info.fen_shu = tmp.get_f32()? as f64;
                }
                _ => {
                    // 其他类型: 股本变动
                    let v1 = tmp.get_u32()?;
                    info.qian_liu_tong = Self::get_v(v1);
                    let v2 = tmp.get_u32()?;
                    info.qian_zong_gu_ben = Self::get_v(v2);
                    let v3 = tmp.get_u32()?;
                    info.hou_liu_tong = Self::get_v(v3);
                    let v4 = tmp.get_u32()?;
                    info.hou_zong_gu_ben = Self::get_v(v4);
                }
            }

            self.list.push(info);
        }

        log::debug!("xdxr fetched market={} ticker={} count={} parsed={}",
                    self.market, self.ticker, self.count, self.list.len());

        Ok(())
    }
}

impl XdxrInfoContext {
    /// 将 u32 整数转换为 f64 浮点数(与 level1 协议中使用的转换一致)
    fn get_v(v: u32) -> f64 {
        if v == 0 {
            return 0.0;
        }
        int_to_float64(v)
    }
}

// ============================================================
// XdxrBatchRequest — 批量除权除息查询
// 对应 Python class XdxrBatch
// ============================================================

/// 批量除权除息请求/响应
#[derive(Debug, Clone)]
pub struct XdxrBatchRequest {
    req_header: RequestHeader,
    resp_header: ResponseHeader,
    /// 证券列表 (exchange, ticker)
    pub instruments: Vec<(Exchange, String)>,
    /// 返回的条目数量
    pub count: u16,
    /// 批量除权除息条目列表
    pub list: Vec<XdxrEntry>,
}

impl XdxrBatchRequest {
    pub fn new(instruments: Vec<(Exchange, String)>) -> Self {
        Self {
            req_header: RequestHeader::new(STD_XDXR_INFO, FLAG_UNCOMPRESSED),
            resp_header: ResponseHeader::new(),
            instruments,
            count: 0,
            list: Vec::new(),
        }
    }
}

impl BaseFrame for XdxrBatchRequest {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        let mut bs = BinaryStream::new();
        let inst_count = self.instruments.len() as u16;
        bs.push_u16(inst_count);

        for (exchange, ticker) in &self.instruments {
            let market_id = exchange_to_market(exchange.code()).unwrap_or(0) as u8;
            bs.push_u8(market_id);

            // code: 6 bytes ASCII, pad with \0 if shorter
            let mut code_bytes = [0u8; 6];
            let ticker_bytes = ticker.as_bytes();
            let len = ticker_bytes.len().min(6);
            code_bytes[..len].copy_from_slice(&ticker_bytes[..len]);
            bs.push_byte_array(&code_bytes);
        }

        bs.data().clone()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.list.clear();
        if data.len() < 2 {
            return Ok(());
        }

        let mut bs = BinaryStream::from_vec(data.to_vec());
        let data_len = data.len();

        self.count = bs.get_u16()?;

        for _ in 0..self.count {
            let remaining = data_len.saturating_sub(bs.position());
            if remaining < 9 {
                log::warn!("Insufficient data when parsing XDXR_BATCH header");
                break;
            }

            // market_id: u8
            let market_id = bs.get_u8()? as i32;

            // ticker: 6 bytes ASCII
            let mut code_bytes = [0u8; 6];
            bs.get_byte_array(&mut code_bytes)?;
            let ticker = String::from_utf8_lossy(&code_bytes)
                .trim_end_matches('\0')
                .trim()
                .to_string();

            // sub_count: u16
            let sub_count = bs.get_u16()? as usize;

            let exchange_str = market_to_exchange(market_id).unwrap_or("SSE");
            let exchange = Exchange::parse(exchange_str)
                .unwrap_or(Exchange::SSE);

            let mut entry = XdxrEntry {
                exchange,
                ticker: ticker.clone(),
                count: sub_count as i32,
                list: Vec::with_capacity(sub_count),
            };

            for _ in 0..sub_count {
                let remaining = data_len.saturating_sub(bs.position());
                if remaining < 29 {
                    log::warn!("Insufficient data when parsing XDXR_BATCH record");
                    break;
                }

                // market (skip)
                bs.skip(1);
                // code (skip)
                bs.skip(6);
                // unknown (skip)
                bs.skip(1);

                // date: u32
                let date_int = bs.get_u32()?;

                // category: u8
                let category = bs.get_u8()? as i32;

                // record_data: 16 bytes
                let mut record_data = [0u8; 16];
                bs.get_byte_array(&mut record_data)?;

                // parse date
                let (year, month, day, _, _) = get_datetime_from_u32(9, date_int, 0);

                let mut info = XdxrInfo {
                    date: format!("{:04}-{:02}-{:02}", year, month, day),
                    category,
                    name: crate::data::schema::XdxrCategory::to_string(category),
                    ..Default::default()
                };

                // parse record_data based on category
                let mut tmp = BinaryStream::from_vec(record_data.to_vec());
                match category {
                    1 => {
                        info.fen_hong = tmp.get_f32()? as f64;
                        info.pei_gu_jia = tmp.get_f32()? as f64;
                        info.song_zhuan_gu = tmp.get_f32()? as f64;
                        info.pei_gu = tmp.get_f32()? as f64;
                    }
                    11 | 12 => {
                        tmp.skip(8);
                        info.suo_gu = tmp.get_f32()? as f64;
                    }
                    13 | 14 => {
                        info.xing_quan_jia = tmp.get_f32()? as f64;
                        tmp.skip(8);
                        info.fen_shu = tmp.get_f32()? as f64;
                    }
                    _ => {
                        let v1 = tmp.get_u32()?;
                        info.qian_liu_tong = XdxrInfoContext::get_v(v1);
                        let v2 = tmp.get_u32()?;
                        info.qian_zong_gu_ben = XdxrInfoContext::get_v(v2);
                        let v3 = tmp.get_u32()?;
                        info.hou_liu_tong = XdxrInfoContext::get_v(v3);
                        let v4 = tmp.get_u32()?;
                        info.hou_zong_gu_ben = XdxrInfoContext::get_v(v4);
                    }
                }

                entry.list.push(info);
            }

            self.list.push(entry);
        }

        log::debug!("xdxr_batch fetched count={} parsed={}",
                    self.count, self.list.len());

        Ok(())
    }
}

// ============================================================
// fetch_xdxr — 便捷函数
// ============================================================

/// 获取单只证券的除权除息信息
pub fn fetch_xdxr(exchange: Exchange, ticker: &str) -> Option<XdxrInfoContext> {
    match super::super::super::client::get_std_conn() {
        Ok(mut conn) => {
            let mut msg = XdxrInfoContext::new(exchange, ticker);
            match super::super::super::protocol::transact_message_sync(conn.stream(), &mut msg) {
                Ok(()) => Some(msg),
                Err(e) => {
                    log::error!("level1 xdxr process error for {} {}: {}", exchange.code(), ticker, e);
                    None
                }
            }
        }
        Err(e) => {
            log::error!("failed to acquire level1 client for xdxr {} {}: {}", exchange.code(), ticker, e);
            None
        }
    }
}

/// 批量获取多只证券的除权除息信息
pub fn fetch_xdxr_batch(instruments: Vec<(Exchange, String)>) -> Option<XdxrBatchRequest> {
    if instruments.is_empty() {
        return None;
    }
    match super::super::super::client::get_std_conn() {
        Ok(mut conn) => {
            let mut msg = XdxrBatchRequest::new(instruments);
            match super::super::super::protocol::transact_message_sync(conn.stream(), &mut msg) {
                Ok(()) => Some(msg),
                Err(e) => {
                    log::error!("level1 xdxr_batch process error: {}", e);
                    None
                }
            }
        }
        Err(e) => {
            log::error!("failed to acquire level1 client for xdxr_batch: {}", e);
            None
        }
    }
}
