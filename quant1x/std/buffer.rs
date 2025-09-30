use std::convert::TryInto;

#[derive(Debug, Clone)]
pub struct BinaryStream {
    buffer: Vec<u8>,
    offset: usize,
}

impl BinaryStream {
    pub fn new() -> Self {
        Self {
            buffer: Vec::new(),
            offset: 0,
        }
    }
    pub fn from_vec(v: Vec<u8>) -> Self {
        Self {
            buffer: v,
            offset: 0,
        }
    }

    fn ensure_capacity(&mut self, required: usize) {
        if required > self.buffer.len() {
            self.buffer.resize(required, 0);
        }
    }

    fn check_available(&self, required: usize) {
        if required > self.buffer.len() || self.offset > self.buffer.len().saturating_sub(required) {
            panic!("Insufficient data in buffer: buffer_len={}, offset={}, required={}", self.buffer.len(), self.offset, required);
        }
    }

    // primitive writes
    pub fn push_u8(&mut self, v: u8) {
        self.ensure_capacity(self.offset + 1);
        self.buffer[self.offset] = v;
        self.offset += 1;
    }
    pub fn push_i8(&mut self, v: i8) {
        self.push_u8(v as u8);
    }

    pub fn push_u16(&mut self, v: u16) {
        let bytes = v.to_le_bytes();
        self.push_bytes(&bytes);
    }
    pub fn push_i16(&mut self, v: i16) {
        self.push_u16(v as u16);
    }

    pub fn push_u32(&mut self, v: u32) {
        let bytes = v.to_le_bytes();
        self.push_bytes(&bytes);
    }
    pub fn push_i32(&mut self, v: i32) {
        self.push_u32(v as u32);
    }

    pub fn push_u64(&mut self, v: u64) {
        let bytes = v.to_le_bytes();
        self.push_bytes(&bytes);
    }
    pub fn push_i64(&mut self, v: i64) {
        self.push_u64(v as u64);
    }

    pub fn push_f32(&mut self, v: f32) {
        self.push_u32(v.to_bits());
    }
    pub fn push_f64(&mut self, v: f64) {
        self.push_u64(v.to_bits());
    }

    fn push_bytes(&mut self, bs: &[u8]) {
        let n = bs.len();
        self.ensure_capacity(self.offset + n);
        self.buffer[self.offset..self.offset + n].copy_from_slice(bs);
        self.offset += n;
    }

    // primitive reads
    pub fn get_u8(&mut self) -> u8 {
        self.check_available(1);
        let v = self.buffer[self.offset];
        self.offset += 1;
        v
    }
    pub fn get_i8(&mut self) -> i8 {
        self.get_u8() as i8
    }

    pub fn get_u16(&mut self) -> u16 {
        self.check_available(2);
        let b: [u8; 2] = self.buffer[self.offset..self.offset + 2]
            .try_into()
            .unwrap();
        self.offset += 2;
        u16::from_le_bytes(b)
    }
    pub fn get_i16(&mut self) -> i16 {
        self.get_u16() as i16
    }

    pub fn get_u32(&mut self) -> u32 {
        self.check_available(4);
        let b: [u8; 4] = self.buffer[self.offset..self.offset + 4]
            .try_into()
            .unwrap();
        self.offset += 4;
        u32::from_le_bytes(b)
    }
    pub fn get_i32(&mut self) -> i32 {
        self.get_u32() as i32
    }

    pub fn get_u64(&mut self) -> u64 {
        self.check_available(8);
        let b: [u8; 8] = self.buffer[self.offset..self.offset + 8]
            .try_into()
            .unwrap();
        self.offset += 8;
        u64::from_le_bytes(b)
    }
    pub fn get_i64(&mut self) -> i64 {
        self.get_u64() as i64
    }

    pub fn get_f32(&mut self) -> f32 {
        f32::from_bits(self.get_u32())
    }
    pub fn get_f64(&mut self) -> f64 {
        f64::from_bits(self.get_u64())
    }

    // byte array ops
    pub fn push_byte_array(&mut self, data: &[u8]) {
        self.push_bytes(data);
    }
    pub fn get_byte_array(&mut self, out: &mut [u8]) {
        let n = out.len();
        self.check_available(n);
        out.copy_from_slice(&self.buffer[self.offset..self.offset + n]);
        self.offset += n;
    }

    // length-prefixed string (u32 length)
    pub fn push_length_prefixed_string(&mut self, s: &str) {
        let len = s.len() as u32;
        self.push_u32(len);
        self.push_byte_array(s.as_bytes());
    }
    pub fn get_length_prefixed_string(&mut self) -> String {
        let len = self.get_u32() as usize;
        self.check_available(len);
        let s = String::from_utf8(self.buffer[self.offset..self.offset + len].to_vec()).unwrap();
        self.offset += len;
        s
    }

    // raw string with fixed len (truncate at first NUL)
    pub fn get_string(&mut self, len: usize) -> String {
        self.check_available(len);
        let slice = &self.buffer[self.offset..self.offset + len];
        let nul_pos = slice.iter().position(|&b| b == 0).unwrap_or(len);
        // tolerate non-UTF8 by using a lossy conversion (matches C++ std::string behavior)
        let s = String::from_utf8_lossy(&slice[..nul_pos]).into_owned();
        self.offset += len;
        s
    }

    // varint decoding (same format as C++ version)
    pub fn varint_decode(&mut self) -> i64 {
        let mut pos = self.offset;
        if pos >= self.buffer.len() {
            panic!("Insufficient data in buffer for varint");
        }
        let mut byte = self.buffer[pos];
        pos += 1;
        let sign = (byte & 0x40) != 0;
        let mut data: u64 = (byte & 0x3F) as u64;
        let mut shift: u32 = 6;
        while (byte & 0x80) != 0 {
            if pos >= self.buffer.len() {
                panic!("Insufficient data in buffer during varint decoding");
            }
            byte = self.buffer[pos];
            pos += 1;
            data |= ((byte & 0x7F) as u64) << shift;
            shift = shift.saturating_add(7);
            if shift >= 64 {
                break;
            }
        }
        self.offset = pos;
        let signed: i64 = if data > (i64::MAX as u64) {
            i64::MAX
        } else {
            data as i64
        };
        if sign {
            -signed
        } else {
            signed
        }
    }

    // utilities
    pub fn position(&self) -> usize {
        self.offset
    }
    pub fn seek(&mut self, new_offset: usize) {
        self.offset = new_offset
    }
    pub fn skip(&mut self, off: usize) {
        self.offset = self.offset.saturating_add(off)
    }
    pub fn data(&self) -> &Vec<u8> {
        &self.buffer
    }
    pub fn clear(&mut self) {
        self.buffer.clear();
        self.offset = 0
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_push_get_ints() {
        let mut s = BinaryStream::new();
        s.push_i8(-5);
        s.push_u8(250);
        s.push_i16(-300);
        s.push_u16(60000);
        s.push_i32(-70000);
        s.push_u32(4000000000);
        s.push_i64(-900000000000);
        s.push_u64(9000000000000);
        s.push_f32(3.14);
        s.push_f64(-2.71828);

        s.seek(0);
        assert_eq!(s.get_i8(), -5);
        assert_eq!(s.get_u8(), 250);
        assert_eq!(s.get_i16(), -300);
        assert_eq!(s.get_u16(), 60000);
        assert_eq!(s.get_i32(), -70000);
        assert_eq!(s.get_u32(), 4000000000);
        assert_eq!(s.get_i64(), -900000000000);
        assert_eq!(s.get_u64(), 9000000000000);
        let f = s.get_f32();
        assert!((f - 3.14).abs() < 1e-6);
        let d = s.get_f64();
        assert!((d + 2.71828).abs() < 1e-12);
    }

    #[test]
    fn test_varint() {
        let mut s = BinaryStream::new();
        // encode a few varints manually (using format from C++):
        // small positive: 0b0 00xxxxxx (no continuation)
        s.push_u8(0x05);
        // negative small: sign bit 0x40
        s.push_u8(0x40 | 0x03); // -3
                                // multi-byte: set continuation bits
        s.push_u8(0x80 | 0x01); // continuation
        s.push_u8(0x02);

        s.seek(0);
        assert_eq!(s.varint_decode(), 5);
        assert_eq!(s.varint_decode(), -3);
        // first byte payload = 1, second byte contributes at shift 6 -> 2<<6 == 128
        assert_eq!(s.varint_decode(), 129);
    }
}
