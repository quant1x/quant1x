use super::sequence_id;
use super::BinaryStream;
use crate::level1::commands::*;
use crate::level1::protocol::{Request, RequestHeader, Response, ResponseHeader};
use encoding_rs::GBK;

#[derive(Debug, Clone)]
pub struct Hello1Request {
    pub header: RequestHeader,
    pub padding: Vec<u8>,
}

impl Request for Hello1Request {
    fn header(&self) -> &RequestHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut RequestHeader {
        &mut self.header
    }

    fn serialize_payload(&mut self) -> Vec<u8> {
        self.header.pkg_len1 = (2 + self.padding.len()) as u16;
        self.header.pkg_len2 = self.header.pkg_len1;
        let mut stream = BinaryStream::new();
        stream.push_byte_array(&self.padding);
        stream.data().clone()
    }

    fn payload_string(&self) -> String {
        format!("Hello1Request {{ padding:{} }}", hex::encode(&self.padding))
    }
}

impl Hello1Request {
    pub fn new() -> Self {
        let mut header = RequestHeader::new();
        header.zip_flag = crate::level1::protocol::zlib_flag::UNCOMPRESSED;
        header.seq_id = sequence_id();
        header.packet_type = 0x01;
        header.method = LOGIN1;

        Hello1Request {
            header,
            padding: hex::decode("01").unwrap_or_default(),
        }
    }
}

#[derive(Debug, Clone)]
pub struct Hello1Response {
    pub header: ResponseHeader,
    pub info: String,
}

impl Response for Hello1Response {
    fn header(&self) -> &ResponseHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.header
    }

    fn deserialize_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        // The body has 68 bytes offset before the info string.
        let offset = 68usize;
        if data.len() >= offset {
            let info_bytes = &data[offset..];
            let (cow, _, _) = GBK.decode(info_bytes);
            self.info = cow.into_owned();
        }
        Ok(())
    }

    fn body_string(&self) -> String {
        format!("Info: {}", self.info)
    }
}

impl Hello1Response {
    pub fn new() -> Self {
        Self {
            header: ResponseHeader::new(),
            info: String::new(),
        }
    }

    // Deprecated
    pub fn deserialize(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.deserialize_body(data)
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
