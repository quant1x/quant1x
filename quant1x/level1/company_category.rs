pub struct CompanyCategoryResponse { pub content: String }
#[allow(dead_code)]
impl CompanyCategoryResponse {
	pub fn new() -> Self { Self { content: String::new() } }

	pub fn deserialize(&mut self, data: &[u8]) {
		// company category payloads are textual in some providers; store a lossy string so callers can inspect.
		self.content = String::from_utf8_lossy(data).into_owned();
	}
}
