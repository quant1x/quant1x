use super::sequence_id;
use crate::level1::security_bars::SecurityBarsResponse;
use crate::std::BinaryStream;
use crate::level1::commands::*;

#[derive(Debug, Clone)]
pub struct IndexBarsRequest {
    pub zip_flag: u8,
    pub seq_id: u32,
    pub packet_type: u8,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
    pub method: u16,
}
#[allow(dead_code)]
impl IndexBarsRequest {
    pub fn new() -> Self {
        IndexBarsRequest {
            zip_flag: 0x0C,
            seq_id: sequence_id(),
            packet_type: 0x01,
            pkg_len1: 0,
            pkg_len2: 0,
                method: INDEX_BARS,
        }
    }
    pub fn serialize(&mut self) -> Vec<u8> {
        let mut s = BinaryStream::new();
        s.push_u8(self.zip_flag);
        s.push_u32(self.seq_id);
        s.push_u8(self.packet_type);
        s.push_u16(self.pkg_len1);
        s.push_u16(self.pkg_len2);
        s.push_u16(self.method);
        s.data().clone()
    }
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct IndexBarsResponse {
    pub inner: SecurityBarsResponse,
}
#[allow(dead_code)]
impl IndexBarsResponse {
    pub fn new() -> Self {
        Self {
            inner: SecurityBarsResponse::new_with(true, 0),
        }
    }

    pub fn new_with(category: u16) -> Self {
        Self {
            inner: SecurityBarsResponse::new_with(true, category),
        }
    }

    pub fn deserialize(&mut self, data: &[u8]) {
        self.inner.deserialize(data);
    }
}
