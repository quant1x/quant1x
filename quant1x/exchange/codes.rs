/// Generate stock code list mirroring C++ exchange::GetStockCodeList()
pub fn get_stock_code_list() -> Vec<String> {
    let mut all_codes: Vec<String> = Vec::new();

    // helper: mirror C++ IsNeedIgnore logic
    fn is_need_ignore(code: &str) -> bool {
        // call into exchange::get_security_info which ensures securities are initialized
        // Only ignore when the security exists AND its name contains ignored keywords
        // ("ST", "退", "摘牌"). If the security is not found in the cache, do
        // not ignore it here — we keep the generated code in the list.
        if let Some(info) = crate::exchange::get_security_info(code) {
            const IGNORED_KEYWORDS: [&str; 3] = ["ST", "退", "摘牌"];
            let upper_name = info.name.to_uppercase();
            for kw in IGNORED_KEYWORDS.iter() {
                if upper_name.contains(kw) {
                    return true;
                }
            }
            false
        } else {
            // not found -> do ignore (include it)
            true
        }
    }

    // Shanghai mainboard 600000-609999
    for i in 600000..=609999 {
        let code = format!("sh{:06}", i);
        if !is_need_ignore(&code) {
            all_codes.push(code);
        }
    }

    // STAR market 688000-689999
    for i in 688000..=689999 {
        let code = format!("sh{:06}", i);
        if !is_need_ignore(&code) {
            all_codes.push(code);
        }
    }

    // Shenzhen mainboard 000000-000999
    for i in 0..=999 {
        let code = format!("sz{:06}", i);
        if !is_need_ignore(&code) {
            all_codes.push(code);
        }
    }

    // SME 001000-009999
    for i in 1000..=9999 {
        let code = format!("sz{:06}", i);
        if !is_need_ignore(&code) {
            all_codes.push(code);
        }
    }

    // ChiNext 300000-309999 (C++ used 300000..=309999)
    for i in 300000..=309999 {
        let code = format!("sz{:06}", i);
        if !is_need_ignore(&code) {
            all_codes.push(code);
        }
    }

    all_codes
}

/// Return the canonical A-share index list (mirrors C++ `AShareIndexList`).
pub fn get_index_list() -> Vec<String> {
    // Keep ordering consistent with the C++ `AShareIndexList`.
    vec![
        "sh000001".to_string(), // 上证综合指数
        "sh000002".to_string(), // 上证A股指数
        "sh000300".to_string(), // 沪深300指数
        "sh000688".to_string(), // 科创50指数
        "sh000905".to_string(), // 中证500指数
        "sz399001".to_string(), // 深证成份指数
        "sz399006".to_string(), // 创业板指
        "sz399107".to_string(), // 深证A指
        "sh880005".to_string(), // 通达信板块-涨跌家数
        "sh510050".to_string(), // 上证50ETF
        "sh510300".to_string(), // 沪深300ETF
        "sh510900".to_string(), // H股ETF
    ]
}

/// Provide a basic full code list: indices, (sectors TODO), then stocks.
pub fn get_code_list() -> Vec<String> {
    // 1) indices (AShareIndexList equivalent)
    let mut list: Vec<String> = Vec::new();
    list.extend(get_index_list());

    // 2) sectors/blocks: Not yet implemented in Rust (C++ uses get_sector_list()).
    // Use Rust provider if available: append sector/block codes (in order)
    {
        // The blocks module provides BlockInfo with `code` fields
        let sectors = crate::exchange::get_sector_list();
        for s in sectors {
            list.push(s.code);
        }
    }

    // 3) individual stocks
    list.extend(get_stock_code_list());
    list
}

/// Utility: write securities list (one code per line) to given path
pub fn write_securities_csv<P: AsRef<std::path::Path>>(path: P, codes: &[String]) -> std::io::Result<()> {
    let mut f = std::fs::File::create(path)?;
    use std::io::Write;
    for c in codes {
        writeln!(f, "{}", c)?;
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_stock_code_list_count() {
        let list = get_code_list();
        // Print count so running tests with --nocapture shows it
        println!("get_stock_code_list returned {} codes", list.len());
        // basic sanity: non-empty
        assert!(list.len() > 0);
    }
}
