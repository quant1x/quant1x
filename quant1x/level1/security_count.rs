use crate::std::BinaryStream;

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityCountResponse { pub count: usize }
#[allow(dead_code)]
impl SecurityCountResponse {
	pub fn new() -> Self { Self { count: 0 } }
	pub fn deserialize(&mut self, data: &[u8]) {
		if data.len() < 2 { return; }
		let mut bs = BinaryStream::from_vec(data.to_vec());
		let c = bs.get_u16();
		self.count = c as usize;
	}
}
