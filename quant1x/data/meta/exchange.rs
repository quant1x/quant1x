// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

use super::region::Region;

/// 交易所
///
/// 每个变体持有 (mic, identifier, region, label) 四元组, 
/// 与 Python 版 Exchange(Enum) 语义一致. 
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Exchange {
    // 中国市场
    SSE,   // 上海证券交易所
    XSSC,  // XSSC: 上海证券交易所 - 沪股通
    SZSE,  // 深圳证券交易所
    XSEC,  // XSEC: 深证证券交易所 - 深股通
    BSE,   // 北京证券交易所
    // 期货交易所
    SHFE,  // 上海期货交易所
    XINE,  // 上海国际能源交易中心
    CZCE,  // 郑州商品交易所
    DCE,   // 大连商品交易所
    CFFEX, // 中国金融期货交易所
    GFEX,  // 广州期货交易所
    SGE,   // 上海黄金交易所
    // 香港
    HKEX,  // 香港交易所(现货股票)
    HKSC,  // 香港交易所-港股通
    HKFE,  // 香港期货交易所
    // 指数
    CSI,   // 中证指数
    CNI,   // 国证指数
    // 扩展
    EXTENDED, // 扩展市场
    // 离岸/在岸
    OFFSHORE, // 国际, 其它离岸市场
    ONSHORE,  // 国内, 其它在岸市场
    OTC,      // 国内, 场外
    OFFEX,    // 场外申赎市场
    // 宏观
    MACRO, // 宏观经济市场
    // 美国
    USA,    // 美国证券市场(泛指)
    NYSE,   // 纽约证券交易所
    NASDAQ, // 纳斯达克
    // 英国
    LSE, // 伦敦证券交易所
    GBR, // 英国证券市场(泛指)
    // 新加坡
    SGX, // 新加坡交易所
    // 其它
    MIRROR,  // 镜像市场
    TEMP,    // 临时市场
    UNKNOWN, // 未知交易所
}

/// 所有枚举变体构成的静态数组, 用于迭代查找(对应 Python 的 `for ex in cls`). 
const ALL: &[Exchange] = &[
    Exchange::SSE, Exchange::XSSC, Exchange::SZSE, Exchange::XSEC, Exchange::BSE,
    Exchange::SHFE, Exchange::XINE, Exchange::CZCE, Exchange::DCE, Exchange::CFFEX,
    Exchange::GFEX, Exchange::SGE,
    Exchange::HKEX, Exchange::HKSC, Exchange::HKFE,
    Exchange::CSI, Exchange::CNI,
    Exchange::EXTENDED,
    Exchange::OFFSHORE, Exchange::ONSHORE, Exchange::OTC, Exchange::OFFEX,
    Exchange::MACRO,
    Exchange::USA, Exchange::NYSE, Exchange::NASDAQ,
    Exchange::LSE, Exchange::GBR,
    Exchange::SGX,
    Exchange::MIRROR, Exchange::TEMP, Exchange::UNKNOWN,
];

impl Exchange {
    /// MIC: Market Identifier Code
    pub fn mic(self) -> &'static str {
        match self {
            Exchange::SSE => "XSHG",
            Exchange::XSSC => "XSSC",
            Exchange::SZSE => "XSHE",
            Exchange::XSEC => "XSEC",
            Exchange::BSE => "BJSE",
            Exchange::SHFE => "XSGE",
            Exchange::XINE => "XINE",
            Exchange::CZCE => "XZCE",
            Exchange::DCE => "XDCE",
            Exchange::CFFEX => "CCFX",
            Exchange::GFEX => "GFEX",
            Exchange::SGE => "SGEX",
            Exchange::HKEX => "XHKG",
            Exchange::HKSC => "XHKG",
            Exchange::HKFE => "XHKF",
            Exchange::CSI => "CSI",
            Exchange::CNI => "CNI",
            Exchange::EXTENDED => "EXTENDED",
            Exchange::OFFSHORE => "OFFSHORE",
            Exchange::ONSHORE => "ONSHORE",
            Exchange::OTC => "OTC",
            Exchange::OFFEX => "OFFEX",
            Exchange::MACRO => "MACRO",
            Exchange::USA => "USA",
            Exchange::NYSE => "XNYS",
            Exchange::NASDAQ => "XNAS",
            Exchange::LSE => "XLON",
            Exchange::GBR => "GBR",
            Exchange::SGX => "XSES",
            Exchange::MIRROR => "MIRROR",
            Exchange::TEMP => "TEMP",
            Exchange::UNKNOWN => "UNKNOWN",
        }
    }

    /// 标识: 交易所的小写缩写, 如 sh/sz/bj/hk
    pub fn identifier(self) -> &'static str {
        match self {
            Exchange::SSE => "sh",
            Exchange::XSSC => "sh",
            Exchange::SZSE => "sz",
            Exchange::XSEC => "sz",
            Exchange::BSE => "bj",
            Exchange::SHFE => "shfe",
            Exchange::XINE => "ine",
            Exchange::CZCE => "zce",
            Exchange::DCE => "dce",
            Exchange::CFFEX => "cff",
            Exchange::GFEX => "gfex",
            Exchange::SGE => "sge",
            Exchange::HKEX => "hk",
            Exchange::HKSC => "hksc",
            Exchange::HKFE => "hkf",
            Exchange::CSI => "csi",
            Exchange::CNI => "cni",
            Exchange::EXTENDED => "ext",
            Exchange::OFFSHORE => "os",
            Exchange::ONSHORE => "on",
            Exchange::OTC => "otc",
            Exchange::OFFEX => "offex",
            Exchange::MACRO => "macro",
            Exchange::USA => "us",
            Exchange::NYSE => "us",
            Exchange::NASDAQ => "us",
            Exchange::LSE => "uk",
            Exchange::GBR => "uk",
            Exchange::SGX => "sg",
            Exchange::MIRROR => "mirror",
            Exchange::TEMP => "temp",
            Exchange::UNKNOWN => "unknown",
        }
    }

    /// 市场区域
    pub fn region(self) -> Region {
        match self {
            Exchange::SSE | Exchange::XSSC | Exchange::SZSE | Exchange::XSEC |
            Exchange::BSE | Exchange::SHFE | Exchange::XINE | Exchange::CZCE |
            Exchange::DCE | Exchange::CFFEX | Exchange::GFEX | Exchange::SGE |
            Exchange::CSI | Exchange::CNI => Region::CN,
            Exchange::HKEX | Exchange::HKSC | Exchange::HKFE => Region::HK,
            Exchange::EXTENDED | Exchange::MACRO | Exchange::MIRROR |
            Exchange::TEMP => Region::GLB,
            Exchange::OFFSHORE => Region::OFFSHORE,
            Exchange::ONSHORE | Exchange::OTC | Exchange::OFFEX => Region::ONSHORE,
            Exchange::USA | Exchange::NYSE | Exchange::NASDAQ => Region::US,
            Exchange::LSE | Exchange::GBR => Region::UK,
            Exchange::SGX => Region::SG,
            Exchange::UNKNOWN => Region::UNKNOWN,
        }
    }

    /// 交易所名称
    pub fn label(self) -> &'static str {
        match self {
            Exchange::SSE => "上海证券交易所",
            Exchange::XSSC => "上海证券交易所",
            Exchange::SZSE => "深圳证券交易所",
            Exchange::XSEC => "深圳证券交易所",
            Exchange::BSE => "北京证券交易所",
            Exchange::SHFE => "上海期货交易所",
            Exchange::XINE => "上海国际能源交易中心",
            Exchange::CZCE => "郑州商品交易所",
            Exchange::DCE => "大连商品交易所",
            Exchange::CFFEX => "中国金融期货交易所",
            Exchange::GFEX => "广州期货交易所",
            Exchange::SGE => "上海黄金交易所",
            Exchange::HKEX => "香港交易所(现货股票)",
            Exchange::HKSC => "香港交易所-港股通",
            Exchange::HKFE => "香港期货交易所(香港指数市场, 指数期货, 商品期货)",
            Exchange::CSI => "中证指数, China Securities Index, 中证指数有限公司",
            Exchange::CNI => "国证指数, CNI Index, 深证证券交易所指数机构",
            Exchange::EXTENDED => "扩展市场, Extended",
            Exchange::OFFSHORE => "国际, 其它离岸市场",
            Exchange::ONSHORE => "国内, 其它在岸市场",
            Exchange::OTC => "国内, 场外",
            Exchange::OFFEX => "场外申赎市场, Off-exchange Subscription/Redemption",
            Exchange::MACRO => "宏观经济市场, Macro-economic",
            Exchange::USA => "美国证券市场(泛指)",
            Exchange::NYSE => "纽约证券交易所",
            Exchange::NASDAQ => "纳斯达克",
            Exchange::LSE => "伦敦证券交易所",
            Exchange::GBR => "英国证券市场(泛指)",
            Exchange::SGX => "新加坡交易所",
            Exchange::MIRROR => "镜像市场, Mirror",
            Exchange::TEMP => "临时市场, Temporary",
            Exchange::UNKNOWN => "未知交易所",
        }
    }

    /// 枚举名, 对应 Python 的 `self.name`
    pub fn code(self) -> &'static str {
        match self {
            Exchange::SSE => "SSE",
            Exchange::XSSC => "XSSC",
            Exchange::SZSE => "SZSE",
            Exchange::XSEC => "XSEC",
            Exchange::BSE => "BSE",
            Exchange::SHFE => "SHFE",
            Exchange::XINE => "XINE",
            Exchange::CZCE => "CZCE",
            Exchange::DCE => "DCE",
            Exchange::CFFEX => "CFFEX",
            Exchange::GFEX => "GFEX",
            Exchange::SGE => "SGE",
            Exchange::HKEX => "HKEX",
            Exchange::HKSC => "HKSC",
            Exchange::HKFE => "HKFE",
            Exchange::CSI => "CSI",
            Exchange::CNI => "CNI",
            Exchange::EXTENDED => "EXTENDED",
            Exchange::OFFSHORE => "OFFSHORE",
            Exchange::ONSHORE => "ONSHORE",
            Exchange::OTC => "OTC",
            Exchange::OFFEX => "OFFEX",
            Exchange::MACRO => "MACRO",
            Exchange::USA => "USA",
            Exchange::NYSE => "NYSE",
            Exchange::NASDAQ => "NASDAQ",
            Exchange::LSE => "LSE",
            Exchange::GBR => "GBR",
            Exchange::SGX => "SGX",
            Exchange::MIRROR => "MIRROR",
            Exchange::TEMP => "TEMP",
            Exchange::UNKNOWN => "UNKNOWN",
        }
    }

    /// 智能解析字符串为 Exchange 实例
    pub fn parse(s: &str) -> Result<Self, String> {
        if s.is_empty() {
            return Err("Empty string cannot be parsed to Exchange".to_string());
        }

        let name = s.trim().to_uppercase();

        // 1. By code (enum name) — 对应 Python 的 `cls[name_]`
        for ex in ALL {
            if ex.code() == name {
                return Ok(*ex);
            }
        }

        // 2. By identifier
        let identifier = name.to_lowercase();
        for ex in ALL {
            if ex.identifier() == identifier {
                return Ok(*ex);
            }
        }

        // 3. By MIC
        for ex in ALL {
            if ex.mic() == name {
                return Ok(*ex);
            }
        }

        Err(format!("Cannot parse exchange from: '{}'", s))
    }

    /// 格式化输出
    pub fn to_string(self) -> String {
        format!(
            "<Exchange.{}: {} ({}) - {}>",
            self.code(),
            self.identifier(),
            self.region().as_str(),
            self.label(),
        )
    }

    /// 是否国内交易所
    pub fn is_domestic(self) -> bool {
        matches!(self.region(), Region::CN | Region::HK)
    }

    /// 是否标准行情接口支持的交易所
    pub fn is_std_quote(self) -> bool {
        matches!(self, Exchange::SSE | Exchange::SZSE | Exchange::BSE)
    }

    /// 是否扩展行情接口支持的交易所
    pub fn is_ext_quote(self) -> bool {
        !self.is_std_quote() && self != Exchange::UNKNOWN
    }

    /// 根据代码创建 Exchange — 对应 Python 的 `from_code`
    pub fn from_code(code: &str) -> Result<Self, String> {
        let name = code.trim().to_uppercase();
        for ex in ALL {
            if ex.code() == name {
                return Ok(*ex);
            }
        }
        Err(format!("Unknown exchange code: {}", code))
    }

    /// 根据缩写创建 Exchange — 对应 Python 的 `from_abbr`
    pub fn from_abbr(abbr: &str) -> Result<Self, String> {
        let identifier = abbr.trim().to_lowercase();
        for ex in ALL {
            if ex.identifier() == identifier {
                return Ok(*ex);
            }
        }
        Err(format!("Unknown exchange abbreviation: {}", abbr))
    }

    /// 根据 MIC 创建 Exchange — 对应 Python 的 `from_mic`
    pub fn from_mic(mic: &str) -> Result<Self, String> {
        let mic = mic.trim().to_uppercase();
        for ex in ALL {
            if ex.mic() == mic {
                return Ok(*ex);
            }
        }
        Err(format!("Unknown MIC: {}", mic))
    }
}

impl std::fmt::Display for Exchange {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.code())
    }
}

/// 智能解析字符串为 Exchange 实例(函数版本, 对应 Go 的 ParseExchange)
pub fn parse_exchange(s: &str) -> Result<Exchange, String> {
    Exchange::parse(s)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_by_code() {
        assert_eq!(Exchange::parse("SSE").unwrap(), Exchange::SSE);
        assert_eq!(Exchange::parse("SZSE").unwrap(), Exchange::SZSE);
        assert_eq!(Exchange::parse("BSE").unwrap(), Exchange::BSE);
    }

    #[test]
    fn test_parse_by_identifier() {
        assert_eq!(Exchange::parse("sh").unwrap(), Exchange::SSE);
        assert_eq!(Exchange::parse("sz").unwrap(), Exchange::SZSE);
        assert_eq!(Exchange::parse("bj").unwrap(), Exchange::BSE);
        assert_eq!(Exchange::parse("hk").unwrap(), Exchange::HKEX);
    }

    #[test]
    fn test_parse_by_mic() {
        assert_eq!(Exchange::parse("XSHG").unwrap(), Exchange::SSE);
        assert_eq!(Exchange::parse("XSHE").unwrap(), Exchange::SZSE);
        assert_eq!(Exchange::parse("BJSE").unwrap(), Exchange::BSE);
    }

    #[test]
    fn test_parse_empty() {
        assert!(Exchange::parse("").is_err());
    }

    #[test]
    fn test_parse_invalid() {
        assert!(Exchange::parse("INVALID").is_err());
    }

    #[test]
    fn test_fields() {
        let ex = Exchange::SSE;
        assert_eq!(ex.mic(), "XSHG");
        assert_eq!(ex.identifier(), "sh");
        assert_eq!(ex.region(), Region::CN);
        assert_eq!(ex.label(), "上海证券交易所");
        assert_eq!(ex.code(), "SSE");
    }

    #[test]
    fn test_to_string() {
        let s = Exchange::SSE.to_string();
        assert!(s.contains("Exchange.SSE"));
        assert!(s.contains("sh"));
        assert!(s.contains("CN"));
        assert!(s.contains("上海证券交易所"));
    }

    #[test]
    fn test_is_domestic() {
        assert!(Exchange::SSE.is_domestic());
        assert!(Exchange::HKEX.is_domestic());
        assert!(!Exchange::USA.is_domestic());
        assert!(!Exchange::LSE.is_domestic());
    }

    #[test]
    fn test_is_std_quote() {
        assert!(Exchange::SSE.is_std_quote());
        assert!(Exchange::SZSE.is_std_quote());
        assert!(Exchange::BSE.is_std_quote());
        assert!(!Exchange::HKEX.is_std_quote());
        assert!(!Exchange::UNKNOWN.is_std_quote());
    }

    #[test]
    fn test_is_ext_quote() {
        assert!(!Exchange::SSE.is_ext_quote());       // 标准行情
        assert!(Exchange::HKEX.is_ext_quote());       // 扩展行情
        assert!(!Exchange::UNKNOWN.is_ext_quote());   // UNKNOWN 不算扩展
    }

    #[test]
    fn test_from_code() {
        assert_eq!(Exchange::from_code("SSE").unwrap(), Exchange::SSE);
        assert!(Exchange::from_code("INVALID").is_err());
    }

    #[test]
    fn test_from_abbr() {
        assert_eq!(Exchange::from_abbr("sz").unwrap(), Exchange::SZSE);
        assert!(Exchange::from_abbr("INVALID").is_err());
    }

    #[test]
    fn test_from_mic() {
        assert_eq!(Exchange::from_mic("XSHG").unwrap(), Exchange::SSE);
        assert!(Exchange::from_mic("INVALID").is_err());
    }

    #[test]
    fn test_display() {
        assert_eq!(format!("{}", Exchange::SSE), "SSE");
    }

    #[test]
    fn test_all_variants_have_data() {
        for ex in ALL {
            assert!(!ex.mic().is_empty(), "{:?}.mic() is empty", ex);
            assert!(!ex.identifier().is_empty(), "{:?}.identifier() is empty", ex);
            assert!(!ex.label().is_empty(), "{:?}.label() is empty", ex);
            assert!(!ex.code().is_empty(), "{:?}.code() is empty", ex);
        }
    }
}
