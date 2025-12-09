use super::sequence_id;
use super::BinaryStream;
use crate::level1::commands::*;
use crate::level1::protocol::{Request, RequestHeader, Response, ResponseHeader};

#[derive(Debug, Clone)]
pub struct HeartbeatRequest {
    pub header: RequestHeader,
}

impl Request for HeartbeatRequest {
    fn header(&self) -> &RequestHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut RequestHeader {
        &mut self.header
    }

    fn serialize_payload(&mut self) -> Vec<u8> {
        self.header.pkg_len1 = 2;
        self.header.pkg_len2 = 2;
        BinaryStream::new().data().clone()
    }

    fn payload_string(&self) -> String {
        "HeartbeatRequest".to_string()
    }
}

impl HeartbeatRequest {
    pub fn new() -> Self {
        let mut header = RequestHeader::new();
        header.zip_flag = crate::level1::protocol::zlib_flag::UNCOMPRESSED;
        header.seq_id = sequence_id();
        header.packet_type = 0x02;
        header.method = HEARTBEAT;

        HeartbeatRequest { header }
    }
}

#[derive(Debug, Clone)]
pub struct HeartbeatResponse {
    pub header: ResponseHeader,
    pub info: String,
}

impl Response for HeartbeatResponse {
    fn header(&self) -> &ResponseHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.header
    }

    fn deserialize_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        let mut bs = BinaryStream::from_vec(data.to_vec());
        // The original code read 10 bytes string?
        // "self.info = bs.get_string(10)?;"
        // Assuming the body contains this.
        self.info = bs.get_string(10)?;
        Ok(())
    }

    fn body_string(&self) -> String {
        format!("Info: {}", self.info)
    }
}

impl HeartbeatResponse {
    pub fn new() -> Self {
        Self {
            header: ResponseHeader::new(),
            info: String::new(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn deserialize_sample_populates_info() {
        let hex_data = "48656172742d486562696f726974";
        let buf = hex::decode(hex_data).unwrap();
        let mut resp = HeartbeatResponse::new();
        resp.deserialize(&buf).unwrap();
        assert!(!resp.info.is_empty());
    }
}
