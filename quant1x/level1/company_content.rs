pub struct CompanyContentResponse { pub content: String }
#[allow(dead_code)]
impl CompanyContentResponse {
	pub fn new() -> Self { Self { content: String::new() } }
	pub fn deserialize(&mut self, data: &[u8]) {
		// Store a lossy string representation for textual payloads.
		self.content = String::from_utf8_lossy(data).into_owned();
	}
}
