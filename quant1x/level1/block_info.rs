#![allow(dead_code)]
use super::sequence_id;
use super::BinaryStream;
use crate::level1::commands::*;
use crate::level1::protocol::{Request, RequestHeader, Response, ResponseHeader};

// Request builder for BLOCK_DATA
#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct BlockInfoRequest {
    header: RequestHeader,
    pub start: u32,
    pub size: u32,
    pub block_filename: [u8; 100],
}

impl Request for BlockInfoRequest {
    fn header(&self) -> &RequestHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut RequestHeader {
        &mut self.header
    }

    fn serialize_payload(&mut self) -> Vec<u8> {
        let mut stream = BinaryStream::new();
        stream.push_u32(self.start);
        stream.push_u32(self.size);
        stream.push_byte_array(&self.block_filename);
        stream.data().clone()
    }

    fn payload_string(&self) -> String {
        format!(
            "BlockInfoRequest{{start:{}, size:{}}}",
            self.start, self.size
        )
    }
}

impl BlockInfoRequest {
    pub fn new(filename: &str, offset: u32) -> Self {
        let mut buf = [0u8; 100];
        let bytes = filename.as_bytes();
        let copy_len = std::cmp::min(bytes.len(), 99);
        buf[..copy_len].copy_from_slice(&bytes[..copy_len]);

        let mut header = RequestHeader::new();
        header.zip_flag = crate::level1::protocol::zlib_flag::UNCOMPRESSED;
        header.seq_id = sequence_id();
        header.packet_type = 0x01;
        header.method = BLOCK_DATA;

        BlockInfoRequest {
            header,
            start: offset,
            size: crate::level1::block_meta::BLOCK_CHUNKS_SIZE,
            block_filename: buf,
        }
    }
}

/// Fetch block data for filename and offset from level1 server.
pub fn fetch_block_info(filename: &str, offset: u32) -> Option<BlockInfoResponse> {
    match crate::level1::client::get_std_conn() {
        Ok(mut pooled) => {
            let mut req = BlockInfoRequest::new(filename, offset);
            let mut resp = BlockInfoResponse::new();
            match crate::level1::process(pooled.stream(), &mut req, &mut resp) {
                Ok(_) => Some(resp),
                Err(e) => {
                    log::error!(
                        "level1 process error for block_info {} offset {}: {}",
                        filename,
                        offset,
                        e.to_string()
                    );
                    None
                }
            }
        }
        Err(e) => {
            log::error!(
                "failed to acquire level1 client for block_info {} offset {}: {}",
                filename,
                offset,
                e
            );
            None
        }
    }
}

#[derive(Debug, Clone)]
pub struct BlockInfoResponse {
    header: ResponseHeader,
    pub size: u32,
    pub data: Vec<u8>,
}

impl Response for BlockInfoResponse {
    fn header(&self) -> &ResponseHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.header
    }

    fn deserialize_body(&mut self, body: &[u8]) -> Result<(), crate::std::DeserializeError> {
        let mut bs = BinaryStream::from_vec(body.to_vec());
        self.size = bs
            .get_u32()
            .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;
        if self.size > 0 {
            let pos = bs.position();
            let remain = bs.data();
            if (remain.len() as usize) > pos {
                self.data.clear();
                self.data.extend_from_slice(&remain[pos..]);
            }
        }
        Ok(())
    }

    fn body_string(&self) -> String {
        format!("BlockInfoResponse{{size:{}}}", self.size)
    }
}

#[allow(dead_code)]
impl BlockInfoResponse {
    pub fn new() -> Self {
        Self {
            header: ResponseHeader::new(),
            size: 0,
            data: Vec::new(),
        }
    }
}
