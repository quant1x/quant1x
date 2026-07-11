// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// ext — 扩展行情协议消息
// 对应 Python contrib/data/tdx/level1/ext.py

use crate::data::meta::instrument::{Instrument, InstrumentType};
use crate::base::BinaryStream;
use encoding_rs::GBK;

use super::super::command::{EXT_INSTRUMENT_COUNT, EXT_INSTRUMENT_INFO, EXT_SYNCHRONIZE, FLAG_GENERIC};
use super::super::protocol::{BaseFrame, RequestHeader, ResponseHeader};
use super::super::market::find_exchange_by_market_and_category;
use super::super::helpers::get_sequence_id;

// ============================================================
// SynchronizeContext — 扩展行情握手请求
// 对应 Python level1/ext.py SynchronizeContext
// 命令字: EXT_SYNCHRONIZE (0x2454)
// ============================================================

#[derive(Debug, Clone)]
pub struct ExtSynchronizeRequest {
    pub req_header: RequestHeader,
    pub resp_header: ResponseHeader,
    pub success: bool,
}

impl BaseFrame for ExtSynchronizeRequest {
    fn request_header(&self) -> &RequestHeader {
        &self.req_header
    }
    fn request_header_mut(&mut self) -> &mut RequestHeader {
        &mut self.req_header
    }
    fn response_header(&self) -> &ResponseHeader {
        &self.resp_header
    }
    fn response_header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.resp_header
    }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        // SynchronizeContext.serialize_request_body 的 80 字节 padding
        let padding = hex::decode(
            "e5bb1c2fafe52594\
             1f32c6e5d53dfb41\
             5b734cc9cdbf0ac9\
             2021bfdd1eb06d22\
             d008884c1611cb13\
             78f6abd824d899d2\
             1f32c6e5d53dfb41\
             1f32c6e5d53dfb41\
             a9325ac935dc0837\
             335a16e4ce17c1bb",
        )
        .unwrap_or_default();
        padding
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::base::DeserializeError> {
        // SynchronizeContext.deserialize_response_body:
        //   第一个字节是 result_code
        if data.is_empty() {
            self.success = false;
        } else {
            self.success = data[0] > 0;
        }
        Ok(())
    }
}

impl ExtSynchronizeRequest {
    pub fn new() -> Self {
        Self {
            req_header: RequestHeader::new(EXT_SYNCHRONIZE, FLAG_GENERIC),
            resp_header: ResponseHeader::new(),
            success: false,
        }
    }
}

// ============================================================
// InstrumentCountContext — 扩展行情 InstrumentCountContext
// 对应 Python level1/ext.py InstrumentCountContext
// 命令字: EXT_INSTRUMENT_COUNT (0x23f4)
// ============================================================

#[derive(Debug, Clone)]
pub struct InstrumentCountRequest {
    req_header: RequestHeader,
    resp_header: ResponseHeader,
    pub count: u32,
}

impl InstrumentCountRequest {
    pub fn new() -> Self {
        Self {
            req_header: RequestHeader::new(EXT_INSTRUMENT_COUNT, FLAG_GENERIC),
            resp_header: ResponseHeader::new(),
            count: 0,
        }
    }
}

impl BaseFrame for InstrumentCountRequest {
    fn request_header(&self) -> &RequestHeader {
        &self.req_header
    }
    fn request_header_mut(&mut self) -> &mut RequestHeader {
        &mut self.req_header
    }
    fn response_header(&self) -> &ResponseHeader {
        &self.resp_header
    }
    fn response_header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.resp_header
    }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        // InstrumentCountContext.serialize_request_body: return b''
        Vec::new()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::base::DeserializeError> {
        // InstrumentCountContext.deserialize_response_body:
        //   (name, reversed1, reversed2, num, reversed3, reversed4) = struct.unpack("<11s5I", data[:31])
        //   self.reply = {"source": name, "count": num}
        if data.len() >= 31 {
            let num_bytes: [u8; 4] = data[15..19].try_into().unwrap_or([0; 4]);
            self.count = u32::from_le_bytes(num_bytes);
        }
        Ok(())
    }
}

// ============================================================
// InstrumentInfo — 扩展行情证券列表
// 对应 Python level1/ext.py InstrumentInfo
// 命令字: EXT_INSTRUMENT_INFO (0x23f5)
// ============================================================

/// 单次最大获取数量, Python PRE_REQUEST_MAX = 1021
pub const EXT_PRE_REQUEST_MAX: u16 = 1021;

#[derive(Debug, Clone)]
pub struct InstrumentInfoRequest {
    req_header: RequestHeader,
    resp_header: ResponseHeader,
    pub start: u32,
    pub count: u16,
    /// 解析后的证券列表
    pub list: Vec<Instrument>,
}

impl InstrumentInfoRequest {
    pub fn new(start: u32, count: u16) -> Self {
        let mut header = RequestHeader::new(EXT_INSTRUMENT_INFO, 0x01);
        header.sequence_id = get_sequence_id();
        Self {
            req_header: header,
            resp_header: ResponseHeader::new(),
            start,
            count,
            list: Vec::new(),
        }
    }
}

impl BaseFrame for InstrumentInfoRequest {
    fn request_header(&self) -> &RequestHeader {
        &self.req_header
    }
    fn request_header_mut(&mut self) -> &mut RequestHeader {
        &mut self.req_header
    }
    fn response_header(&self) -> &ResponseHeader {
        &self.resp_header
    }
    fn response_header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.resp_header
    }

    /// 序列化请求体: start(u32) + count(u16) = 6 bytes (小端)
    fn serialize_request_body(&mut self) -> Vec<u8> {
        log::debug!(
            "[InstrumentInfo] serialize: start={}, count={}",
            self.start,
            self.count
        );
        let mut bs = BinaryStream::new();
        bs.push_u32(self.start);
        bs.push_u16(self.count);
        bs.data().clone()
    }

    /// 反序列化响应体
    /// 格式:
    ///   - 6 bytes: start(u32) + count(u16)
    ///   - 每条记录 64 bytes
    fn deserialize_response_body(
        &mut self,
        data: &[u8],
    ) -> Result<(), crate::base::DeserializeError> {
        self.list.clear();
        let mut bs = BinaryStream::from_vec(data.to_vec());

        let _start = bs.get_u32()?;
        let count = bs.get_u16()?;
        log::debug!(
            "[InstrumentInfo] deserialize: start={}, count={}",
            _start,
            count
        );

        for _ in 0..count {
            if bs.data().len() - bs.position() < 64 {
                log::warn!(
                    "[InstrumentInfo] insufficient data, remaining: {}",
                    bs.data().len() - bs.position()
                );
                break;
            }

            let category = bs.get_u8()?;
            let market = bs.get_u8()?;
            let price_precision = bs.get_u8()? as i32;
            let lot_size = bs.get_u8()? as i32;
            let _unused = bs.get_u8()?;

            let mut code_buf = [0u8; 9];
            bs.get_byte_array(&mut code_buf)?;
            let code = decode_gbk_null_trimmed(&code_buf);

            let mut name_buf = [0u8; 17];
            bs.get_byte_array(&mut name_buf)?;
            let name = decode_gbk_null_trimmed(&name_buf);

            let mut desc_buf = [0u8; 9];
            bs.get_byte_array(&mut desc_buf)?;
            let _desc = decode_gbk_null_trimmed(&desc_buf);

            // skip padding to 64 bytes (已读 5 + 9 + 17 + 9 = 40)
            let skip = 64usize.saturating_sub(40);
            if skip > 0 {
                let _ = bs.skip(skip);
            }

            let (exchange, instrument_type) =
                find_exchange_by_market_and_category(market as i32, category as i32);

            let inst = Instrument {
                exchange,
                instrument_type,
                ticker: code.to_lowercase(),
                name,
                lot_size,
                price_precision,
                ext_market: market as i32,
                ext_category: category as i32,
                alias_ticker: String::new(),
            };
            self.list.push(inst);
        }

        Ok(())
    }
}


fn decode_gbk_null_trimmed(buf: &[u8]) -> String {
    let nul = buf.iter().position(|&b| b == 0).unwrap_or(buf.len());
    let (cow, _, _) = GBK.decode(&buf[..nul]);
    cow.into_owned()
}
