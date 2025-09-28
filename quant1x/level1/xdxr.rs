use super::BinaryStream;

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
            seq_id: super::sequence_id(),
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
