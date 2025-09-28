use crate::std::BinaryStream;
use encoding_rs::GBK;
use crate::level1::int_to_float64;

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct Security {
	pub code: String,
	pub vol_unit: u16,
	pub decimal_point: u8,
	pub name: String,
	pub pre_close: f64,
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityListResponse {
	pub count: u16,
	pub list: Vec<Security>,
}

#[allow(dead_code)]
impl SecurityListResponse {
	pub fn new() -> Self { Self { count: 0, list: Vec::new() } }

	pub fn deserialize(&mut self, data: &[u8]) {
		self.count = 0;
		self.list.clear();
		let mut bs = BinaryStream::from_vec(data.to_vec());
	// Count
	if bs.data().len().saturating_sub(bs.position()) < 2 { return; }
	self.count = bs.get_u16();

		for _ in 0..self.count {
			// Code: 6 bytes string
			let code = bs.get_string(6);
			// VolUnit: u16
			let vol_unit = bs.get_u16();
			// Name: 8 bytes, GBK -> UTF-8
			let name_raw = bs.get_string(8);
			let (cow, _, _) = GBK.decode(name_raw.as_bytes());
			let name = cow.into_owned();
			// Reversed1: 4 bytes skip
			let mut _rev1 = [0u8;4];
			bs.get_byte_array(&mut _rev1);
			// DecimalPoint
			let decimal_point = bs.get_u8();
			// PreClose: u32 -> IntToFloat64
			let tmp = bs.get_u32();
			let pre_close = int_to_float64(tmp);
			// Reversed2: 4 bytes skip
			let mut _rev2 = [0u8;4];
			bs.get_byte_array(&mut _rev2);

			self.list.push(Security {
				code,
				vol_unit,
				decimal_point,
				name,
				pre_close,
			});
		}
	}
}

