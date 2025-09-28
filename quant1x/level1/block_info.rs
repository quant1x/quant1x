use super::BinaryStream;

#[derive(Debug, Clone)]
pub struct BlockInfoResponse { pub size: u32, pub data: Vec<u8> }
#[allow(dead_code)]
impl BlockInfoResponse {
	pub fn new() -> Self { Self { size: 0, data: Vec::new() } }
	pub fn deserialize(&mut self, body: &[u8]) {
		let mut bs = BinaryStream::from_vec(body.to_vec());
		self.size = bs.get_u32();
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
