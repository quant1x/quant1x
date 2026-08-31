//! CRC32-IEEE 校验 (与 Go hash/crc32 ChecksumIEEE 一致, 多项式 0xEDB88320)

/// 查表法 CRC32 (IEEE 多项式, 初值 0xFFFFFFFF, 结果取反)
/// 与 Go `crc32.ChecksumIEEE` 输出一致, 保证状态文件格式跨语言兼容.
pub(crate) fn crc32_ieee(data: &[u8]) -> u32 {
    let mut crc: u32 = 0xFFFF_FFFF;
    for &byte in data {
        let index = ((crc ^ byte as u32) & 0xFF) as usize;
        crc = (crc >> 8) ^ CRC32_IEEE_TABLE[index];
    }
    crc ^ 0xFFFF_FFFF
}

/// CRC32-IEEE 查表 (256 项, 反射多项式 0xEDB88320)
static CRC32_IEEE_TABLE: [u32; 256] = build_crc32_table();

const fn build_crc32_table() -> [u32; 256] {
    let mut table = [0u32; 256];
    let mut n = 0;
    while n < 256 {
        let mut c = n as u32;
        let mut k = 0;
        while k < 8 {
            if c & 1 != 0 {
                c = 0xEDB8_8320 ^ (c >> 1);
            } else {
                c >>= 1;
            }
            k += 1;
        }
        table[n] = c;
        n += 1;
    }
    table
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_known_vectors() {
        // 与 Go crc32.ChecksumIEEE 的已知输出对照
        assert_eq!(crc32_ieee(b""), 0x0000_0000);
        assert_eq!(crc32_ieee(b"123456789"), 0xCBF4_3926);
        assert_eq!(crc32_ieee(b"The quick brown fox jumps over the lazy dog"), 0x414F_A339);
    }
}
