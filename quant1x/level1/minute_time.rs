use crate::std::BinaryStream;

#[derive(Debug, Clone)]
pub struct MinuteTime {
	pub price: f32,
	pub vol: i64,
}

impl MinuteTime {
	pub fn new() -> Self { Self { price: 0.0, vol: 0 } }
}

#[derive(Debug, Clone)]
pub struct MinuteTimeResponse {
	pub count: u16,
	pub list: Vec<MinuteTime>,
	pub market_: i32,
	pub code_: String,
}

#[allow(dead_code)]
impl MinuteTimeResponse {
	pub fn new(market: i32, code: &str) -> Self {
		Self { count: 0, list: Vec::new(), market_: market, code_: code.to_string() }
	}

	pub fn deserialize(&mut self, data: &[u8]) {
		if data.len() < 2 { return; }
		let mut bs = BinaryStream::from_vec(data.to_vec());
		self.count = bs.get_u16();
		self.list.reserve(self.count as usize);
		let base_unit = super::default_base_unit(self.market_, &self.code_);
		let _is_index = super::assert_index_by_market_and_code(self.market_, &self.code_);
		let mut last_price: i64 = 0;
		// skip 4 bytes as C++ does for history minute header
		bs.skip(4);
		for _ in 0..self.count {
			let mut e = MinuteTime::new();
			let raw_price = bs.varint_decode();
			let _reversed1 = bs.varint_decode(); // ignored
			let vol = bs.varint_decode();
			e.vol = vol;
			last_price += raw_price;
			e.price = (last_price as f32) / (base_unit as f32);
			self.list.push(e);
		}
	}
}
