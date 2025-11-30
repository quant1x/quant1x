#![allow(dead_code)]
use super::sequence_id;
use super::BinaryStream;
use crate::level1::commands::*;

// Request builder for BLOCK_DATA
#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct BlockInfoRequest {
    pub zip_flag: u8,
    pub seq_id: u32,
    pub packet_type: u8,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
    pub method: u16,
    pub start: u32,
    pub size: u32,
    pub block_filename: [u8; 100],
}

impl BlockInfoRequest {
    pub fn new(filename: &str, offset: u32) -> Self {
        let mut buf = [0u8; 100];
        let bytes = filename.as_bytes();
        let copy_len = std::cmp::min(bytes.len(), 99);
        buf[..copy_len].copy_from_slice(&bytes[..copy_len]);

        BlockInfoRequest {
            zip_flag: 0x0C,
            seq_id: sequence_id(),
            packet_type: 0x01,
            pkg_len1: 0,
            pkg_len2: 0,
            method: BLOCK_DATA, // StdCommand::BLOCK_DATA
            start: offset,
            size: crate::level1::block_meta::BLOCK_CHUNKS_SIZE,
            block_filename: buf,
        }
    }

    pub fn serialize(&mut self) -> Vec<u8> {
        // fixed pkg len from C++: 0x6e
        self.pkg_len1 = 0x6e;
        self.pkg_len2 = 0x6e;

        let mut header = BinaryStream::new();
        header.push_u8(self.zip_flag);
        header.push_u32(self.seq_id);
        header.push_u8(self.packet_type);
        header.push_u16(self.pkg_len1);
        header.push_u16(self.pkg_len2);
        header.push_u16(self.method);

        let mut stream = BinaryStream::new();
        stream.push_u32(self.start);
        stream.push_u32(self.size);
        stream.push_byte_array(&self.block_filename);

        let mut buf = header.data().clone();
        let data = stream.data();
        buf.extend_from_slice(data);
        buf
    }
}

/// Fetch block data for filename and offset from level1 server.
pub fn fetch_block_info(filename: &str, offset: u32) -> Option<BlockInfoResponse> {
    match crate::level1::client::get_std_conn() {
        Ok(mut pooled) => {
            let mut req = BlockInfoRequest::new(filename, offset);
            let req_buf = req.serialize();
            match crate::level1::process_request(pooled.stream(), req_buf.as_slice()) {
                Ok(body) => {
                    let mut resp = BlockInfoResponse::new();
                    resp.deserialize(&body);
                    Some(resp)
                }
                Err(e) => {
                    log::error!(
                        "level1 process_request error for block_info {} offset {}: {}",
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
    pub size: u32,
    pub data: Vec<u8>,
}
#[allow(dead_code)]
impl BlockInfoResponse {
    pub fn new() -> Self {
        Self {
            size: 0,
            data: Vec::new(),
        }
    }
    pub fn deserialize(&mut self, body: &[u8]) {
        let mut bs = BinaryStream::from_vec(body.to_vec());
        self.size = bs.get_u32().expect("buffer error");
        if self.size > 0 {
            let pos = bs.position();
            let remain = bs.data();
            if (remain.len() as usize) > pos {
                self.data.clear();
                self.data.extend_from_slice(&remain[pos..]);
            }
        }
    }
}
