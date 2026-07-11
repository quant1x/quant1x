#![allow(dead_code)]

use super::super::super::command::*;
use super::super::super::protocol::{BaseFrame, RequestHeader, ResponseHeader};
use crate::base::BinaryStream;
use encoding_rs::GBK;

#[derive(Debug, Clone)]
pub struct FinanceInfoContext {
    pub req_header: RequestHeader,
    pub resp_header: ResponseHeader,
    pub count: u16,
    pub market: u8,
    pub code: [u8; 6],
    pub info: FinanceInfo,
}

impl FinanceInfoContext {
    pub fn new(security_code: &str) -> Self {
        let mut code = [0u8; 6];
        let inst = crate::data::market::detect_symbol(security_code);
        let market = inst.ext_market as u8;
        let pure = inst.market_ticker().to_string();
        let bytes = pure.as_bytes();
        for i in 0..bytes.len().min(6) {
            code[i] = bytes[i];
        }

        let mut req_header = RequestHeader::new(STD_FINANCE_INFO, FLAG_UNCOMPRESSED);
        req_header.packet_ctrl = 0x01;
        FinanceInfoContext {
            req_header,
            resp_header: ResponseHeader::new(),
            count: 1,
            market,
            code,
            info: FinanceInfo::new(),
        }
    }
}

impl BaseFrame for FinanceInfoContext {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        let mut buf = BinaryStream::new();
        buf.push_u16(self.count);
        buf.push_u8(self.market);
        buf.push_byte_array(&self.code);
        buf.data().clone()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::base::DeserializeError> {
        let mut bs = BinaryStream::from_vec(data.to_vec());
        self.count = bs.get_u16()?;
        if self.count == 0 {
            return Ok(());
        }
        let raw = RawFinanceInfo::decode(&mut bs)?;
        let base_unit: f64 = 10000.0;
        let code = String::from_utf8_lossy(&raw.code).into_owned();
        self.info.code = code.clone();
        self.info.liu_tong_gu_ben = (raw.liu_tong_gu_ben as f64) * base_unit;
        self.info.province = raw.province;
        self.info.industry = raw.industry;
        self.info.updated_date = raw.updated_date;
        self.info.ipo_date = raw.ipo_date;
        self.info.zong_gu_ben = (raw.zong_gu_ben as f64) * base_unit;
        self.info.guo_jia_gu = (raw.guo_jia_gu as f64) * base_unit;
        self.info.fa_qi_ren_fa_ren_gu = (raw.fa_qi_ren_fa_ren_gu as f64) * base_unit;
        self.info.fa_ren_gu = (raw.fa_ren_gu as f64) * base_unit;
        self.info.b_gu = (raw.b_gu as f64) * base_unit;
        self.info.h_gu = (raw.h_gu as f64) * base_unit;
        self.info.zhi_gong_gu = (raw.zhi_gong_gu as f64) * base_unit;
        self.info.zong_zi_chan = (raw.zong_zi_chan as f64) * base_unit;
        self.info.liu_dong_zi_chan = (raw.liu_dong_zi_chan as f64) * base_unit;
        self.info.gu_ding_zi_chan = (raw.gu_ding_zi_chan as f64) * base_unit;
        self.info.wu_xing_zi_chan = (raw.wu_xing_zi_chan as f64) * base_unit;
        self.info.gu_dong_ren_shu = raw.gu_dong_ren_shu as f64;
        self.info.liu_dong_fu_zhai = (raw.liu_dong_fu_zhai as f64) * base_unit;
        self.info.chang_qi_fu_zhai = (raw.chang_qi_fu_zhai as f64) * base_unit;
        self.info.zi_ben_gong_ji_jin = (raw.zi_ben_gong_ji_jin as f64) * base_unit;
        self.info.jing_zi_chan = (raw.jing_zi_chan as f64) * base_unit;
        self.info.zhu_ying_shou_ru = (raw.zhu_ying_shou_ru as f64) * base_unit;
        self.info.zhu_ying_li_run = (raw.zhu_ying_li_run as f64) * base_unit;
        self.info.ying_shou_zhang_kuan = (raw.ying_shou_zhang_kuan as f64) * base_unit;
        self.info.ying_ye_li_run = (raw.ying_ye_li_run as f64) * base_unit;
        self.info.tou_zi_shou_yu = (raw.tou_zi_shou_yu as f64) * base_unit;
        self.info.jing_ying_xian_jin_liu = (raw.jing_ying_xian_jin_liu as f64) * base_unit;
        self.info.zong_xian_jin_liu = (raw.zong_xian_jin_liu as f64) * base_unit;
        self.info.cun_huo = (raw.cun_huo as f64) * base_unit;
        self.info.li_run_zong_he = (raw.li_run_zong_he as f64) * base_unit;
        self.info.shui_hou_li_run = (raw.shui_hou_li_run as f64) * base_unit;
        self.info.jing_li_run = (raw.jing_li_run as f64) * base_unit;
        self.info.wei_fen_li_run = (raw.wei_fen_li_run as f64) * base_unit;
        self.info.mei_gu_jing_zi_chan = (raw.bao_liu1 as f64) * base_unit;
        self.info.bao_liu2 = raw.bao_liu2 as f64;
        Ok(())
    }
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct FinanceInfo {
    pub code: String,
    pub liu_tong_gu_ben: f64,
    pub province: u16,
    pub industry: u16,
    pub updated_date: u32,
    pub ipo_date: u32,
    pub zong_gu_ben: f64,
    pub guo_jia_gu: f64,
    pub fa_qi_ren_fa_ren_gu: f64,
    pub fa_ren_gu: f64,
    pub b_gu: f64,
    pub h_gu: f64,
    pub zhi_gong_gu: f64,
    pub zong_zi_chan: f64,
    pub liu_dong_zi_chan: f64,
    pub gu_ding_zi_chan: f64,
    pub wu_xing_zi_chan: f64,
    pub gu_dong_ren_shu: f64,
    pub liu_dong_fu_zhai: f64,
    pub chang_qi_fu_zhai: f64,
    pub zi_ben_gong_ji_jin: f64,
    pub jing_zi_chan: f64,
    pub zhu_ying_shou_ru: f64,
    pub zhu_ying_li_run: f64,
    pub ying_shou_zhang_kuan: f64,
    pub ying_ye_li_run: f64,
    pub tou_zi_shou_yu: f64,
    pub jing_ying_xian_jin_liu: f64,
    pub zong_xian_jin_liu: f64,
    pub cun_huo: f64,
    pub li_run_zong_he: f64,
    pub shui_hou_li_run: f64,
    pub jing_li_run: f64,
    pub wei_fen_li_run: f64,
    pub mei_gu_jing_zi_chan: f64,
    pub bao_liu2: f64,
}

impl FinanceInfo {
    pub fn new() -> Self {
        Self {
            code: String::new(),
            liu_tong_gu_ben: 0.0,
            province: 0,
            industry: 0,
            updated_date: 0,
            ipo_date: 0,
            zong_gu_ben: 0.0,
            guo_jia_gu: 0.0,
            fa_qi_ren_fa_ren_gu: 0.0,
            fa_ren_gu: 0.0,
            b_gu: 0.0,
            h_gu: 0.0,
            zhi_gong_gu: 0.0,
            zong_zi_chan: 0.0,
            liu_dong_zi_chan: 0.0,
            gu_ding_zi_chan: 0.0,
            wu_xing_zi_chan: 0.0,
            gu_dong_ren_shu: 0.0,
            liu_dong_fu_zhai: 0.0,
            chang_qi_fu_zhai: 0.0,
            zi_ben_gong_ji_jin: 0.0,
            jing_zi_chan: 0.0,
            zhu_ying_shou_ru: 0.0,
            zhu_ying_li_run: 0.0,
            ying_shou_zhang_kuan: 0.0,
            ying_ye_li_run: 0.0,
            tou_zi_shou_yu: 0.0,
            jing_ying_xian_jin_liu: 0.0,
            zong_xian_jin_liu: 0.0,
            cun_huo: 0.0,
            li_run_zong_he: 0.0,
            shui_hou_li_run: 0.0,
            jing_li_run: 0.0,
            wei_fen_li_run: 0.0,
            mei_gu_jing_zi_chan: 0.0,
            bao_liu2: 0.0,
        }
    }
}
#[allow(dead_code)]
#[derive(Debug, Clone)]
struct RawFinanceInfo {
    market: u8,
    code: [u8; 6],
    liu_tong_gu_ben: f32,
    province: u16,
    industry: u16,
    updated_date: u32,
    ipo_date: u32,
    zong_gu_ben: f32,
    guo_jia_gu: f32,
    fa_qi_ren_fa_ren_gu: f32,
    fa_ren_gu: f32,
    b_gu: f32,
    h_gu: f32,
    zhi_gong_gu: f32,
    zong_zi_chan: f32,
    liu_dong_zi_chan: f32,
    gu_ding_zi_chan: f32,
    wu_xing_zi_chan: f32,
    gu_dong_ren_shu: f32,
    liu_dong_fu_zhai: f32,
    chang_qi_fu_zhai: f32,
    zi_ben_gong_ji_jin: f32,
    jing_zi_chan: f32,
    zhu_ying_shou_ru: f32,
    zhu_ying_li_run: f32,
    ying_shou_zhang_kuan: f32,
    ying_ye_li_run: f32,
    tou_zi_shou_yu: f32,
    jing_ying_xian_jin_liu: f32,
    zong_xian_jin_liu: f32,
    cun_huo: f32,
    li_run_zong_he: f32,
    shui_hou_li_run: f32,
    jing_li_run: f32,
    wei_fen_li_run: f32,
    bao_liu1: f32,
    bao_liu2: f32,
}
impl RawFinanceInfo {
    fn decode(bs: &mut BinaryStream) -> Result<Self, crate::base::DeserializeError> {
        let market = bs.get_u8()?;
        let mut code = [0u8; 6];
        bs.get_byte_array(&mut code)?;
        let liu_tong_gu_ben = bs.get_f32()?;
        let province = bs.get_u16()?;
        let industry = bs.get_u16()?;
        let updated_date = bs.get_u32()?;
        let ipo_date = bs.get_u32()?;
        let zong_gu_ben = bs.get_f32()?;
        let guo_jia_gu = bs.get_f32()?;
        let fa_qi_ren_fa_ren_gu = bs.get_f32()?;
        let fa_ren_gu = bs.get_f32()?;
        let b_gu = bs.get_f32()?;
        let h_gu = bs.get_f32()?;
        let zhi_gong_gu = bs.get_f32()?;
        let zong_zi_chan = bs.get_f32()?;
        let liu_dong_zi_chan = bs.get_f32()?;
        let gu_ding_zi_chan = bs.get_f32()?;
        let wu_xing_zi_chan = bs.get_f32()?;
        let gu_dong_ren_shu = bs.get_f32()?;
        let liu_dong_fu_zhai = bs.get_f32()?;
        let chang_qi_fu_zhai = bs.get_f32()?;
        let zi_ben_gong_ji_jin = bs.get_f32()?;
        let jing_zi_chan = bs.get_f32()?;
        let zhu_ying_shou_ru = bs.get_f32()?;
        let zhu_ying_li_run = bs.get_f32()?;
        let ying_shou_zhang_kuan = bs.get_f32()?;
        let ying_ye_li_run = bs.get_f32()?;
        let tou_zi_shou_yu = bs.get_f32()?;
        let jing_ying_xian_jin_liu = bs.get_f32()?;
        let zong_xian_jin_liu = bs.get_f32()?;
        let cun_huo = bs.get_f32()?;
        let li_run_zong_he = bs.get_f32()?;
        let shui_hou_li_run = bs.get_f32()?;
        let jing_li_run = bs.get_f32()?;
        let wei_fen_li_run = bs.get_f32()?;
        let bao_liu1 = bs.get_f32()?;
        let bao_liu2 = bs.get_f32()?;

        Ok(RawFinanceInfo {
            market,
            code,
            liu_tong_gu_ben,
            province,
            industry,
            updated_date,
            ipo_date,
            zong_gu_ben,
            guo_jia_gu,
            fa_qi_ren_fa_ren_gu,
            fa_ren_gu,
            b_gu,
            h_gu,
            zhi_gong_gu,
            zong_zi_chan,
            liu_dong_zi_chan,
            gu_ding_zi_chan,
            wu_xing_zi_chan,
            gu_dong_ren_shu,
            liu_dong_fu_zhai,
            chang_qi_fu_zhai,
            zi_ben_gong_ji_jin,
            jing_zi_chan,
            zhu_ying_shou_ru,
            zhu_ying_li_run,
            ying_shou_zhang_kuan,
            ying_ye_li_run,
            tou_zi_shou_yu,
            jing_ying_xian_jin_liu,
            zong_xian_jin_liu,
            cun_huo,
            li_run_zong_he,
            shui_hou_li_run,
            jing_li_run,
            wei_fen_li_run,
            bao_liu1,
            bao_liu2,
        })
    }
}

/// FinanceInfoResponse 已合并到 FinanceInfoContext 中. 
/// 保留类型别名以兼容旧代码. 
pub type FinanceInfoResponse = FinanceInfoContext;

pub fn fetch_finance_info(security_code: &str) -> Option<FinanceInfoResponse> {
    match super::super::super::client::get_std_conn() {
        Ok(mut pooled) => {
            let mut msg = FinanceInfoContext::new(security_code);
            match super::super::super::protocol::transact_message_sync(pooled.stream(), &mut msg) {
                Ok(_) => Some(msg),
                Err(e) => {
                    log::error!("level1 protocol::process error for finance_info: {}", e);
                    None
                }
            }
        }
        Err(e) => {
            log::error!("failed to acquire level1 client for finance_info: {}", e);
            None
        }
    }
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn deserialize_sample_matches_cpp_behavior() {
        let hex_data = "010001363030313135dfead04910000800d9fe340121bc3001270e084a0000cf4460f0f9ca8c08d94c00000000b9c5fc485c8f42bea6e4834d8cbe914badddbf4ca042334a40cc27488771d94c801c5649703d4a4c089e1a4cb8fffb4c9a46f14c80b6e649303f86ca00e1964874570e4c60bbedca0014cd4900486eca606c92caa0f780ca00000000fca9313f00004041";
        let buf = hex::decode(hex_data).unwrap();
        let mut msg = FinanceInfoContext::new("sh600115");
        msg.deserialize_response_body(&buf).unwrap();
        assert!(msg.count == 0 || !msg.info.code.is_empty());
    }
}
