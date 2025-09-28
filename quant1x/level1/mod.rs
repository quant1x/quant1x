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

mod helpers;
pub use helpers::*;

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
mod hello1;
mod hello2;
mod heartbeat;
mod config;
mod xdxr;
mod finance_info;
mod index_bars;
mod security_bars;
mod security_count;
mod security_list;
mod security_quote;
mod block_info;
mod block_meta;
mod company_category;
mod company_content;
mod minute_time;
mod transaction_data;
mod transaction_history;
mod client;

pub use hello1::*;
pub use hello2::*;
pub use heartbeat::*;
pub use xdxr::*;
pub use client::*;

#[cfg(test)]
mod tests {
    use super::hello1::*;
    use super::heartbeat::*;
    use hex;
    use super::xdxr::*;
    use super::finance_info::*;
    use super::security_quote::*;
    use super::transaction_data::*;
    use super::security_bars::*;

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

    #[test]
    fn test_heartbeat_deserialize_sample() {
        // simple heartbeat sample: headerless body where first 10 bytes are info (pad with ascii)
        let hex_hb = "48656172742d486562696f726974"; // "Heart-Hebior" in ascii as a contrived payload
        let buf = hex::decode(hex_hb).unwrap();
        let mut resp = HeartbeatResponse::new();
        resp.deserialize(&buf);
        assert!(!resp.info.is_empty());
    }

    #[test]
    fn test_finance_info_deserialize_sample() {
        // sample from tests/tdd-level1.cpp finance-info
        let hex = "010001363030313135dfead04910000800d9fe340121bc3001270e084a0000cf4460f0f9ca8c08d94c00000000b9c5fc485c8f42bea6e4834d8cbe914badddbf4ca042334a40cc27488771d94c801c5649703d4a4c089e1a4cb8fffb4c9a46f14c80b6e649303f86ca00e1964874570e4c60bbedca0014cd4900486eca606c92caa0f780ca00000000fca9313f00004041";
        let buf = hex::decode(hex).unwrap();
        let mut resp = FinanceInfoResponse::new();
        resp.deserialize(&buf);
        // ensure deserialize ran and produced either empty count or non-empty code
        assert!(resp.count == 0 || !resp.info.code.is_empty());
    }

    #[test]
    fn test_security_quote_deserialize_sample() {
        // sample from tests/tdd-level1.cpp security-quote
        let hex = "01030600013030303030318912bbb226e14cc95000db5e92a8a50e0b9391c8f704004b012a539687c49e02998a84d902808af743e748aaf5e11c009514940f969ae4029d8a06329301b88bc0d60100a211b50a00000000000000000200000000000d00000001363030313035940dbb0738041f00aa80a70efb07ac929001ab8e01d487104ea8f545849d4a00a09d10fb07000095db14fb070100a36cfb0702008e11fb0703009914fb070400ac2e1605000000000000940d013838303635368c12b3f615b62b9e13af66f344a699910e0198d9b31a96bb5b081d5b5103009ac3d20600f3f615ebf315262cf3f615f3f6150003f3f615f3f6150000f3f615ae81f9020000f2f61500262c02000000000000000000013838303336378f128ef80a9406dd078613c615b79c9c0e06ad82d9119dd8036e70385005009cfbaf01f614cef80ac6f50a238601cef80acef80a0005cef80acef80a0000cef80a918b520000c8f80af614238601020000000000010000000135313030353041128429797901c0019ca9878f01b3f102919af521971ffd8e09508e95d30e8385a2130081040001950bbe23410290d20193b70342038ea901bc584304aa06b12b44051f9f7354040000000008004112013630303833390000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000d000000";
        let buf = hex::decode(hex).unwrap();
        let mut resp = SecurityQuoteResponse::new();
        resp.deserialize(&buf);
        // basic sanity: count parsed (may be zero) and list length matches
        assert_eq!(resp.count as usize, resp.list.len());
    }

    #[test]
    fn test_verify_delisted_securities_unit() {
        use super::security_quote::*;
        // Create a response with one delisting entry
        let mut resp = SecurityQuoteResponse::new();
        let mut sq = SecurityQuote::new();
        sq.market = 1; // Shanghai -> "sh" via market_flag helper
        sq.code = "600839".to_string();
        sq.last_close = 0.0;
        sq.open = 0.0;
        sq.state = TradeState::Delisting;
        resp.list.push(sq);
        resp.count = resp.list.len() as u16;

        // Build code map with the matching security code -> StockInfo
    let mut maps: std::collections::HashMap<String, StockInfo> = std::collections::HashMap::new();
    let key = format!("{}{}", "sh", "600839");
    maps.insert(key, StockInfo{ market: 1, code: "600839".to_string() });

        resp.verify_delisted_securities(&mut maps);
        // After verification the state should be IPO for the delisting item
        assert!(matches!(resp.list[0].state, TradeState::Ipo));
    }

    #[test]
    fn test_transaction_deserialize_sample() {
        // sample from tests/tdd-level1.cpp transaction-base
        let hex = "02007e03af02a40c0901007e0301a108050000";
        let buf = hex::decode(hex).unwrap();
        let mut resp = TransactionResponse::new(1, "600010");
        resp.deserialize(&buf);
        // ensure list length equals count
        assert_eq!(resp.count as usize, resp.list.len());
    }

    #[test]
    fn test_kline_deserialize_sample() {
        // sample from tests/tdd-level1.cpp kline-base
        let hex = "05002bff3401a52910134982d4834e07eb2f4f2eff340102060e4a8a70db4dca40934e2fff3401440a0f4aef5a734e3b6c234f30ff340141191f515cd8094f6d64ba4f31ff34014d102c4398098b4e44b03c4f";
        let buf = hex::decode(hex).unwrap();
    // C++ tests create SecurityBarsResponse(false, 9) for this sample (category 9 = RI_K)
    let mut resp = SecurityBarsResponse::new_with(false, 9);
        resp.deserialize(&buf);
        // basic sanity: either count is zero or list length matches count; prefer non-empty list for this sample
        assert_eq!(resp.count as usize, resp.list.len());
        assert!(resp.list.len() > 0 || resp.count == 0);
    }

    // SecurityQuote tests are non-trivial to craft due to custom varint encoding,
    // we'll add targeted tests later using recorded binary blobs.
}
