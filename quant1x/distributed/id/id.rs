//! 64 位可排序 ID (对应 Go 的 ID)
//!
//! 布局: `elapsed(41bit) << 22 | nodeID(workerBits) << seqBits | seq(seqBits)`
//! 编码: 8 字节大端 (Bytes), 11 字符 base64url 无填充 (String)

use std::fmt;
use std::str::FromStr;

use crate::distributed::id::Error;

/// 起始时间戳 (2026-01-01 00:00:00 UTC, 毫秒)
pub const EPOCH_MS: i64 = 1_767_225_600_000;

/// 低 22 位承载 payload (节点 + 序号)
pub(crate) const PAYLOAD_BITS: u8 = 22;
/// 高 41 位承载物理时间 (相对起始时间的毫秒数)
pub(crate) const PHYSICAL_BITS: u8 = 41;

/// 64 位可排序 ID
#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Debug, Default)]
pub struct Id(pub u64);

impl Id {
    /// 以大端序 8 字节返回 ID 的二进制表示
    pub fn bytes(self) -> [u8; 8] {
        self.0.to_be_bytes()
    }

    /// 从大端序 8 字节还原 ID
    pub fn from_bytes(bytes: [u8; 8]) -> Id {
        Id(u64::from_be_bytes(bytes))
    }

    /// 解析 11 字符 base64url (无填充) 字符串
    pub fn parse(s: &str) -> Result<Id, Error> {
        let bytes = decode_base64url(s).ok_or_else(|| Error::ParseId(s.to_string()))?;
        Ok(Id::from_bytes(bytes))
    }

    /// 相对起始时间的物理时间毫秒数
    pub fn physical(self) -> i64 {
        (self.0 >> PAYLOAD_BITS) as i64
    }

    /// 提取节点编号 (需传入 worker 位数)
    pub fn node_id(self, worker_bits: u8) -> u32 {
        let shift = PAYLOAD_BITS - worker_bits;
        ((self.0 >> shift) as u32) & ((1u32 << worker_bits) - 1)
    }

    /// 提取序号 (需传入 worker 位数)
    pub fn seq(self, worker_bits: u8) -> u32 {
        let shift = PAYLOAD_BITS - worker_bits;
        (self.0 as u32) & ((1u32 << shift) - 1)
    }

    /// 校验相对起始时间是否在 41 位可表示范围内
    pub fn check_epoch(elapsed: i64) -> Result<i64, Error> {
        if elapsed < 0 || elapsed >= (1i64 << PHYSICAL_BITS) {
            return Err(Error::EpochElapsedOutOfRange(elapsed));
        }
        Ok(elapsed)
    }
}

impl fmt::Display for Id {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", encode_base64url(self.0))
    }
}

impl FromStr for Id {
    type Err = Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        Id::parse(s)
    }
}

/// base64url 字符表 (无填充, 与 Go base64.RawURLEncoding 一致)
const BASE64URL_ALPHABET: &[u8; 64] =
    b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

/// 将 u64 编码为 11 字符 base64url 字符串 (无填充)
fn encode_base64url(value: u64) -> String {
    let bytes = value.to_be_bytes();
    let b = &bytes;
    let mut out = [0u8; 11];
    out[0] = BASE64URL_ALPHABET[(b[0] >> 2) as usize];
    out[1] = BASE64URL_ALPHABET[((b[0] & 0x03) << 4 | b[1] >> 4) as usize];
    out[2] = BASE64URL_ALPHABET[((b[1] & 0x0F) << 2 | b[2] >> 6) as usize];
    out[3] = BASE64URL_ALPHABET[(b[2] & 0x3F) as usize];
    out[4] = BASE64URL_ALPHABET[(b[3] >> 2) as usize];
    out[5] = BASE64URL_ALPHABET[((b[3] & 0x03) << 4 | b[4] >> 4) as usize];
    out[6] = BASE64URL_ALPHABET[((b[4] & 0x0F) << 2 | b[5] >> 6) as usize];
    out[7] = BASE64URL_ALPHABET[(b[5] & 0x3F) as usize];
    out[8] = BASE64URL_ALPHABET[(b[6] >> 2) as usize];
    out[9] = BASE64URL_ALPHABET[((b[6] & 0x03) << 4 | b[7] >> 4) as usize];
    // 末字符: byte[7] 的低 4 位置于 6 位值的高 4 位, 低 2 位补 0.
    // 这是标准 RawURLEncoding 的布局 (Go base64 / Python urlsafe_b64encode),
    // 早期实现把数据放在低 4 位, 与 Go/Python 产生的字符串不互通, 已修正.
    out[10] = BASE64URL_ALPHABET[((b[7] & 0x0F) << 2) as usize];
    String::from_utf8(out.to_vec()).expect("base64url output is always ASCII")
}

/// 将单个 base64url 字符解码为 6 位值, 非法字符返回 None
fn decode_base64url_char(c: u8) -> Option<u8> {
    match c {
        b'A'..=b'Z' => Some(c - b'A'),
        b'a'..=b'z' => Some(c - b'a' + 26),
        b'0'..=b'9' => Some(c - b'0' + 52),
        b'-' => Some(62),
        b'_' => Some(63),
        _ => None,
    }
}

/// 解码 11 字符 base64url (无填充) 字符串为 8 字节
///
/// 与 Go base64 解码一致: 最后一个字符仅取高 4 位 (>> 2), 低 2 位为编码补零, 忽略.
fn decode_base64url(s: &str) -> Option<[u8; 8]> {
    if s.len() != 11 {
        return None;
    }
    let mut v = [0u8; 11];
    for (i, c) in s.bytes().enumerate() {
        v[i] = decode_base64url_char(c)?;
    }
    Some([
        (v[0] << 2) | (v[1] >> 4),
        (v[1] << 4) | (v[2] >> 2),
        (v[2] << 6) | v[3],
        (v[4] << 2) | (v[5] >> 4),
        (v[5] << 4) | (v[6] >> 2),
        (v[6] << 6) | v[7],
        (v[8] << 2) | (v[9] >> 4),
        ((v[9] & 0x0F) << 4) | (v[10] >> 2),
    ])
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_bytes_round_trip() {
        let id = Id(0x1234_5678_9ABC_DEF0);
        assert_eq!(Id::from_bytes(id.bytes()), id);
    }

    #[test]
    fn test_string_round_trip() {
        for value in [0u64, 1, 42, 0x1234_5678_9ABC_DEF0, u64::MAX] {
            let id = Id(value);
            let s = id.to_string();
            assert_eq!(s.len(), 11, "id {value} encoded to {s}");
            let parsed = Id::parse(&s).unwrap();
            assert_eq!(parsed, id);
        }
    }

    /// 与 Go base64.RawURLEncoding / Python base64.urlsafe_b64encode 对照的标准向量
    ///
    /// 早期实现把末字符的数据放在低 4 位, 与 Go/Python 不互通; 这些断言用于
    /// 锁定跨语言兼容的编码布局, 防止回归.
    #[test]
    fn test_string_matches_go_python_vectors() {
        assert_eq!(Id(0).to_string(), "AAAAAAAAAAA");
        assert_eq!(Id(1).to_string(), "AAAAAAAAAAE");
        assert_eq!(Id(42).to_string(), "AAAAAAAAACo");
        assert_eq!(Id(0x1234_5678_9ABC_DEF0).to_string(), "EjRWeJq83vA");
        assert_eq!(Id(u64::MAX).to_string(), "__________8");
    }

    #[test]
    fn test_field_extraction() {
        // elapsed=123, node=7, seq=0  (worker_bits=11, seq_bits=11)
        let raw = (123u64 << 22) | (7u64 << 11);
        let id = Id(raw);
        assert_eq!(id.physical(), 123);
        assert_eq!(id.node_id(11), 7);
        assert_eq!(id.seq(11), 0);
    }

    #[test]
    fn test_check_epoch() {
        assert_eq!(Id::check_epoch(0).unwrap(), 0);
        assert_eq!(Id::check_epoch((1 << 41) - 1).unwrap(), (1 << 41) - 1);
        assert!(Id::check_epoch(-1).is_err());
        assert!(Id::check_epoch(1 << 41).is_err());
    }
}
