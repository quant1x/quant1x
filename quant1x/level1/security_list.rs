use super::sequence_id;
use crate::level1::commands::*;
use crate::level1::int_to_float64;
use crate::std::BinaryStream;
use encoding_rs::GBK;

// Request builder for SECURITY_LIST
#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityListRequest {
    pub zip_flag: u8,
    pub seq_id: u32,
    pub packet_type: u8,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
    pub method: u16,
    pub market: u16,
    pub start: u16,
}

impl SecurityListRequest {
    pub fn new(market: u16, start: u16) -> Self {
        SecurityListRequest {
            zip_flag: 0x0C,
            seq_id: sequence_id(),
            packet_type: 0x01,
            pkg_len1: 0,
            pkg_len2: 0,
            method: SECURITY_LIST,
            market,
            start,
        }
    }

    pub fn serialize(&mut self) -> Vec<u8> {
        // payload is market(u16) + start(u16) => 4 bytes
        // pkg_len includes 2 bytes for Method + payload
        self.pkg_len1 = 2u16 + 4u16;
        self.pkg_len2 = self.pkg_len1;

        let mut buf = BinaryStream::new();
        buf.push_u8(self.zip_flag);
        buf.push_u32(self.seq_id);
        buf.push_u8(self.packet_type);
        buf.push_u16(self.pkg_len1);
        buf.push_u16(self.pkg_len2);
        buf.push_u16(self.method);

        // payload: market then start (matches C++ ordering)
        buf.push_u16(self.market);
        buf.push_u16(self.start);

        buf.data().clone()
    }
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct Security {
    pub code: String,
    pub vol_unit: u16,
    pub decimal_point: u8,
    pub name: String,
    pub pre_close: f64,
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityListResponse {
    pub count: u16,
    pub list: Vec<Security>,
}

#[allow(dead_code)]
impl SecurityListResponse {
    pub fn new() -> Self {
        Self {
            count: 0,
            list: Vec::new(),
        }
    }

    pub fn deserialize(&mut self, data: &[u8]) {
        self.count = 0;
        self.list.clear();
        let mut bs = BinaryStream::from_vec(data.to_vec());
        // Count
        if bs.data().len().saturating_sub(bs.position()) < 2 {
            return;
        }
        self.count = bs.get_u16();
        if self.count == 0 {
            return;
        }
        // Rough estimate: each security needs at least ~25 bytes
        let min_required = 2 + (self.count as usize) * 25;
        if data.len() < min_required {
            log::warn!(
                "insufficient data for {} securities: data len {}, min required {}",
                self.count,
                data.len(),
                min_required
            );
            return;
        }

        for _ in 0..self.count {
            // Code: 6 bytes string (ASCII numeric code in protocol) - no GBK decoding
            let code = bs.get_string(6);
            // VolUnit: u16
            let vol_unit = bs.get_u16();
            // Name: 8 bytes, GBK -> UTF-8
            let mut name_buf = [0u8; 8];
            bs.get_byte_array(&mut name_buf);
            let name_nul = name_buf.iter().position(|&b| b == 0).unwrap_or(8);
            let (name_cow, _, _) = GBK.decode(&name_buf[..name_nul]);
            let name = name_cow.into_owned();
            // Reversed1: 4 bytes skip
            let mut _rev1 = [0u8; 4];
            bs.get_byte_array(&mut _rev1);
            // DecimalPoint
            let decimal_point = bs.get_u8();
            // PreClose: u32 -> IntToFloat64
            let tmp = bs.get_u32();
            let pre_close = int_to_float64(tmp);
            // Reversed2: 4 bytes skip
            let mut _rev2 = [0u8; 4];
            bs.get_byte_array(&mut _rev2);

            self.list.push(Security {
                code,
                vol_unit,
                decimal_point,
                name,
                pre_close,
            });
        }
    }
}

/// Fetch a single page of security list from level1 server.
/// Returns Some(SecurityListResponse) on success, None on any IO error.
pub fn fetch_security_list(market: u16, start: u16) -> Option<SecurityListResponse> {
    match crate::level1::client::client() {
        Ok(mut pooled) => {
            let mut req = SecurityListRequest::new(market, start);
            let req_buf = req.serialize();
            match crate::level1::process_request(pooled.stream(), req_buf.as_slice()) {
                Ok(body) => {
                    let mut resp = SecurityListResponse::new();
                    resp.deserialize(&body);
                    log::info!(
                        "level1::security_list - market={} start={} count={}",
                        market,
                        start,
                        resp.count
                    );
                    Some(resp)
                }
                Err(e) => {
                    log::error!(
                        "level1 process_request error for security_list market={} start={}: {}",
                        market,
                        start,
                        e
                    );
                    None
                }
            }
        }
        Err(e) => {
            log::error!(
                "failed to acquire level1 client for security_list market={} start={}: {}",
                market,
                start,
                e
            );
            None
        }
    }
}
