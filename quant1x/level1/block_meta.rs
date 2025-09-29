use super::BinaryStream;

#[derive(Debug, Clone)]
pub struct BlockMeta {
    pub size: u32,
    pub c1: u8,
    pub hash_value: [u8; 32],
    pub c2: u8,
}

impl BlockMeta {
    pub fn new() -> Self {
        Self {
            size: 0,
            c1: 0,
            hash_value: [0u8; 32],
            c2: 0,
        }
    }
}

#[derive(Debug, Clone)]
pub struct BlockMetaResponse {
    pub meta: BlockMeta,
}
#[allow(dead_code)]
impl BlockMetaResponse {
    pub fn new() -> Self {
        Self {
            meta: BlockMeta::new(),
        }
    }
    pub fn deserialize(&mut self, data: &[u8]) {
        let mut bs = BinaryStream::from_vec(data.to_vec());
        self.meta.size = bs.get_u32();
        self.meta.c1 = bs.get_u8();
        bs.get_byte_array(&mut self.meta.hash_value);
        self.meta.c2 = bs.get_u8();
    }
}
