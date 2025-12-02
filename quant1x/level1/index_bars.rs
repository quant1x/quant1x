use super::sequence_id;
use crate::level1::commands::*;
use crate::level1::protocol::{Request, RequestHeader, Response, ResponseHeader};
use crate::level1::security_bars::SecurityBarsResponse;
use crate::std::BinaryStream;

#[derive(Debug, Clone)]
pub struct IndexBarsRequest {
    header: RequestHeader,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
}
#[allow(dead_code)]
impl IndexBarsRequest {
    pub fn new() -> Self {
        let mut header = RequestHeader::new();
        header.zip_flag = crate::level1::protocol::zlib_flag::UNCOMPRESSED;
        header.seq_id = sequence_id();
        header.packet_type = 0x01;
        header.method = INDEX_BARS;
        IndexBarsRequest {
            header,
            pkg_len1: 0,
            pkg_len2: 0,
        }
    }
}

impl Request for IndexBarsRequest {
    fn header(&self) -> &RequestHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut RequestHeader {
        &mut self.header
    }

    fn serialize_payload(&mut self) -> Vec<u8> {
        let mut s = BinaryStream::new();
        s.push_u16(self.pkg_len1);
        s.push_u16(self.pkg_len2);
        s.data().clone()
    }

    fn payload_string(&self) -> String {
        format!("{{Len1:{}, Len2:{}}}", self.pkg_len1, self.pkg_len2)
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
}

impl Response for IndexBarsResponse {
    fn header(&self) -> &ResponseHeader {
        self.inner.header()
    }

    fn header_mut(&mut self) -> &mut ResponseHeader {
        self.inner.header_mut()
    }

    fn deserialize_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.inner.deserialize_body(data)
    }

    fn body_string(&self) -> String {
        self.inner.body_string()
    }
}
