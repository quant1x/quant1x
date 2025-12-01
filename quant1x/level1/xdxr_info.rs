use super::BinaryStream;
use crate::level1::commands::*;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct XdxrInfoRequest {
    pub zip_flag: u8,
    pub seq_id: u32,
    pub packet_type: u8,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
    pub method: u16,
    pub market: u8,
    pub code: [u8; 6],
    pub padding: Vec<u8>,
}

impl XdxrInfoRequest {
    /// 从完整的证券代码字符串创建请求，例如 "sh600000" 或 "600000"。
    /// 功能等价于 C++ 的 DetectMarket：去除市场前缀并设置 market id。
    pub fn new(security_code: &str) -> Self {
        let mut code = [0u8; 6];
        let (_mid, _flag, pure) = crate::exchange::detect_market(security_code);
        let market = _mid;
        let bytes = pure.as_bytes();
        for i in 0..bytes.len().min(6) {
            code[i] = bytes[i];
        }
        XdxrInfoRequest {
            zip_flag: crate::level1::protocol::zlib_flag::UNCOMPRESSED,
            seq_id: super::sequence_id(),
            packet_type: 0x01,
            pkg_len1: 0,
            pkg_len2: 0,
            method: XDXR_INFO,
            market,
            code,
            padding: hex::decode("0100").unwrap_or_default(),
        }
    }

    pub fn serialize(&mut self) -> Vec<u8> {
        // payload = padding + market(1) + code(6)
        let payload_len = (self.padding.len() + 1 + self.code.len()) as u16;
        // pkg_len includes method (2) + payload
        self.pkg_len1 = 2u16 + payload_len;
        self.pkg_len2 = self.pkg_len1;

        // 构建与 C++ 中 RequestHeader::headerSerialize() 完全一致的头部
        let mut buf = BinaryStream::new();
        buf.push_u8(self.zip_flag);
        buf.push_u32(self.seq_id);
        buf.push_u8(self.packet_type);
        buf.push_u16(self.pkg_len1);
        buf.push_u16(self.pkg_len2);
        buf.push_u16(self.method);

        // payload
        buf.push_byte_array(&self.padding);
        buf.push_u8(self.market);
        buf.push_byte_array(&self.code);

        buf.data().clone()
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct XdxrInfo {
    #[serde(rename = "Date")]
    pub date: String,
    #[serde(rename = "Category")]
    pub category: u8,
    #[serde(rename = "Name")]
    pub name: String,
    #[serde(rename = "FenHong")]
    pub fenhong: f32,
    #[serde(rename = "PeiGuJia")]
    pub peigu_jia: f32,
    #[serde(rename = "SongZhuanGu")]
    pub songzhuan: f32,
    #[serde(rename = "PeiGu")]
    pub peigu: f32,
    #[serde(rename = "SuoGu")]
    pub suogu: f32,
    #[serde(rename = "QianLiuTong")]
    pub qian_liutong: f64,
    #[serde(rename = "HouLiuTong")]
    pub hou_liutong: f64,
    #[serde(rename = "QianZongGuBen")]
    pub qian_zonggu: f64,
    #[serde(rename = "HouZongGuBen")]
    pub hou_zonggu: f64,
    #[serde(rename = "FenShu")]
    pub fenshu: f32,
    #[serde(rename = "XingQuanJia")]
    pub xingquan_jia: f32,
}

impl XdxrInfo {
    /// 返回 CSV 表头，顺序与 C++ 及 datasets::xdxr 保持一致
    pub fn headers() -> &'static [&'static str] {
        &[
            "Date",
            "Category",
            "Name",
            "FenHong",
            "PeiGuJia",
            "SongZhuanGu",
            "PeiGu",
            "SuoGu",
            "QianLiuTong",
            "HouLiuTong",
            "QianZongGuBen",
            "HouZongGuBen",
            "FenShu",
            "XingQuanJia",
        ]
    }

    pub fn monetary_factor(&self) -> f64 {
        // 现金分红调整金额 = 每股分红 * 除权前总股本 / 10
        ((self.peigu as f64 * self.peigu_jia as f64) - self.fenhong as f64
            + (self.fenshu as f64 * self.xingquan_jia as f64))
            / 10.0
    }

    pub fn share_ratio_factor(&self) -> f64 {
        // 股本变动调整比例 = 除权后总股本 / 除权前总股本
        ((self.songzhuan as f64) + (self.peigu as f64) - (self.suogu as f64) + (self.fenshu as f64))
            / 10.0
    }

    /// 计算除权因子 (m, a)，与 C++ adjustFactor() 等价
    pub fn adjust_factor(&self) -> (f64, f64) {
        // A = (PeiGu * PeiGuJia - FenHong + FenShu * XingQuanJia) / 10.0
        // B = (SongZhuanGu + PeiGu - SuoGu + FenShu) / 10.0
        let a = self.monetary_factor();
        let b = self.share_ratio_factor();
        if (1.0 + b).abs() > 1e-10 {
            let m = 1.0 / (1.0 + b);
            let aa = a * m;
            (m, aa)
        } else {
            (1.0, 0.0)
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct XdxrInfoResponse {
    pub count: u16,
    pub list: Vec<XdxrInfo>,
}
impl XdxrInfoResponse {
    pub fn new() -> Self {
        Self {
            count: 0,
            list: Vec::new(),
        }
    }
    pub fn deserialize(&mut self, body: &[u8]) -> Result<(), crate::std::DeserializeError> {
        let mut bs = BinaryStream::from_vec(body.to_vec());
        bs.skip(9);
        self.count = bs.get_u16()?;
        // each entry uses 1+6+1+4+1+16 = 29 bytes
        let remaining = if body.len() > bs.position() {
            body.len() - bs.position()
        } else {
            0
        };
        let entry_size = 29usize;
        let max_entries = remaining / entry_size;
        let to_read = std::cmp::min(self.count as usize, max_entries);
        for _ in 0..to_read {
            let _market = bs.get_u8()?;
            let code = bs.get_string(6)?;
            let _unk = bs.get_u8()?;
            let date = bs.get_u32()?;
            let category = bs.get_u8()?;
            let mut data = [0u8; 16];
            bs.get_byte_array(&mut data)?;

            let (y, m, d, _hh, _mm) = super::get_datetime_from_u32(9 as i32, date, 0);
            let mut info = XdxrInfo {
                date: format!("{:04}-{:02}-{:02}", y, m, d),
                category,
                name: code.clone(),
                fenhong: 0.0,
                peigu_jia: 0.0,
                songzhuan: 0.0,
                peigu: 0.0,
                suogu: 0.0,
                qian_liutong: 0.0,
                hou_liutong: 0.0,
                qian_zonggu: 0.0,
                hou_zonggu: 0.0,
                fenshu: 0.0,
                xingquan_jia: 0.0,
            };

            let mut tmp = BinaryStream::from_vec(data.to_vec());
            match category as i32 {
                1 => {
                    info.fenhong = tmp.get_f32()?;
                    info.peigu_jia = tmp.get_f32()?;
                    info.songzhuan = tmp.get_f32()?;
                    info.peigu = tmp.get_f32()?;
                }
                11 | 12 => {
                    tmp.skip(8);
                    info.suogu = tmp.get_f32()?;
                }
                13 | 14 => {
                    info.xingquan_jia = tmp.get_f32()?;
                    tmp.skip(8);
                    info.fenshu = tmp.get_f32()?;
                }
                _ => {
                    let v1 = tmp.get_u32()?;
                    info.qian_liutong = super::int_to_float64(v1);
                    let v2 = tmp.get_u32()?;
                    info.qian_zonggu = super::int_to_float64(v2);
                    let v3 = tmp.get_u32()?;
                    info.hou_liutong = super::int_to_float64(v3);
                    let v4 = tmp.get_u32()?;
                    info.hou_zonggu = super::int_to_float64(v4);
                }
            }

            self.list.push(info);
        }
        Ok(())
    }
}

/// Fetch XDXR info for a single security code using the level1 client pool.
/// Returns Some(XdxrInfoResponse) on success, None on any error.
pub fn fetch_xdxr(code: &str) -> Option<XdxrInfoResponse> {
    // XdxrInfoRequest::new will detect market and pure code from the supplied string

    // Acquire a pooled client connection
    match crate::level1::client::get_std_conn() {
        Ok(mut pooled) => {
            // prepare request
            let mut req = XdxrInfoRequest::new(code);
            let req_buf = XdxrInfoRequest::serialize(&mut req);
            // process_request does the write/read and optional unzip
            match crate::level1::process_request(pooled.stream(), req_buf.as_slice())
                .map_err(|e| std::io::Error::new(std::io::ErrorKind::Other, e))
            {
                Ok(body) => {
                    let mut resp = XdxrInfoResponse::new();
                    if let Err(e) = resp.deserialize(&body) {
                        log::error!("level1::xdxr - deserialize error for {}: {}", code, e);
                        return None;
                    }
                    // Log response summary to help observation/diagnostics
                    log::info!("level1::xdxr - code={} count={}", code, resp.count);
                    for (i, it) in resp.list.iter().enumerate() {
                        log::debug!("level1::xdxr [{}] date={} category={} name={} fenhong={} peigu_jia={} songzhuan={} peigu={} suogu={} qian_liutong={} hou_liutong={} qian_zonggu={} hou_zonggu={} fenshu={} xingquan_jia={}",
                            i, it.date, it.category, it.name, it.fenhong, it.peigu_jia, it.songzhuan, it.peigu, it.suogu, it.qian_liutong, it.hou_liutong, it.qian_zonggu, it.hou_zonggu, it.fenshu, it.xingquan_jia);
                    }
                    Some(resp)
                }
                Err(e) => {
                    log::error!(
                        "level1 process_request error for {}: {}",
                        code,
                        e.to_string()
                    );
                    None
                }
            }
        }
        Err(e) => {
            log::error!("failed to acquire level1 client for {}: {}", code, e);
            None
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_xdxr_request_encoding_matches_cpp() {
        // Build request for code "sh600000" (we'll use "sh6000" 6-bytes) and force seq_id=3
        let mut req = XdxrInfoRequest::new("sh600000");
        req.seq_id = 3; // match the sequence observed in logs
        let buf = XdxrInfoRequest::serialize(&mut req);
        // Expected hex per C++ serializeImpl (little-endian fields):
        // header(12): 0c 03 00 00 00 01 0b 00 0b 00 0f 00
        // payload(9): 01 00 00 73 68 36 30 30 30
        let expected_hex = "0c03000000010b000b000f00010001363030303030";
        assert_eq!(hex::encode(&buf), expected_hex);
    }
}
