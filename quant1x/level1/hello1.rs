use super::sequence_id;
use super::BinaryStream;
use crate::level1::commands::*;
use encoding_rs::GBK;

#[derive(Debug, Clone)]
pub struct Hello1Request {
    pub zip_flag: u8,
    pub seq_id: u32,
    pub packet_type: u8,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
    pub method: u16,
    pub padding: Vec<u8>,
}

impl Hello1Request {
    pub fn new() -> Self {
        Hello1Request {
            zip_flag: crate::level1::protocol::zlib_flag::UNCOMPRESSED, // NotZipped
            seq_id: sequence_id(),
            packet_type: 0x01,
            pkg_len1: 0,
            pkg_len2: 0,
            method: LOGIN1,
            padding: hex::decode("01").unwrap_or_default(),
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

    pub fn to_string(&self) -> String {
        format!("Hello1Request {{ ZipFlag:{}, SeqID:{}, PacketType:{}, PkgLen1:{}, PkgLen2:{}, Method:{:#06x}, padding:{} }}",
                self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method, hex::encode(&self.padding))
    }
}

#[derive(Debug, Clone)]
pub struct Hello1Response {
    pub info: String,
}

impl Hello1Response {
    pub fn new() -> Self {
        Self {
            info: String::new(),
        }
    }

    pub fn deserialize(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        let offset = 68usize;
        if data.len() >= offset {
            let info_bytes = &data[offset..];
            // decode GBK -> UTF-8 using encoding_rs
            let (cow, _, _) = GBK.decode(info_bytes);
            self.info = cow.into_owned();
        }
        Ok(())
    }

    pub fn to_string(&self) -> String {
        format!("Info: {}", self.info)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn deserialize_sample_matches_cpp() {
        let hex_data = "00e9070204280900073a02b2020c03840384038403840384033a02b2020c03840384038403840384030022ff3401194a010022ff3401154a0100ff00f70000010101ff00b1b1bea9c1aacda8d0d0c7e9b6fe000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010000010023b8dbb0c400000000000000000000000000000000000000000000000000";
        let buf = hex::decode(hex_data).unwrap();
        let mut resp = Hello1Response::new();
        resp.deserialize(&buf).expect("deserialize error");
        assert!(!resp.info.is_empty());
    }
}
