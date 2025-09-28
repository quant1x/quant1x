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
		Self { time: String::new(), price: 0.0, vol: 0, num: 0, amount: 0.0, buy_or_sell: 0 }
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
		Self { count: 0, list: Vec::new(), market_: market, code_: code.to_string() }
	}

	pub fn deserialize(&mut self, data: &[u8]) {
		if data.len() < 2 { return; }
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
