use super::sequence_id;
use crate::level1::protocol::{commands, Request, RequestHeader, Response, ResponseHeader};
use crate::std::BinaryStream;

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct TransactionRequest {
    header: RequestHeader,
    market: u16,
    code: [u8; 6],
    start: u16,
    count: u16,
}

#[allow(dead_code)]
impl TransactionRequest {
    pub fn new(security_code: &str, start: u16, count: u16) -> Self {
        let mut header = RequestHeader::new();
        header.zip_flag = 0x0C;
        header.seq_id = sequence_id();
        header.packet_type = 0x00;
        header.method = commands::TRANSACTION_DATA;

        let (market_u8, _flag, pure) = crate::exchange::detect_market(security_code);
        let mut code = [0u8; 6];
        let sym = pure.as_bytes();
        let copy_len = std::cmp::min(sym.len(), 6);
        code[..copy_len].copy_from_slice(&sym[..copy_len]);

        Self {
            header,
            market: market_u8 as u16,
            code,
            start,
            count,
        }
    }

    pub fn market(&self) -> u16 {
        self.market
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

impl Request for TransactionRequest {
    fn header(&self) -> &RequestHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut RequestHeader {
        &mut self.header
    }

    fn serialize_payload(&mut self) -> Vec<u8> {
        let mut payload = BinaryStream::new();
        payload.push_u16(self.market);
        payload.push_byte_array(&self.code);
        payload.push_u16(self.start);
        payload.push_u16(self.count);
        payload.data().clone()
    }

    fn payload_string(&self) -> String {
        format!(
            "{{Market:{}, Code:{}, Start:{}, Count:{}}}",
            self.market,
            self.code_string(),
            self.start,
            self.count
        )
    }
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct TickTransaction {
    pub time: String,
    pub price: f64,
    pub vol: i64,
    pub num: i64,
    pub amount: f64,
    pub buy_or_sell: i64,
}

impl TickTransaction {
    pub fn new() -> Self {
        Self {
            time: String::new(),
            price: 0.0,
            vol: 0,
            num: 0,
            amount: 0.0,
            buy_or_sell: 0,
        }
    }
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct TransactionResponse {
    header: ResponseHeader,
    pub count: u16,
    pub list: Vec<TickTransaction>,
    pub market_: i32,
    pub code_: String,
}

#[allow(dead_code)]
impl TransactionResponse {
    pub fn new(market: i32, code: &str) -> Self {
        Self {
            header: ResponseHeader::new(),
            count: 0,
            list: Vec::new(),
            market_: market,
            code_: code.to_string(),
        }
    }

    pub fn new_from_request(req: &TransactionRequest) -> Self {
        Self::new(req.market() as i32, &req.code_string())
    }
}

impl Response for TransactionResponse {
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

        let min_required = 2 + (self.count as usize) * 5;
        if data.len() < min_required {
            log::warn!(
                "insufficient data for {} transactions: data len {}, min required {}",
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

        for _ in 0..self.count {
            let mut entry = TickTransaction::new();
            let seconds = bs.get_u16();
            let h = seconds / 60;
            let m = seconds % 60;
            entry.time = format!("{:02}:{:02}", h, m);
            let raw_price = bs.varint_decode();
            entry.vol = bs.varint_decode();
            entry.num = bs.varint_decode();
            entry.buy_or_sell = bs.varint_decode();
            last_price += raw_price;
            entry.price = (last_price as f64) / base_unit;

            if is_index {
                let amount = (entry.vol as i64) * 100;
                entry.amount = amount as f64;
                if entry.price != 0.0 {
                    entry.vol = (entry.amount / entry.price) as i64;
                } else {
                    entry.vol = 0;
                }
            } else {
                entry.vol *= 100;
                entry.amount = (entry.vol as f64) * entry.price;
            }

            let _skip = bs.varint_decode();
            self.list.push(entry);
        }
    }

    fn body_string(&self) -> String {
        format!(
            "{{Count:{}, Market:{}, Code:{}}}",
            self.count, self.market_, self.code_
        )
    }
}
