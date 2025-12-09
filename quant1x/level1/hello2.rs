use super::sequence_id;
use super::BinaryStream;
use crate::level1::commands::*;
use crate::level1::protocol::{Request, RequestHeader, Response, ResponseHeader};
use encoding_rs::GBK;

#[derive(Debug, Clone)]
pub struct Hello2Request {
    pub header: RequestHeader,
    pub padding: Vec<u8>,
}

impl Request for Hello2Request {
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
        format!("Hello2Request {{ padding:{} }}", hex::encode(&self.padding))
    }
}

impl Hello2Request {
    pub fn new() -> Self {
        let mut header = RequestHeader::new();
        header.zip_flag = crate::level1::protocol::zlib_flag::UNCOMPRESSED;
        header.seq_id = sequence_id();
        header.packet_type = 0x01;
        header.method = LOGIN2;

        Hello2Request {
            header,
            padding: hex::decode("d5d0c9ccd6a4a8af0000008fc22540130000d500c9ccbdf0d7ea00000002")
                .unwrap_or_default(),
        }
    }
}

#[derive(Debug, Clone)]
pub struct Hello2Response {
    pub header: ResponseHeader,
    pub info: String,
}

impl Response for Hello2Response {
    fn header(&self) -> &ResponseHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.header
    }

    fn deserialize_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        let offset = 58usize;
        if data.len() >= offset {
            let (cow, _, _) = GBK.decode(&data[offset..]);
            self.info = cow.into_owned();
        }
        Ok(())
    }

    fn body_string(&self) -> String {
        format!("Info: {}", self.info)
    }
}

impl Hello2Response {
    pub fn new() -> Self {
        Self {
            header: ResponseHeader::new(),
            info: String::new(),
        }
    }
}
