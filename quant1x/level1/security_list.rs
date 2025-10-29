use super::sequence_id;
use crate::level1::int_to_float64;
use crate::level1::protocol::{self, commands, Request, RequestHeader, Response, ResponseHeader};
use crate::std::BinaryStream;
use encoding_rs::GBK;

// Request builder for SECURITY_LIST
#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityListRequest {
    header: RequestHeader,
    market: u16,
    start: u32,
    count: u32,
    unknown: u32,
}

impl SecurityListRequest {
    pub fn new(market: u16, start: u32, count: u32) -> Self {
        let mut header = RequestHeader::new();
        header.zip_flag = 0x0C;
        header.seq_id = sequence_id();
        header.packet_type = 0x01;
        header.method = commands::SECURITY_LIST;

        SecurityListRequest {
            header,
            market,
            start,
            count,
            unknown: 0,
        }
    }

    pub fn market(&self) -> u16 {
        self.market
    }

    pub fn start(&self) -> u32 {
        self.start
    }
}

impl Request for SecurityListRequest {
    fn header(&self) -> &RequestHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut RequestHeader {
        &mut self.header
    }

    fn serialize_payload(&mut self) -> Vec<u8> {
        let mut payload = BinaryStream::new();
        payload.push_u16(self.market);
        payload.push_u32(self.start);
        payload.push_u32(self.count);
        payload.push_u32(self.unknown);
        payload.data().clone()
    }

    fn payload_string(&self) -> String {
        format!("{{Market:{}, Start:{}}}", self.market, self.start)
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
    header: ResponseHeader,
    pub count: u16,
    pub list: Vec<Security>,
}

#[allow(dead_code)]
impl SecurityListResponse {
    pub fn new() -> Self {
        Self {
            header: ResponseHeader::new(),
            count: 0,
            list: Vec::new(),
        }
    }
}

impl Response for SecurityListResponse {
    fn header(&self) -> &ResponseHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.header
    }

    fn deserialize_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.count = 0;
        self.list.clear();
        let mut bs = BinaryStream::from_vec(data.to_vec());
        if bs.data().len().saturating_sub(bs.position()) < 2 {
            return Ok(());
        }
        self.count = bs.get_u16()?;
        if self.count == 0 {
            return Ok(());
        }
        let min_required = 2 + (self.count as usize) * 25;
        if data.len() < min_required {
            log::warn!(
                "insufficient data for {} securities: data len {}, min required {}",
                self.count,
                data.len(),
                min_required
            );
            self.count = 0;
            return Ok(());
        }

        for _ in 0..self.count {
            let code = bs.get_string(6)?;
            let vol_unit = bs.get_u16()?;
            let mut name_buf = [0u8; 8];
            bs.get_byte_array(&mut name_buf)?;
            let name_nul = name_buf.iter().position(|&b| b == 0).unwrap_or(8);
            let (name_cow, _, _) = GBK.decode(&name_buf[..name_nul]);
            let name = name_cow.into_owned();
            let mut _rev1 = [0u8; 8];
            bs.get_byte_array(&mut _rev1)?;
            let mut _rev2 = [0u8; 4];
            bs.get_byte_array(&mut _rev2)?;
            let decimal_point = bs.get_u8()?;
            let tmp = bs.get_u32()?;
            let pre_close = int_to_float64(tmp);
            let mut _rev3 = [0u8; 4];
            bs.get_byte_array(&mut _rev3)?;

            self.list.push(Security {
                code,
                vol_unit,
                decimal_point,
                name,
                pre_close,
            });
        }
        Ok(())
    }

    fn body_string(&self) -> String {
        format!("{{Count:{}, Parsed:{}}}", self.count, self.list.len())
    }
}

/// Fetch a single page of security list from level1 server.
/// Returns Some(SecurityListResponse) on success, None on any IO error.
pub fn fetch_security_list(market: u16, start: u32, count: u32) -> Option<SecurityListResponse> {
    match crate::level1::client::client() {
        Ok(mut pooled) => {
            let mut request = SecurityListRequest::new(market, start, count);
            let mut response = SecurityListResponse::new();
            match protocol::process(pooled.stream(), &mut request, &mut response) {
                Ok(_) => {
                    log::info!(
                        "level1::security_list - market={} start={} count={}",
                        market,
                        start,
                        response.count
                    );
                    Some(response)
                }
                Err(e) => {
                    log::error!(
                        "level1 protocol::process error for security_list market={} start={}: {}",
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
