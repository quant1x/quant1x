use crate::std::BinaryStream;
use std::sync::atomic::{AtomicU32, Ordering};
use flate2::read::ZlibDecoder;
use std::io::Read;
use std::io::Write;
use mio::net::TcpStream as MioTcpStream;

// Global sequence id to mimic C++ SequenceId()
static SEQ_ID: AtomicU32 = AtomicU32::new(0);

pub fn sequence_id() -> u32 {
    // Pre-increment semantics: ++seq
    SEQ_ID.fetch_add(1, Ordering::SeqCst).wrapping_add(1)
}

// helper: mimic level1::helpers::GetDatetimeFromUint32
pub fn get_datetime_from_u32(category: i32, zipday: u32, tminutes: u16) -> (i32, i32, i32, i32, i32) {
    if category < 4 || category == 7 || category == 8 {
        let year = ((zipday >> 11) as i32) + 2004;
        let rem = (zipday % 2048) as i32;
        let month = rem / 100;
        let day = rem % 100;
        let hour = (tminutes / 60) as i32;
        let minute = (tminutes % 60) as i32;
        (year, month, day, hour, minute)
    } else {
        let year = (zipday / 10000) as i32;
        let month = ((zipday % 10000) / 100) as i32;
        let day = (zipday % 100) as i32;
        (year, month, day, 15, 0)
    }
}

// helper: mimic level1::helpers::IntToFloat64
pub fn int_to_float64(v: u32) -> f64 {
    if v == 0 { return 0.0; }
    let uinteger = v;
    let log_point = ((uinteger >> 24) & 0xFF) as i32;
    let hleax = ((uinteger >> 16) & 0xFF) as i32;
    let lheax = ((uinteger >> 8) & 0xFF) as i32;
    let lleax = (uinteger & 0xFF) as i32;

    let dw_ecx = log_point * 2 - 0x7F;
    let dw_edx = log_point * 2 - 0x86;
    let dw_esi = log_point * 2 - 0x8E;
    let dw_eax = log_point * 2 - 0x96;

    // dblXmm6
    let mut dbl_xmm6: f64;
    let tmp_eax = if dw_ecx < 0 { -dw_ecx } else { dw_ecx };
    dbl_xmm6 = 2f64.powi(tmp_eax);
    if dw_ecx < 0 { dbl_xmm6 = 1.0 / dbl_xmm6; }

    // dblXmm4
    let dbl_xmm4: f64 = if hleax > 0x80 {
        let dwtmpeax = dw_edx + 1;
        let tmpdbl_xmm3 = 2f64.powi(dwtmpeax);
        let mut dbl_xmm0 = 2f64.powi(dw_edx) * 128.0;
        dbl_xmm0 += (hleax & 0x7F) as f64 * tmpdbl_xmm3;
        dbl_xmm0
    } else {
        if dw_edx >= 0 {
            2f64.powi(dw_edx) * (hleax as f64)
        } else {
            (1.0 / 2f64.powi(-dw_edx)) * (hleax as f64)
        }
    };

    // dblXmm3 and dblXmm1
    let mut dbl_xmm3 = 2f64.powi(dw_esi) * (lheax as f64);
    let mut dbl_xmm1 = 2f64.powi(dw_eax) * (lleax as f64);

    if (hleax & 0x80) != 0 {
        dbl_xmm3 *= 2.0;
        dbl_xmm1 *= 2.0;
    }

    dbl_xmm6 + dbl_xmm4 + dbl_xmm3 + dbl_xmm1
}

/// 解压 zlib 压缩的数据
pub fn unzip(body: Vec<u8>, unzipped_size: usize) -> std::io::Result<Vec<u8>> {
    let mut d = ZlibDecoder::new(&body[..]);
    let mut out = Vec::with_capacity(unzipped_size);
    d.read_to_end(&mut out)?;
    Ok(out)
}

/// Send a request buffer on a blocking `mio::net::TcpStream`, read the
/// level1 response header and body, and return the (possibly decompressed)
/// body bytes. This mirrors the C++ `level1::process()` unzip semantics.
pub fn process_request(stream: &mut MioTcpStream, req_buf: &[u8]) -> std::io::Result<Vec<u8>> {
    // Write the request
    stream.write_all(req_buf)?;

    // Read fixed-size response header (16 bytes)
    let mut hdr = [0u8; 16];
    stream.read_exact(&mut hdr)?;

    // Parse header fields (little-endian)
    let mut bs = crate::std::BinaryStream::from_vec(hdr.to_vec());
    let _i1 = bs.get_u32();
    let _zip_flag = bs.get_u8();
    let _seq_id = bs.get_u32();
    let _i2 = bs.get_u8();
    let _method = bs.get_u16();
    let zip_size = bs.get_u16() as usize;
    let unzip_size = bs.get_u16() as usize;

    if zip_size == 0 {
        return Ok(Vec::new());
    }

    // Read body of zip_size
    let mut body = vec![0u8; zip_size];
    stream.read_exact(&mut body)?;

    // Decompress if needed
    if zip_size != unzip_size {
        let un = unzip(body, unzip_size)?;
        Ok(un)
    } else {
        Ok(body)
    }
}

pub mod hello1 {
    use super::sequence_id;
    use super::BinaryStream;
    use encoding_rs::GBK;

    #[derive(Debug, Clone)]
    pub struct Hello1Request {
        pub zip_flag: u8,
        pub seq_id: u32,
        pub packet_type: u8,
        pub pkg_len1: u16,
        pub pkg_len2: u16,
        pub method: u16,
        pub padding: Vec<u8>,
    }

    impl Hello1Request {
        pub fn new() -> Self {
            Hello1Request {
                zip_flag: 0x0C, // NotZipped
                seq_id: sequence_id(),
                packet_type: 0x01,
                pkg_len1: 0,
                pkg_len2: 0,
                method: 0x000d, // LOGIN1
                padding: hex::decode("01").unwrap_or_default(),
            }
        }

        pub fn serialize(&mut self) -> Vec<u8> {
            self.pkg_len1 = (2 + self.padding.len()) as u16;
            self.pkg_len2 = (2 + self.padding.len()) as u16;
            let mut stream = BinaryStream::new();
            stream.push_u8(self.zip_flag);
            stream.push_u32(self.seq_id);
            stream.push_u8(self.packet_type);
            stream.push_u16(self.pkg_len1);
            stream.push_u16(self.pkg_len2);
            stream.push_u16(self.method);
            stream.push_byte_array(&self.padding);
            stream.data().clone()
        }

        pub fn to_string(&self) -> String {
            format!("Hello1Request {{ ZipFlag:{}, SeqID:{}, PacketType:{}, PkgLen1:{}, PkgLen2:{}, Method:{:#06x}, padding:{} }}",
                    self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method, hex::encode(&self.padding))
        }
    }

    #[derive(Debug, Clone)]
    pub struct Hello1Response {
        pub info: String,
    }

    impl Hello1Response {
        pub fn new() -> Self { Self { info: String::new() } }

        pub fn deserialize(&mut self, data: &[u8]) {
            let offset = 68usize;
            if data.len() >= offset {
                let info_bytes = &data[offset..];
                // decode GBK -> UTF-8 using encoding_rs
                let (cow, _, _) = GBK.decode(info_bytes);
                self.info = cow.into_owned();
            }
        }

        pub fn to_string(&self) -> String { format!("Info: {}", self.info) }
    }
}

pub use hello1::*;

pub mod hello2 {
    use super::sequence_id;
    use super::BinaryStream;
    use encoding_rs::GBK;

    #[derive(Debug, Clone)]
    pub struct Hello2Request {
        pub zip_flag: u8,
        pub seq_id: u32,
        pub packet_type: u8,
        pub pkg_len1: u16,
        pub pkg_len2: u16,
        pub method: u16,
        pub padding: Vec<u8>,
    }

    impl Hello2Request {
        pub fn new() -> Self {
            Hello2Request {
                zip_flag: 0x0C,
                seq_id: sequence_id(),
                packet_type: 0x01,
                pkg_len1: 0,
                pkg_len2: 0,
                method: 0x0fdb, // LOGIN2
                padding: hex::decode("d5d0c9ccd6a4a8af0000008fc22540130000d500c9ccbdf0d7ea00000002").unwrap_or_default(),
            }
        }

        pub fn serialize(&mut self) -> Vec<u8> {
            self.pkg_len1 = (2 + self.padding.len()) as u16;
            self.pkg_len2 = (2 + self.padding.len()) as u16;
            let mut stream = BinaryStream::new();
            stream.push_u8(self.zip_flag);
            stream.push_u32(self.seq_id);
            stream.push_u8(self.packet_type);
            stream.push_u16(self.pkg_len1);
            stream.push_u16(self.pkg_len2);
            stream.push_u16(self.method);
            stream.push_byte_array(&self.padding);
            stream.data().clone()
        }
    }

    #[derive(Debug, Clone)]
    pub struct Hello2Response { pub info: String }
    impl Hello2Response {
        pub fn new() -> Self { Self { info: String::new() } }
        pub fn deserialize(&mut self, data: &[u8]) {
            let offset = 58usize;
            if data.len() >= offset {
                let (cow, _, _) = GBK.decode(&data[offset..]);
                self.info = cow.into_owned();
            }
        }
    }
}

pub use hello2::*;

pub mod heartbeat {
    use super::sequence_id;
    use super::BinaryStream;

    #[derive(Debug, Clone)]
    pub struct HeartbeatRequest {
        pub zip_flag: u8,
        pub seq_id: u32,
        pub packet_type: u8,
        pub pkg_len1: u16,
        pub pkg_len2: u16,
        pub method: u16,
    }

    impl HeartbeatRequest {
        pub fn new() -> Self {
            HeartbeatRequest { zip_flag: 0x0C, seq_id: sequence_id(), packet_type: 0x02, pkg_len1: 0, pkg_len2: 0, method: 0x0004 }
        }
        pub fn serialize(&mut self) -> Vec<u8> {
            self.pkg_len1 = 2;
            self.pkg_len2 = 2;
            let mut stream = BinaryStream::new();
            stream.push_u8(self.zip_flag);
            stream.push_u32(self.seq_id);
            stream.push_u8(self.packet_type);
            stream.push_u16(self.pkg_len1);
            stream.push_u16(self.pkg_len2);
            stream.push_u16(self.method);
            stream.data().clone()
        }
    }

    #[derive(Debug, Clone)]
    pub struct HeartbeatResponse { pub info: String }
    impl HeartbeatResponse {
        pub fn new() -> Self { Self { info: String::new() } }
        pub fn deserialize(&mut self, data: &[u8]) {
            let mut bs = BinaryStream::from_vec(data.to_vec());
            self.info = bs.get_string(10);
        }
    }
}

pub use heartbeat::*;

pub mod xdxr {
    use super::BinaryStream;
    use super::sequence_id;

    #[derive(Debug, Clone)]
    pub struct XdxrInfoRequest {
        pub zip_flag: u8,
        pub seq_id: u32,
        pub packet_type: u8,
        pub pkg_len1: u16,
        pub pkg_len2: u16,
        pub method: u16,
        pub market: u8,
        pub code: [u8;6],
        pub padding: Vec<u8>,
    }

    impl XdxrInfoRequest {
        pub fn new(market: u8, code_str: &str) -> Self {
            let mut code = [0u8;6];
            let bytes = code_str.as_bytes();
            for i in 0..bytes.len().min(6) { code[i] = bytes[i]; }
            XdxrInfoRequest {
                zip_flag: 0x0C,
                seq_id: sequence_id(),
                packet_type: 0x01,
                pkg_len1: 0,
                pkg_len2: 0,
                method: 0x000f,
                market,
                code,
                padding: hex::decode("0100").unwrap_or_default(),
            }
        }

        pub fn serialize(&mut self) -> Vec<u8> {
            self.pkg_len1 = (2 + 1 + 6 + 2) as u16;
            self.pkg_len2 = self.pkg_len1;
            let mut buf = BinaryStream::new();
            buf.push_byte_array(&self.padding);
            buf.push_u8(self.market);
            buf.push_byte_array(&self.code);
            buf.data().clone()
        }
    }

    #[derive(Debug, Clone)]
    pub struct XdxrInfo {
        pub date: String,
        pub category: u8,
        pub name: String,
        pub fenhong: f32,
        pub peigu_jia: f32,
        pub songzhuan: f32,
        pub peigu: f32,
        pub suogu: f32,
        pub qian_liutong: f64,
        pub hou_liutong: f64,
        pub qian_zonggu: f64,
        pub hou_zonggu: f64,
        pub fenshu: f32,
        pub xingquan_jia: f32,
    }

    #[derive(Debug, Clone)]
    pub struct XdxrInfoResponse { pub count: u16, pub list: Vec<XdxrInfo> }
    impl XdxrInfoResponse {
        pub fn new() -> Self { Self { count: 0, list: Vec::new() } }
        pub fn deserialize(&mut self, body: &[u8]) {
            let mut bs = BinaryStream::from_vec(body.to_vec());
            bs.skip(9);
            self.count = bs.get_u16();
            eprintln!("xdxr: body.len={} pos={} count={}", body.len(), bs.position(), self.count);
            // each entry uses 1+6+1+4+1+16 = 29 bytes
            let remaining = if body.len() > bs.position() { body.len() - bs.position() } else { 0 };
            let entry_size = 29usize;
            let max_entries = remaining / entry_size;
            let to_read = std::cmp::min(self.count as usize, max_entries);
            for _ in 0..to_read {
                let _market = bs.get_u8();
                let code = bs.get_string(6);
                let _unk = bs.get_u8();
                let date = bs.get_u32();
                let category = bs.get_u8();
                let mut data = [0u8;16];
                bs.get_byte_array(&mut data);

                let (y, m, d, _hh, _mm) = super::get_datetime_from_u32(category as i32, date, 0);
                let mut info = XdxrInfo { date: format!("{:04}-{:02}-{:02}", y, m, d), category, name: code.clone(), fenhong:0.0, peigu_jia:0.0, songzhuan:0.0, peigu:0.0, suogu:0.0, qian_liutong:0.0, hou_liutong:0.0, qian_zonggu:0.0, hou_zonggu:0.0, fenshu:0.0, xingquan_jia:0.0 };

                let mut tmp = BinaryStream::from_vec(data.to_vec());
                match category as i32 {
                    1 => {
                        info.fenhong = tmp.get_f32();
                        info.peigu_jia = tmp.get_f32();
                        info.songzhuan = tmp.get_f32();
                        info.peigu = tmp.get_f32();
                    }
                    11 | 12 => {
                        tmp.skip(8);
                        info.suogu = tmp.get_f32();
                    }
                    13 | 14 => {
                        info.xingquan_jia = tmp.get_f32();
                        tmp.skip(8);
                        info.fenshu = tmp.get_f32();
                    }
                    _ => {
                        let v1 = tmp.get_u32();
                        info.qian_liutong = super::int_to_float64(v1);
                        let v2 = tmp.get_u32();
                        info.qian_zonggu = super::int_to_float64(v2);
                        let v3 = tmp.get_u32();
                        info.hou_liutong = super::int_to_float64(v3);
                        let v4 = tmp.get_u32();
                        info.hou_zonggu = super::int_to_float64(v4);
                    }
                }

                self.list.push(info);
            }
        }
    }
}

pub use xdxr::*;

#[cfg(test)]
mod tests {
    use super::hello1::*;
    use hex;
    use super::xdxr::*;

    #[test]
    fn test_hello1_deserialize_sample() {
        // sample from tests/tdd-level1.cpp
        let hex2 = "00e9070204280900073a02b2020c03840384038403840384033a02b2020c03840384038403840384030022ff3401194a010022ff3401154a0100ff00f70000010101ff00b1b1bea9c1aacda8d0d0c7e9b6fe000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010000010023b8dbb0c400000000000000000000000000000000000000000000000000";
        let buf = hex::decode(hex2).unwrap();
        let mut resp = Hello1Response::new();
        resp.deserialize(&buf);
        assert!(!resp.info.is_empty());
    }

    #[test]
    fn test_xdxr_deserialize_sample() {
        // sample from tests/tdd-level1.cpp xdxr-response
        let hex1 = "01000136303031313522000136303031313500d9e030010300f0d246e0a4ed480060ea46e0a4ed480136303031313500e456310101cdcc4c3e0000000000000000000000000136303031313500e77d310101cdcc4c3e000000000000000000000000013630303131350029f3310101cdcc4c3e0000000000000000000000000136303031313500e03e3201010000000000000000cdcc4c40000000000136303031313500e03e3201050060ea46e0a4ed4800b01a47e0a4ed480136303031313500028f32010500b01a47e0a4ed4800b01a4708ea194901363030313135004e8f32010500b01a4708ea194900b01a47a0013d4901363030313135004a9132010500b01a47a0013d4900b01a4720f848490136303031313500569132010500b01a4720f8484900b01a47a0ed694901363030313135006ab432010500b01a47a0ed6949c0f82f482fa7894901363030313135006bb8320105c0f82f482fa7894960fa81482fa789490136303031313500310433010560fa81482fa789497c1590482fa7894901363030313135007e043301057c1590482fa78949ac44d6482fa7894901363030313135009504330103ac44d6482fa78949560832492fa7894901363030313135008806330105560832492fa78949d6fe3d492fa789490136303031313500712a330105d6fe3d49232f924901363030313135003d2b330105d6fe3d49232f9249d6fe3d4917b79a4901363030313135007d7a330105d6fe3d4917b79a49d6fe3d490f67a0490136303031313500a29f330105d6fe3d490f67a049be0e4f490f67a049013630303131350076a0330105be0e4f490f67a049be0e4f49359bb0490136303031313500b7a23301015c8f023f0000000000000000000000000136303031313500cfc7330105be0e4f49359bb04909776f49359bb04901363030313135003ac833010148e1fa3e0000000000000000000000000136303031313500f4ee3301015c8f023f00000000000000000000000001363030313135006d1634010509776f49359bb04909776f49f3ecb64901363030313135006e1634010509776f49f3ecb64909776f49f7f1c7490136303031313500663d3401010000003f0000000000000000000000000136303031313500a56534010509776f49f7f1c74909776f49a066e64901363030313135009e8b34010509776f49a066e64989c08849a066e6490136303031313500e0af34010589c08849a066e64989c08849270e084a013630303131350038b234010589c08849270e084afd8ea449270e084a013630303131350048d9340105fd8ea449270e084a3676b249270e084a0136303031313500d7da3401053676b249270e084adfead049270e084a";
        let buf = hex::decode(hex1).unwrap();
        let mut resp = XdxrInfoResponse::new();
        resp.deserialize(&buf);
        assert!(resp.count > 0 || resp.list.len() > 0);
    }
}
