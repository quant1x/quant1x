#![allow(dead_code)]
use super::sequence_id;
use crate::level1::commands::*;
use crate::level1::protocol::{Request, RequestHeader, Response, ResponseHeader};
use crate::std::BinaryStream;

// Request builder for SECURITY_COUNT
#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityCountRequest {
    header: RequestHeader,
    pub market: u16,
}

impl Request for SecurityCountRequest {
    fn header(&self) -> &RequestHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut RequestHeader {
        &mut self.header
    }

    fn serialize_payload(&mut self) -> Vec<u8> {
        let mut buf = BinaryStream::new();
        // payload
        buf.push_u16(self.market);
        // padding bytes: 75c73301
        buf.push_byte_array(&[0x75, 0xc7, 0x33, 0x01]);
        buf.data().clone()
    }

    fn payload_string(&self) -> String {
        format!("SecurityCountRequest{{market:{}}}", self.market)
    }
}

impl SecurityCountRequest {
    pub fn new(market: u16) -> Self {
        let mut header = RequestHeader::new();
        header.zip_flag = crate::level1::protocol::zlib_flag::UNCOMPRESSED;
        header.seq_id = sequence_id();
        header.packet_type = 0x01;
        header.method = SECURITY_COUNT;

        SecurityCountRequest { header, market }
    }
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityCountResponse {
    header: ResponseHeader,
    pub count: usize,
}

impl Response for SecurityCountResponse {
    fn header(&self) -> &ResponseHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.header
    }

    fn deserialize_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        if data.len() < 2 {
            return Ok(());
        }
        let mut bs = BinaryStream::from_vec(data.to_vec());
        let c = bs.get_u16()?;
        self.count = c as usize;
        Ok(())
    }

    fn body_string(&self) -> String {
        format!("SecurityCountResponse{{count:{}}}", self.count)
    }
}

#[allow(dead_code)]
impl SecurityCountResponse {
    pub fn new() -> Self {
        Self {
            header: ResponseHeader::new(),
            count: 0,
        }
    }
}

/// Fetch security count for a market from level1 server.
pub fn fetch_security_count(market: u16) -> Option<SecurityCountResponse> {
    match crate::level1::client::get_std_conn() {
        Ok(mut pooled) => {
            let mut req = SecurityCountRequest::new(market);
            let mut resp = SecurityCountResponse::new();
            match crate::level1::process(pooled.stream(), &mut req, &mut resp) {
                Ok(_) => {
                    log::info!(
                        "level1::security_count - market={} count={}",
                        market,
                        resp.count
                    );
                    Some(resp)
                }
                Err(e) => {
                    log::error!(
                        "level1 process error for security_count market={}: {}",
                        market,
                        e.to_string()
                    );
                    None
                }
            }
        }
        Err(e) => {
            log::error!(
                "failed to acquire level1 client for security_count market={}: {}",
                market,
                e
            );
            None
        }
    }
}
