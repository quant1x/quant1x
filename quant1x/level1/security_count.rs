#![allow(dead_code)]
use super::sequence_id;
use crate::std::BinaryStream;

// Request builder for SECURITY_COUNT
#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityCountRequest {
    pub zip_flag: u8,
    pub seq_id: u32,
    pub packet_type: u8,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
    pub method: u16,
    pub market: u16,
}

impl SecurityCountRequest {
    pub fn new(market: u16) -> Self {
        SecurityCountRequest {
            zip_flag: 0x0C,
            seq_id: sequence_id(),
            packet_type: 0x01,
            pkg_len1: 0,
            pkg_len2: 0,
            method: 0x0451, // StdCommand::SECURITY_COUNT in C++
            market,
        }
    }

    pub fn serialize(&mut self) -> Vec<u8> {
        // payload: market(u16) + padding(4)
        self.pkg_len1 = 2u16 + 2u16 + 4u16; // 2 bytes Method + payload
        self.pkg_len2 = self.pkg_len1;

        let mut buf = BinaryStream::new();
        buf.push_u8(self.zip_flag);
        buf.push_u32(self.seq_id);
        buf.push_u8(self.packet_type);
        buf.push_u16(self.pkg_len1);
        buf.push_u16(self.pkg_len2);
        buf.push_u16(self.method);

        // payload
        buf.push_u16(self.market);
        // padding bytes: 75c73301
        buf.push_byte_array(&[0x75, 0xc7, 0x33, 0x01]);

        buf.data().clone()
    }
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityCountResponse {
    pub count: usize,
}
#[allow(dead_code)]
impl SecurityCountResponse {
    pub fn new() -> Self {
        Self { count: 0 }
    }
    pub fn deserialize(&mut self, data: &[u8]) {
        if data.len() < 2 {
            return;
        }
        let mut bs = BinaryStream::from_vec(data.to_vec());
        let c = bs.get_u16();
        self.count = c as usize;
    }
}

/// Fetch security count for a market from level1 server.
pub fn fetch_security_count(market: u16) -> Option<SecurityCountResponse> {
    match crate::level1::client::client() {
        Ok(mut pooled) => {
            let mut req = SecurityCountRequest::new(market);
            let req_buf = req.serialize();
            match crate::level1::process_request(pooled.stream(), req_buf.as_slice()) {
                Ok(body) => {
                    let mut resp = SecurityCountResponse::new();
                    resp.deserialize(&body);
                    log::info!(
                        "level1::security_count - market={} count={}",
                        market,
                        resp.count
                    );
                    Some(resp)
                }
                Err(e) => {
                    log::error!(
                        "level1 process_request error for security_count market={}: {}",
                        market,
                        e
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
