use crate::std::BinaryStream;

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
    pub count: u16,
    pub list: Vec<TickTransaction>,
    pub market_: i32,
    pub code_: String,
}

#[allow(dead_code)]
impl TransactionResponse {
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
        self.list.reserve(self.count as usize);
        let base_unit = super::default_base_unit(self.market_, &self.code_);
        let is_index = super::assert_index_by_market_and_code(self.market_, &self.code_);
        let mut last_price: i64 = 0;
        for _ in 0..self.count {
            let mut e = TickTransaction::new();
            let seconds = bs.get_u16();
            let h = seconds / 60;
            let m = seconds % 60;
            e.time = format!("{:02}:{:02}", h, m);
            let raw_price = bs.varint_decode();
            e.vol = bs.varint_decode();
            e.num = bs.varint_decode();
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

// Public helper that sends a TRANSACTION_DATA request and parses the response
pub fn fetch_transaction_data(
    security_code: &str,
    start: u16,
    count: u16,
) -> Option<TransactionResponse> {
    // Build request header + payload similar to C++ TransactionRequest
    #[derive(Debug)]
    struct TransactionRequest {
        zip_flag: u8,
        seq_id: u32,
        packet_type: u8,
        pkg_len1: u16,
        pkg_len2: u16,
        method: u16,

        market: u16,
        code: [u8; 6],
        start: u16,
        count: u16,
    }

    impl TransactionRequest {
        fn new(security_code: &str, start: u16, count: u16) -> Self {
            let (market_u8, _flag, pure) = crate::exchange::detect_market(security_code);
            let mut code = [0u8; 6];
            let sym = pure.as_bytes();
            let copy_len = std::cmp::min(sym.len(), 6);
            code[..copy_len].copy_from_slice(&sym[..copy_len]);

            TransactionRequest {
                zip_flag: 0x0C,
                seq_id: super::sequence_id(),
                packet_type: 0x00,
                pkg_len1: 0,
                pkg_len2: 0,
                method: 0x0fc5, // TRANSACTION_DATA
                market: market_u8 as u16,
                code,
                start,
                count,
            }
        }

        fn serialize(&mut self) -> Vec<u8> {
            // payload: Market(u16) + Code[6] + Start(u16) + Count(u16) -> 2+6+2+2 = 12
            // PkgLen fields in C++ for TransactionRequest are set to: 2(method) + 2(market) + 6(code) + 2(start) + 2(count) = 14
            self.pkg_len1 = 2u16 + 2u16 + 6u16 + 2u16 + 2u16; // method(2) + payload
            self.pkg_len2 = self.pkg_len1;

            let mut header = crate::std::BinaryStream::new();
            header.push_u8(self.zip_flag);
            header.push_u32(self.seq_id);
            header.push_u8(self.packet_type);
            header.push_u16(self.pkg_len1);
            header.push_u16(self.pkg_len2);
            header.push_u16(self.method);

            let mut stream = crate::std::BinaryStream::new();
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

    match crate::level1::client::client() {
        Ok(mut pooled) => {
            let mut req = TransactionRequest::new(security_code, start, count);
            let req_buf = req.serialize();
            match crate::level1::process_request(pooled.stream(), req_buf.as_slice()) {
                Ok(body) => {
                    let mut resp = TransactionResponse::new(
                        req.market as i32,
                        &String::from_utf8_lossy(&req.code),
                    );
                    resp.deserialize(&body);
                    Some(resp)
                }
                Err(e) => {
                    log::error!(
                        "level1 process_request error for transaction_data {}: {}",
                        security_code,
                        e
                    );
                    None
                }
            }
        }
        Err(e) => {
            log::error!(
                "failed to acquire level1 client for transaction_data {}: {}",
                security_code,
                e
            );
            None
        }
    }
}
