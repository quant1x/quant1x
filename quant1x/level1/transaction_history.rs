use super::sequence_id;
use crate::level1::commands::*;
use crate::level1::transaction_data::TickTransaction;
use crate::std::BinaryStream;

// Request builder for HISTORY_TRANSACTION_DATA (aligns with C++ HistoryTransactionRequest)
#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct HistoryTransactionRequest {
    pub zip_flag: u8,
    pub seq_id: u32,
    pub packet_type: u8,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
    pub method: u16,

    pub date: u32,
    pub market: u16, // NOTE: C++ uses uint16_t here; ensure u16 to match protocol
    pub code: [u8; 6],
    pub start: u16,
    pub count: u16,
}

impl HistoryTransactionRequest {
    pub fn new(security_code: &str, date: u32, start: u16, count: u16) -> Self {
        let (market_u8, _flag, pure) = crate::exchange::detect_market(security_code);
        let mut code = [0u8; 6];
        let sym = pure.as_bytes();
        let copy_len = std::cmp::min(sym.len(), 6);
        code[..copy_len].copy_from_slice(&sym[..copy_len]);

        HistoryTransactionRequest {
            zip_flag: 0x0C,
            seq_id: sequence_id(),
            packet_type: 0x00,
            pkg_len1: 0,
            pkg_len2: 0,
            method: HISTORY_TRANSACTION_DATA,
            date,
            market: market_u8 as u16, // cast to u16 to match C++
            code,
            start,
            count,
        }
    }

    pub fn serialize(&mut self) -> Vec<u8> {
        // payload: Date(u32) + Market(u16) + Code[6] + Start(u16) + Count(u16) -> 4+2+6+2+2 = 16
        self.pkg_len1 = 2u16 + 4u16 + 2u16 + 6u16 + 2u16 + 2u16;
        self.pkg_len2 = self.pkg_len1;

        let mut header = BinaryStream::new();
        header.push_u8(self.zip_flag);
        header.push_u32(self.seq_id);
        header.push_u8(self.packet_type);
        header.push_u16(self.pkg_len1);
        header.push_u16(self.pkg_len2);
        header.push_u16(self.method);

        let mut stream = BinaryStream::new();
        stream.push_u32(self.date);
        stream.push_u16(self.market);
        stream.push_byte_array(&self.code);
        stream.push_u16(self.start);
        stream.push_u16(self.count);

        let mut buf = header.data().clone();
        let data = stream.data();
        buf.extend_from_slice(data);
        buf
    }
}

/// Public helper that sends a HISTORY_TRANSACTION_DATA request and parses the response
pub fn fetch_history_transactions(
    security_code: &str,
    date: u32,
    start: u16,
    count: u16,
) -> Option<TransactionHistoryResponse> {
    match crate::level1::client::client() {
        Ok(mut pooled) => {
            let mut req = HistoryTransactionRequest::new(security_code, date, start, count);
            let req_buf = req.serialize();
            match crate::level1::process_request(pooled.stream(), req_buf.as_slice()) {
                Ok(body) => {
                    let mut resp = TransactionHistoryResponse::new(
                        req.market as i32,
                        &String::from_utf8_lossy(&req.code),
                    );
                    resp.deserialize(&body);
                    Some(resp)
                }
                Err(e) => {
                    log::error!(
                        "level1 process_request error for history_transaction {} date {}: {}",
                        security_code,
                        date,
                        e
                    );
                    None
                }
            }
        }
        Err(e) => {
            log::error!(
                "failed to acquire level1 client for history_transaction {} date {}: {}",
                security_code,
                date,
                e
            );
            None
        }
    }
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct TransactionHistoryResponse {
    pub count: u16,
    pub list: Vec<TickTransaction>,
    pub market_: i32,
    pub code_: String,
}

#[allow(dead_code)]
impl TransactionHistoryResponse {
    pub fn new(market: i32, code: &str) -> Self {
        Self {
            count: 0,
            list: Vec::new(),
            market_: market,
            code_: code.to_string(),
        }
    }

    pub fn deserialize(&mut self, data: &[u8]) {
        if data.len() < 2 {
            return;
        }
        let mut bs = BinaryStream::from_vec(data.to_vec());
        self.count = bs.get_u16();
        if self.count == 0 {
            return;
        }
        // Rough estimate: header 6 bytes + each transaction ~5 bytes (u16 + 5 varints)
        let min_required = 6 + (self.count as usize) * 5;
        if data.len() < min_required {
            log::warn!(
                "insufficient data for {} historical transactions: data len {}, min required {}",
                self.count,
                data.len(),
                min_required
            );
            return;
        }
        self.list.reserve(self.count as usize);
        let base_unit = super::default_base_unit(self.market_, &self.code_);
        let is_index = super::assert_index_by_market_and_code(self.market_, &self.code_);
        let mut last_price: i64 = 0;
        // skip 4 bytes as in C++ implementation
        bs.skip(4);
        for _ in 0..self.count {
            let mut e = TickTransaction::new();
            let minutes = bs.get_u16();
            let h = minutes / 60;
            let m = minutes % 60;
            e.time = format!("{:02}:{:02}", h, m);
            let raw_price = bs.varint_decode();
            e.vol = bs.varint_decode();
            // historical tick has no num field in C++ version
            e.buy_or_sell = bs.varint_decode();
            last_price += raw_price;
            e.price = (last_price as f64) / base_unit;
            if is_index {
                let amount = (e.vol as i64) * 100;
                e.amount = amount as f64;
                if e.price != 0.0 {
                    e.vol = ((e.amount) / e.price) as i64;
                } else {
                    e.vol = 0;
                }
            } else {
                e.vol *= 100;
                e.amount = (e.vol as f64) * e.price;
            }
            let _skip = bs.varint_decode();
            self.list.push(e);
        }
    }
}
