use super::sequence_id;
use crate::level1::protocol::{self, commands, Request, RequestHeader, Response, ResponseHeader};
use crate::level1::transaction_data::TickTransaction;
use crate::std::BinaryStream;

// Request builder for HISTORY_TRANSACTION_DATA (aligns with C++ HistoryTransactionRequest)
#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct HistoryTransactionRequest {
    header: RequestHeader,
    date: u32,
    market: u16,
    code: [u8; 6],
    start: u16,
    count: u16,
}

impl HistoryTransactionRequest {
    pub fn new(security_code: &str, date: u32, start: u16, count: u16) -> Self {
        let mut header = RequestHeader::new();
        header.zip_flag = 0x0C;
        header.seq_id = sequence_id();
        header.packet_type = 0x00;
        header.method = commands::HISTORY_TRANSACTION_DATA;

        let (market_u8, _flag, pure) = crate::exchange::detect_market(security_code);
        let mut code = [0u8; 6];
        let sym = pure.as_bytes();
        let copy_len = std::cmp::min(sym.len(), 6);
        code[..copy_len].copy_from_slice(&sym[..copy_len]);

        Self {
            header,
            date,
            market: market_u8 as u16,
            code,
            start,
            count,
        }
    }

    pub fn date(&self) -> u32 {
        self.date
    }

    pub fn market(&self) -> u16 {
        self.market
    }

    pub fn code(&self) -> &[u8; 6] {
        &self.code
    }

    pub fn code_string(&self) -> String {
        let nul_pos = self
            .code
            .iter()
            .position(|&b| b == 0)
            .unwrap_or(self.code.len());
        String::from_utf8_lossy(&self.code[..nul_pos]).into_owned()
    }

    pub fn start(&self) -> u16 {
        self.start
    }

    pub fn count(&self) -> u16 {
        self.count
    }
}

impl Request for HistoryTransactionRequest {
    fn header(&self) -> &RequestHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut RequestHeader {
        &mut self.header
    }

    fn serialize_payload(&mut self) -> Vec<u8> {
        let mut payload = BinaryStream::new();
        payload.push_u32(self.date);
        payload.push_u16(self.market);
        payload.push_byte_array(&self.code);
        payload.push_u16(self.start);
        payload.push_u16(self.count);
        payload.data().clone()
    }

    fn payload_string(&self) -> String {
        format!(
            "{{Date:{}, Market:{}, Code:{}, Start:{}, Count:{}}}",
            self.date,
            self.market,
            self.code_string(),
            self.start,
            self.count
        )
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
            let mut request = HistoryTransactionRequest::new(security_code, date, start, count);
            let mut response = TransactionHistoryResponse::new_from_request(&request);
            match protocol::process(pooled.stream(), &mut request, &mut response) {
                Ok(_) => Some(response),
                Err(e) => {
                    log::error!(
                        "level1 protocol::process error for history_transaction {} date {}: {}",
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
    header: ResponseHeader,
    pub count: u16,
    pub list: Vec<TickTransaction>,
    pub market_: i32,
    pub code_: String,
}

#[allow(dead_code)]
impl TransactionHistoryResponse {
    pub fn new(market: i32, code: &str) -> Self {
        Self {
            header: ResponseHeader::new(),
            count: 0,
            list: Vec::new(),
            market_: market,
            code_: code.to_string(),
        }
    }

    pub fn new_from_request(req: &HistoryTransactionRequest) -> Self {
        Self::new(req.market() as i32, &req.code_string())
    }
}

impl Response for TransactionHistoryResponse {
    fn header(&self) -> &ResponseHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.header
    }

    fn deserialize_body(&mut self, data: &[u8]) {
        self.list.clear();
        self.count = 0;

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
            self.count = 0;
            return;
        }

        self.list.reserve(self.count as usize);
        let base_unit = super::default_base_unit(self.market_, &self.code_);
        let is_index = crate::exchange::assert_index_by_market_and_code(self.market_ as u8, &self.code_);
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
                    e.vol = (e.amount / e.price) as i64;
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

    fn body_string(&self) -> String {
        format!(
            "{{Count:{}, Market:{}, Code:{}}}",
            self.count, self.market_, self.code_
        )
    }
}
