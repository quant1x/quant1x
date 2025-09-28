use crate::std::BinaryStream;
use super::sequence_id;

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityBarsRequest { pub zip_flag:u8, pub seq_id:u32, pub packet_type:u8, pub pkg_len1:u16, pub pkg_len2:u16, pub method:u16 }
#[allow(dead_code)]
impl SecurityBarsRequest {
	pub fn new() -> Self { SecurityBarsRequest { zip_flag:0x0C, seq_id: sequence_id(), packet_type:0x01, pkg_len1:0, pkg_len2:0, method:0x052d } }
	pub fn serialize(&mut self)->Vec<u8>{ let mut s = BinaryStream::new(); s.push_u8(self.zip_flag); s.push_u32(self.seq_id); s.push_u8(self.packet_type); s.push_u16(self.pkg_len1); s.push_u16(self.pkg_len2); s.push_u16(self.method); s.data().clone() }
}

#[derive(Debug, Clone)]
pub struct SecurityBar {
	pub open: f64,
	pub close: f64,
	pub high: f64,
	pub low: f64,
	pub vol: f64,
	pub amount: f64,
	pub year: i32,
	pub month: i32,
	pub day: i32,
	pub hour: i32,
	pub minute: i32,
	pub datetime: String,
	pub up_count: u16,
	pub down_count: u16,
}

impl SecurityBar {
	pub fn new() -> Self {
		Self {
			open: 0.0,
			close: 0.0,
			high: 0.0,
			low: 0.0,
			vol: 0.0,
			amount: 0.0,
			year: 0,
			month: 0,
			day: 0,
			hour: 0,
			minute: 0,
			datetime: String::new(),
			up_count: 0,
			down_count: 0,
		}
	}
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityBarsResponse {
	pub count: u16,
	pub list: Vec<SecurityBar>,
	// additional metadata typically provided by the request/response constructor
	pub is_index: bool,
	pub category: u16,
}

#[allow(dead_code)]
impl SecurityBarsResponse {
	pub fn new() -> Self { Self { count: 0, list: Vec::new(), is_index: false, category: 0 } }
	pub fn new_with(is_index: bool, category: u16) -> Self { Self { count: 0, list: Vec::new(), is_index, category } }

	pub fn deserialize(&mut self, data: &[u8]) {
		self.count = 0;
		self.list.clear();
		if data.len() < 2 { return; }
		let mut bs = BinaryStream::from_vec(data.to_vec());
		self.count = bs.get_u16();
		self.list.reserve(self.count as usize);

		let mut pre_diff_base: i64 = 0;
		for _ in 0..self.count {
			let mut e = SecurityBar::new();

			// decode date/time depending on category
			if (self.category as i32) < 4 || self.category == 7 || self.category == 8 {
				let zipday = bs.get_u16() as u32;
				let tminutes = bs.get_u16();
				let (y, m, d, hh, mm) = super::get_datetime_from_u32(self.category as i32, zipday, tminutes);
				e.year = y; e.month = m; e.day = d; e.hour = hh; e.minute = mm;
			} else {
				let zipday = bs.get_u32();
				let (y, m, d, hh, mm) = super::get_datetime_from_u32(self.category as i32, zipday, 0);
				e.year = y; e.month = m; e.day = d; e.hour = hh; e.minute = mm;
			}
			e.datetime = format!("{:04}-{:02}-{:02} {:02}:{:02}:00", e.year, e.month, e.day, e.hour, e.minute);

			// price diffs (varint encoded)
			let mut price_open_diff = bs.varint_decode();
			let price_close_diff = bs.varint_decode();
			let price_high_diff = bs.varint_decode();
			let price_low_diff = bs.varint_decode();

			let ivol = bs.get_u32();
			e.vol = super::int_to_float64(ivol);

			let dbvol = bs.get_u32();
			e.amount = super::int_to_float64(dbvol);

			// compute prices: values are divided by 1000.0 per C++ implementation
			e.open = (price_open_diff + pre_diff_base) as f64 / 1000.0;
			price_open_diff += pre_diff_base;

			e.close = (price_open_diff + price_close_diff) as f64 / 1000.0;
			e.high = (price_open_diff + price_high_diff) as f64 / 1000.0;
			e.low = (price_open_diff + price_low_diff) as f64 / 1000.0;

			pre_diff_base = price_open_diff + price_close_diff;

			if self.is_index {
				e.up_count = bs.get_u16();
				e.down_count = bs.get_u16();
			}

			self.list.push(e);
		}
	}
}
