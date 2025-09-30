use super::sequence_id;
use super::BinaryStream;
use crate::level1::commands::*;
use encoding_rs::GBK;

#[derive(Debug, Clone)]
pub struct Hello2Request {
    pub zip_flag: u8,
    pub seq_id: u32,
    pub packet_type: u8,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
    pub method: u16,
    pub padding: Vec<u8>,
}

impl Hello2Request {
    pub fn new() -> Self {
        Hello2Request {
            zip_flag: 0x0C,
            seq_id: sequence_id(),
            packet_type: 0x01,
            pkg_len1: 0,
            pkg_len2: 0,
            method: LOGIN2, // LOGIN2
            padding: hex::decode("d5d0c9ccd6a4a8af0000008fc22540130000d500c9ccbdf0d7ea00000002")
                .unwrap_or_default(),
        }
    }

    pub fn serialize(&mut self) -> Vec<u8> {
        self.pkg_len1 = (2 + self.padding.len()) as u16;
        self.pkg_len2 = (2 + self.padding.len()) as u16;
        let mut stream = BinaryStream::new();
        stream.push_u8(self.zip_flag);
        stream.push_u32(self.seq_id);
        stream.push_u8(self.packet_type);
        stream.push_u16(self.pkg_len1);
        stream.push_u16(self.pkg_len2);
        stream.push_u16(self.method);
        stream.push_byte_array(&self.padding);
        stream.data().clone()
    }
}

#[derive(Debug, Clone)]
pub struct Hello2Response {
    pub info: String,
}
impl Hello2Response {
    pub fn new() -> Self {
        Self {
            info: String::new(),
        }
    }
    pub fn deserialize(&mut self, data: &[u8]) {
        let offset = 58usize;
        if data.len() >= offset {
            let (cow, _, _) = GBK.decode(&data[offset..]);
            self.info = cow.into_owned();
        }
    }
}
