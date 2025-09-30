use super::sequence_id;
use super::BinaryStream;
use crate::level1::commands::*;

#[derive(Debug, Clone)]
pub struct HeartbeatRequest {
    pub zip_flag: u8,
    pub seq_id: u32,
    pub packet_type: u8,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
    pub method: u16,
}

impl HeartbeatRequest {
    pub fn new() -> Self {
        HeartbeatRequest {
            zip_flag: 0x0C,
            seq_id: sequence_id(),
            packet_type: 0x02,
            pkg_len1: 0,
            pkg_len2: 0,
            method: HEARTBEAT,
        }
    }
    pub fn serialize(&mut self) -> Vec<u8> {
        self.pkg_len1 = 2;
        self.pkg_len2 = 2;
        let mut stream = BinaryStream::new();
        stream.push_u8(self.zip_flag);
        stream.push_u32(self.seq_id);
        stream.push_u8(self.packet_type);
        stream.push_u16(self.pkg_len1);
        stream.push_u16(self.pkg_len2);
        stream.push_u16(self.method);
        stream.data().clone()
    }
}

#[derive(Debug, Clone)]
pub struct HeartbeatResponse {
    pub info: String,
}
impl HeartbeatResponse {
    pub fn new() -> Self {
        Self {
            info: String::new(),
        }
    }
    pub fn deserialize(&mut self, data: &[u8]) {
        let mut bs = BinaryStream::from_vec(data.to_vec());
        self.info = bs.get_string(10);
    }
}
