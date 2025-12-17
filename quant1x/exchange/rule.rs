use crate::exchange::{
    ExchangeId, SecurityCode, EXCHANGE_BJSE, EXCHANGE_HK, EXCHANGE_SSE, EXCHANGE_SZSE, EXCHANGE_US,
};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum SecurityType {
    TypeUnknown = 0,
    TypeStock = 1,
    TypeETF = 2,
    TypeFund = 3,
    TypeBond = 4,
    TypeBStock = 5,
    TypeIPO = 6,
    TypeIndex = 7,
    TypeBlock = 8,
    TypeOption = 9,
    TypeFuture = 10,
    TypeWarrant = 11,
    TypeForex = 12,
    TypeCommodity = 13,
    TypeOther = 255,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
// Reuse `SecurityType` and `SecurityCodeExt` from `exchange.rs` to match Go layout

// CodeRule equivalent (match Go's struct with Prefix, Type, Desc)
struct CodeRule {
    prefix: &'static str,
    typ: SecurityType,
    desc: &'static str,
    note: &'static str,
}

// Rule tables (ported from code_rule.go)
const GLOBAL_RULES: &[CodeRule] = &[
    CodeRule { prefix: "880", typ: SecurityType::TypeBlock, desc: "板块指数", note: "通达信" },
    CodeRule { prefix: "881", typ: SecurityType::TypeBlock, desc: "板块指数", note: "通达信" },
];

const SSE_RULES: &[CodeRule] = &[
    CodeRule { prefix: "000", typ: SecurityType::TypeIndex, desc: "上证指数", note: "上证指数系列；000680-000689 用于科创板相关指数" },
    CodeRule { prefix: "009", typ: SecurityType::TypeBond, desc: "国债", note: "国债（2000年前发行）" },
    CodeRule { prefix: "010", typ: SecurityType::TypeBond, desc: "国债", note: "国债（2000-2009年发行）" },
    CodeRule { prefix: "018", typ: SecurityType::TypeBond, desc: "政策性银行债", note: "政策性银行金融债" },
    CodeRule { prefix: "019", typ: SecurityType::TypeBond, desc: "国债", note: "国债（2010年及以后发行）" },
    CodeRule { prefix: "020", typ: SecurityType::TypeBond, desc: "记账式贴现国债", note: "记账式贴现国债" },
    CodeRule { prefix: "090", typ: SecurityType::TypeBond, desc: "国债质押回购出入库", note: "国债质押式回购质押券出入库" },
    CodeRule { prefix: "091", typ: SecurityType::TypeBond, desc: "国债质押回购出入库", note: "对应019***" },
    CodeRule { prefix: "099", typ: SecurityType::TypeBond, desc: "国债质押回购出入库", note: "对应009***" },

    CodeRule { prefix: "100", typ: SecurityType::TypeBond, desc: "债券回售/可转债", note: "100000-100899 用于可转换公司债券（对应600***）；100900-100999 用于债券回售（不再增用部分）" },
    CodeRule { prefix: "101", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },
    CodeRule { prefix: "102", typ: SecurityType::TypeBond, desc: "企业债质押出入库", note: "对应127000-127999" },
    CodeRule { prefix: "103", typ: SecurityType::TypeBond, desc: "企业债质押出入库", note: "对应124000-124999" },
    CodeRule { prefix: "104", typ: SecurityType::TypeBond, desc: "公司/企业债质押出入库", note: "104000-104499 用于公司债质押（对应122000-122499）；104500-104999 用于企业债质押（对应122500-122999）" },
    CodeRule { prefix: "105", typ: SecurityType::TypeBond, desc: "债券质押出入库", note: "105000-105699 分离交易的可转债质押（对应126***）；105700-105799 债券ETF质押；105800-105899 可转债质押（对应110***、113***）；105900-105999 企业债质押（对应120***、129***）" },
    CodeRule { prefix: "106", typ: SecurityType::TypeBond, desc: "地方政府债质押出入库", note: "对应130***" },
    CodeRule { prefix: "107", typ: SecurityType::TypeBond, desc: "记账式贴现国债质押出入库", note: "对应020***" },
    CodeRule { prefix: "108", typ: SecurityType::TypeBond, desc: "政策性银行债质押出入库", note: "对应018***" },
    CodeRule { prefix: "109", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },

    CodeRule { prefix: "110", typ: SecurityType::TypeBond, desc: "可转换公司债", note: "110000-110799 上市公司公开发行可转债（对应600***）；110800-110999 非公开发行" },
    CodeRule { prefix: "111", typ: SecurityType::TypeBond, desc: "可转换公司债", note: "111000-111499 对应605***" },
    CodeRule { prefix: "112", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },
    CodeRule { prefix: "113", typ: SecurityType::TypeBond, desc: "可转换公司债", note: "113000-113499 对应601***；113500-113999 对应603***" },
    CodeRule { prefix: "114", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "115", typ: SecurityType::TypeBond, desc: "公开公司债", note: "公开发行公司债券" },
    CodeRule { prefix: "118", typ: SecurityType::TypeBond, desc: "科创板可转债", note: "118000-118499 用于科创板上市公司公开发行可转债" },

    CodeRule { prefix: "120", typ: SecurityType::TypeBond, desc: "企业/公司债", note: "122000-122499 用于公司债券；122500-122999 用于企业债券（见122）" },
    CodeRule { prefix: "121", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },
    CodeRule { prefix: "122", typ: SecurityType::TypeBond, desc: "公司债/企业债", note: "122000-122499 用于公司债券；122500-122999 用于企业债券" },
    CodeRule { prefix: "123", typ: SecurityType::TypeBond, desc: "公司/企业债/ABS", note: "123000-123499 用于企业/公司债；123500-123999 用于资产支持证券" },
    CodeRule { prefix: "124", typ: SecurityType::TypeBond, desc: "企业债质押出入库", note: "对应124000-124999" },
    CodeRule { prefix: "125", typ: SecurityType::TypeBond, desc: "中小企业私募债/非公开公司债", note: "中小企业私募债券、非公开发行公司债券" },
    CodeRule { prefix: "126", typ: SecurityType::TypeBond, desc: "分离交易可转债", note: "分离交易的可转换公司债券" },
    CodeRule { prefix: "127", typ: SecurityType::TypeBond, desc: "企业债", note: "127000-127899 用于企业债券；127900-127999 用于政府支持债（中国铁路建设债专用）" },
    CodeRule { prefix: "128", typ: SecurityType::TypeBond, desc: "信贷资产支持证券", note: "信贷资产支持证券" },
    CodeRule { prefix: "129", typ: SecurityType::TypeBond, desc: "企业债", note: "企业债券" },

    CodeRule { prefix: "130", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券(对应130***)" },
    CodeRule { prefix: "131", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },
    CodeRule { prefix: "132", typ: SecurityType::TypeBond, desc: "可交换公司债", note: "可交换公司债券" },
    CodeRule { prefix: "133", typ: SecurityType::TypeBond, desc: "可交换债质押出入库", note: "对应132***" },
    CodeRule { prefix: "134", typ: SecurityType::TypeBond, desc: "公开公司债质押出入库", note: "对应136***" },
    CodeRule { prefix: "135", typ: SecurityType::TypeBond, desc: "证券公司短期债/并购私募债", note: "证券公司短期债、并购重组私募债券、非公开发行公司债券" },
    CodeRule { prefix: "136", typ: SecurityType::TypeBond, desc: "公开公司债质押出入库", note: "对应136***" },
    CodeRule { prefix: "137", typ: SecurityType::TypeBond, desc: "可交换/公开公司债", note: "137000-137499 非公开可交换；137500-137999 公开公司债" },
    CodeRule { prefix: "138", typ: SecurityType::TypeBond, desc: "可交换换股/公开公司债", note: "138000-138499 非公开可交换换股(对应137000-137499)；138500-138999 公开公司债" },
    CodeRule { prefix: "139", typ: SecurityType::TypeBond, desc: "企业债", note: "企业债券" },

    CodeRule { prefix: "140", typ: SecurityType::TypeBond, desc: "地方政府债质押出入库", note: "对应140***" },
    CodeRule { prefix: "141", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },
    CodeRule { prefix: "142", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },
    CodeRule { prefix: "143", typ: SecurityType::TypeBond, desc: "公开公司债质押出入库", note: "对应143***" },
    CodeRule { prefix: "144", typ: SecurityType::TypeBond, desc: "公开公司债", note: "公开发行公司债券" },
    CodeRule { prefix: "145", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "146", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },
    CodeRule { prefix: "147", typ: SecurityType::TypeBond, desc: "地方政府债质押出入库", note: "对应147***" },
    CodeRule { prefix: "148", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },
    CodeRule { prefix: "149", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },

    CodeRule { prefix: "150", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "151", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "152", typ: SecurityType::TypeBond, desc: "企业债质押出入库", note: "对应152***" },
    CodeRule { prefix: "153", typ: SecurityType::TypeBond, desc: "企业债", note: "企业债券" },
    CodeRule { prefix: "154", typ: SecurityType::TypeBond, desc: "公司债质押出入库", note: "对应155***" },
    CodeRule { prefix: "155", typ: SecurityType::TypeBond, desc: "公司债质押出入库", note: "对应155***" },
    CodeRule { prefix: "156", typ: SecurityType::TypeBond, desc: "公司债", note: "公司债券" },
    CodeRule { prefix: "157", typ: SecurityType::TypeBond, desc: "地方政府债质押出入库", note: "对应157***" },
    CodeRule { prefix: "158", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },
    CodeRule { prefix: "159", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },

    CodeRule { prefix: "160", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },
    CodeRule { prefix: "161", typ: SecurityType::TypeBond, desc: "地方政府债质押出入库", note: "对应160***" },
    CodeRule { prefix: "162", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "163", typ: SecurityType::TypeBond, desc: "公开公司债质押出入库", note: "对应163***" },
    CodeRule { prefix: "164", typ: SecurityType::TypeBond, desc: "公开公司债", note: "公开发行公司债券" },
    CodeRule { prefix: "165", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },
    CodeRule { prefix: "166", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "167", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "168", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },
    CodeRule { prefix: "169", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },

    CodeRule { prefix: "170", typ: SecurityType::TypeBond, desc: "信用保护工具", note: "170000-170499 用于信用保护凭证；170900-170999 用于组合型信用保护合约" },
    CodeRule { prefix: "171", typ: SecurityType::TypeBond, desc: "地方政府债质押出入库", note: "对应171***" },
    CodeRule { prefix: "172", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },
    CodeRule { prefix: "173", typ: SecurityType::TypeBond, desc: "地方政府债质押出入库", note: "对应173***" },
    CodeRule { prefix: "174", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },
    CodeRule { prefix: "175", typ: SecurityType::TypeBond, desc: "公开公司债质押出入库", note: "对应175***" },
    CodeRule { prefix: "176", typ: SecurityType::TypeBond, desc: "公开公司债", note: "公开发行公司债券" },
    CodeRule { prefix: "177", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "178", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "179", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },

    CodeRule { prefix: "180", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },
    CodeRule { prefix: "181", typ: SecurityType::TypeBond, desc: "可转债转股/非公开公司债", note: "对应600*** 的转股等/182000 系列为回售或非公开" },
    CodeRule { prefix: "182", typ: SecurityType::TypeBond, desc: "债券回售/非公开公司债", note: "182000-182299 用于债券回售；182300-182999 用于非公开发行公司债券" },
    CodeRule { prefix: "183", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },
    CodeRule { prefix: "184", typ: SecurityType::TypeBond, desc: "企业债/政府支持债", note: "184000-184799 企业债券；184800-184999 政府支持债（中国铁路建设债专用）" },
    CodeRule { prefix: "185", typ: SecurityType::TypeBond, desc: "公开公司债", note: "公开发行公司债券" },
    CodeRule { prefix: "186", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },
    CodeRule { prefix: "187", typ: SecurityType::TypeBond, desc: "公开公司债质押出入库", note: "对应188***" },
    CodeRule { prefix: "188", typ: SecurityType::TypeBond, desc: "公开公司债质押出入库", note: "对应188***" },
    CodeRule { prefix: "189", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },

    CodeRule { prefix: "190", typ: SecurityType::TypeBond, desc: "可转债转股", note: "对应600***（已不再增用部分）" },
    CodeRule { prefix: "191", typ: SecurityType::TypeBond, desc: "可转债转股", note: "191000-191499 对应601***；191500-191999 对应603***" },
    CodeRule { prefix: "192", typ: SecurityType::TypeBond, desc: "可交换债换股", note: "对应132***" },
    CodeRule { prefix: "193", typ: SecurityType::TypeBond, desc: "创新创业转股/ABS", note: "193000-193099 创新创业公司非公开可转债转股（对应145900-145999）；193100-193999 用于资产支持证券" },
    CodeRule { prefix: "194", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "195", typ: SecurityType::TypeBond, desc: "可转债转股", note: "195000-195499 用于可转债转股，对应605***" },
    CodeRule { prefix: "196", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "197", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "198", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },
    CodeRule { prefix: "199", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },

    CodeRule { prefix: "201", typ: SecurityType::TypeBond, desc: "国债回购", note: "国债回购（席位托管方式）" },
    CodeRule { prefix: "202", typ: SecurityType::TypeBond, desc: "企业债回购", note: "企业债回购（席位托管方式）" },
    CodeRule { prefix: "203", typ: SecurityType::TypeBond, desc: "国债买断式回购", note: "国债买断式回购" },
    CodeRule { prefix: "204", typ: SecurityType::TypeBond, desc: "债券质押式回购(账户托管)", note: "债券质押式回购（账户托管方式）" },
    CodeRule { prefix: "205", typ: SecurityType::TypeBond, desc: "质押式报价回购", note: "质押式报价回购" },
    CodeRule { prefix: "206", typ: SecurityType::TypeBond, desc: "质押式协议回购", note: "债券质押式协议回购" },
    CodeRule { prefix: "207", typ: SecurityType::TypeBond, desc: "质押式三方回购", note: "债券质押式三方回购" },
    CodeRule { prefix: "208", typ: SecurityType::TypeBond, desc: "债券借贷", note: "208000-208009 用于债券借贷业务" },

    CodeRule { prefix: "230", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },
    CodeRule { prefix: "231", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },
    CodeRule { prefix: "232", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },
    CodeRule { prefix: "233", typ: SecurityType::TypeBond, desc: "地方政府债", note: "地方政府债券" },

    CodeRule { prefix: "240", typ: SecurityType::TypeBond, desc: "公开公司债", note: "公开发行公司债券" },
    CodeRule { prefix: "241", typ: SecurityType::TypeBond, desc: "公开公司债", note: "公开发行公司债券" },

    CodeRule { prefix: "250", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "251", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "252", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "253", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "254", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "255", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "256", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },
    CodeRule { prefix: "257", typ: SecurityType::TypeBond, desc: "非公开公司债", note: "非公开发行公司债券" },

    CodeRule { prefix: "260", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },
    CodeRule { prefix: "261", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },
    CodeRule { prefix: "262", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },
    CodeRule { prefix: "263", typ: SecurityType::TypeBond, desc: "资产支持证券", note: "资产支持证券" },

    CodeRule { prefix: "270", typ: SecurityType::TypeBond, desc: "企业债", note: "企业债券" },
    CodeRule { prefix: "271", typ: SecurityType::TypeBond, desc: "企业债", note: "企业债券" },
    CodeRule { prefix: "272", typ: SecurityType::TypeBond, desc: "企业债", note: "企业债券" },

    CodeRule { prefix: "310", typ: SecurityType::TypeBond, desc: "国债期货", note: "国债期货（已暂停）" },
    CodeRule { prefix: "330", typ: SecurityType::TypeIPO, desc: "优先股(公开)", note: "公开发行优先股" },
    CodeRule { prefix: "360", typ: SecurityType::TypeOther, desc: "非公开优先股", note: "非公开发行优先股" },

    CodeRule { prefix: "500", typ: SecurityType::TypeFund, desc: "封闭式基金", note: "契约型封闭式基金" },
    CodeRule { prefix: "501", typ: SecurityType::TypeFund, desc: "上市开放式基金", note: "上市开放式基金" },
    CodeRule { prefix: "502", typ: SecurityType::TypeFund, desc: "上市开放式基金", note: "上市开放式基金" },
    CodeRule { prefix: "505", typ: SecurityType::TypeFund, desc: "创新封闭式基金", note: "505800-505899 用于创新型封闭式证券投资基金" },
    CodeRule { prefix: "506", typ: SecurityType::TypeFund, desc: "科创板LOF", note: "506000-506099 用于科创板相关 LOF" },
    CodeRule { prefix: "508", typ: SecurityType::TypeFund, desc: "公募REITs", note: "508000-508099 用于公募 REITs" },
    CodeRule { prefix: "511", typ: SecurityType::TypeETF, desc: "债券ETF/货基", note: "511000-511299 单市场债券（沪）ETF；511300-511599 现金申赎类债券ETF；511600-511999 交易型货币基金" },
    CodeRule { prefix: "517", typ: SecurityType::TypeETF, desc: "跨市场股票ETF", note: "517000-517999 用于跨市场股票（沪港深京）ETF" },
    CodeRule { prefix: "520", typ: SecurityType::TypeETF, desc: "跨境ETF", note: "520500-520999 用于跨境 ETF" },
    CodeRule { prefix: "588", typ: SecurityType::TypeETF, desc: "科创板ETF", note: "588000-588299 单市场（科创板）ETF；588300-588699 跨市场（含科创板）ETF；588700-588999 单市场（科创板）ETF" },
    CodeRule { prefix: "519", typ: SecurityType::TypeFund, desc: "开放式基金申赎/认购", note: "519*** 系列用于开放式基金的申赎/认购/跨市场转托管/分红/转换等" },
    CodeRule { prefix: "580", typ: SecurityType::TypeWarrant, desc: "权证", note: "含股改权证、公司权证；582/582x 可用于权证行权等" },

    CodeRule { prefix: "600", typ: SecurityType::TypeStock, desc: "主板A股", note: "主板 A 股" },
    CodeRule { prefix: "601", typ: SecurityType::TypeStock, desc: "主板A股", note: "主板 A 股" },
    CodeRule { prefix: "603", typ: SecurityType::TypeStock, desc: "主板A股", note: "主板 A 股" },
    CodeRule { prefix: "605", typ: SecurityType::TypeStock, desc: "主板A股", note: "主板 A 股（配套号段）" },
    CodeRule { prefix: "688", typ: SecurityType::TypeStock, desc: "科创板", note: "科创板股票" },
    CodeRule { prefix: "689", typ: SecurityType::TypeStock, desc: "科创板存托凭证", note: "科创板存托凭证" },

    CodeRule { prefix: "700", typ: SecurityType::TypeOther, desc: "配股", note: "配股（对应600***）" },
    CodeRule { prefix: "701", typ: SecurityType::TypeOther, desc: "转配股", note: "转配股" },
    CodeRule { prefix: "702", typ: SecurityType::TypeOther, desc: "职工股配股", note: "对应600***" },
    CodeRule { prefix: "703", typ: SecurityType::TypeOther, desc: "配售", note: "配售" },
    CodeRule { prefix: "704", typ: SecurityType::TypeOther, desc: "可转债配债", note: "可转换公司债券持股配债（对应600***）" },
    CodeRule { prefix: "706", typ: SecurityType::TypeOther, desc: "要约收购/现金选择权", note: "706000-706599 主板；706600-706999 科创板" },
    CodeRule { prefix: "707", typ: SecurityType::TypeOther, desc: "网上按市值申购/增发", note: "对应605***" },
    CodeRule { prefix: "708", typ: SecurityType::TypeOther, desc: "网上按市值申购配号", note: "对应605***" },
    CodeRule { prefix: "713", typ: SecurityType::TypeOther, desc: "可转债申购", note: "对应605***" },
    CodeRule { prefix: "714", typ: SecurityType::TypeOther, desc: "可转债申购配号", note: "对应605***" },
    CodeRule { prefix: "715", typ: SecurityType::TypeOther, desc: "可转债持股配债", note: "对应605***" },
    CodeRule { prefix: "718", typ: SecurityType::TypeOther, desc: "科创板可转债申购", note: "对应118000-118499" },
    CodeRule { prefix: "726", typ: SecurityType::TypeOther, desc: "科创板可转债配债", note: "对应118000-118499" },
    CodeRule { prefix: "730", typ: SecurityType::TypeIPO, desc: "新股申购", note: "新股申购/网上申购" },
    CodeRule { prefix: "758", typ: SecurityType::TypeOther, desc: "可交换债配号", note: "758000-758099" },
    CodeRule { prefix: "759", typ: SecurityType::TypeOther, desc: "可交换债申购", note: "759000-759099" },
    CodeRule { prefix: "786", typ: SecurityType::TypeOther, desc: "科创板配售/存托配售", note: "786000-786899 科创板股票配售；786900-786999 科创板存托凭证配售" },
    CodeRule { prefix: "799", typ: SecurityType::TypeOther, desc: "特殊业务代码", note: "指定交易/融资融券/网络投票/资金前端控制/身份认证等（见799xxx 具体编码）" },

    CodeRule { prefix: "880", typ: SecurityType::TypeBlock, desc: "板块指数", note: "通达信" },
    CodeRule { prefix: "881", typ: SecurityType::TypeBlock, desc: "板块指数", note: "通达信" },
    CodeRule { prefix: "888", typ: SecurityType::TypeBond, desc: "标准券", note: "888880 为新标准券，用于债券回购转换成标准券" },

    CodeRule { prefix: "900", typ: SecurityType::TypeBStock, desc: "B股", note: "B 股" },
    CodeRule { prefix: "901", typ: SecurityType::TypeBStock, desc: "B转H", note: "901000-901099 用于 B 转 H" },
    CodeRule { prefix: "938", typ: SecurityType::TypeOther, desc: "网络投票", note: "对应 B 股（不再增用）" },
    CodeRule { prefix: "939", typ: SecurityType::TypeOther, desc: "密码服务", note: "939988 用于 B 股网络投票密码服务" },

    CodeRule { prefix: "0", typ: SecurityType::TypeIndex, desc: "指数/国债", note: "首位 0：指数、国债" },
    CodeRule { prefix: "1", typ: SecurityType::TypeBond, desc: "债券现券", note: "首位 1：债券现券" },
    CodeRule { prefix: "2", typ: SecurityType::TypeBond, desc: "债券回购/借贷", note: "首位 2：债券回购、债券借贷等" },
    CodeRule { prefix: "3", typ: SecurityType::TypeOther, desc: "优先股/国债期货", note: "首位 3：优先股、国债期货（已暂停）" },
    CodeRule { prefix: "4", typ: SecurityType::TypeOther, desc: "备用", note: "首位 4：备用" },
    CodeRule { prefix: "5", typ: SecurityType::TypeFund, desc: "基金/REITs/权证", note: "首位 5：基金、公募 REITs、权证" },
    CodeRule { prefix: "6", typ: SecurityType::TypeStock, desc: "A股/存托凭证", note: "首位 6：A 股、存托凭证" },
    CodeRule { prefix: "7", typ: SecurityType::TypeOther, desc: "非交易业务", note: "首位 7：非交易业务" },
    CodeRule { prefix: "8", typ: SecurityType::TypeBond, desc: "标准券/备用", note: "首位 8：标准券、备用" },
    CodeRule { prefix: "9", typ: SecurityType::TypeBStock, desc: "B股", note: "首位 9：B 股" },
];

const SZSE_RULES: &[CodeRule] = &[
    CodeRule { prefix: "395", typ: SecurityType::TypeIndex, desc: "成交量统计指数", note: "" },
    CodeRule { prefix: "399", typ: SecurityType::TypeIndex, desc: "深证指数", note: "" },
    CodeRule { prefix: "000", typ: SecurityType::TypeStock, desc: "主板A股", note: "" },
    CodeRule { prefix: "001", typ: SecurityType::TypeStock, desc: "主板A股", note: "" },
    CodeRule { prefix: "002", typ: SecurityType::TypeStock, desc: "主板A股", note: "" },
    CodeRule { prefix: "003", typ: SecurityType::TypeStock, desc: "主板A股", note: "" },
    CodeRule { prefix: "030", typ: SecurityType::TypeWarrant, desc: "权证", note: "" },
    CodeRule { prefix: "031", typ: SecurityType::TypeWarrant, desc: "权证", note: "" },
    CodeRule { prefix: "032", typ: SecurityType::TypeWarrant, desc: "权证", note: "" },
    CodeRule { prefix: "036", typ: SecurityType::TypeWarrant, desc: "创业板股权激励计划涉及的员工认股权", note: "" },
    CodeRule { prefix: "0370", typ: SecurityType::TypeWarrant, desc: "主板A股股权激励计划涉及的员工认股权", note: "" },
    CodeRule { prefix: "0371", typ: SecurityType::TypeWarrant, desc: "主板A股股权激励计划涉及的员工认股权", note: "" },
    CodeRule { prefix: "0372", typ: SecurityType::TypeWarrant, desc: "创业板股权激励计划审计的员工认股权", note: "" },
    CodeRule { prefix: "0373", typ: SecurityType::TypeWarrant, desc: "主板A股股权激励计划涉及的员工认股权", note: "" },
    CodeRule { prefix: "0374", typ: SecurityType::TypeWarrant, desc: "主板A股股权激励计划涉及的员工认股权", note: "" },
    CodeRule { prefix: "0375", typ: SecurityType::TypeWarrant, desc: "中小企业板股权激励计划涉及的员工认股权", note: "" },
    CodeRule { prefix: "0376", typ: SecurityType::TypeWarrant, desc: "中小企业板股权激励计划涉及的员工认股权", note: "" },
    CodeRule { prefix: "0377", typ: SecurityType::TypeWarrant, desc: "中小企业板股权激励计划涉及的员工认股权", note: "" },
    CodeRule { prefix: "0378", typ: SecurityType::TypeWarrant, desc: "中小企业板股权激励计划涉及的员工认股权", note: "" },
    CodeRule { prefix: "0379", typ: SecurityType::TypeWarrant, desc: "中小企业板股权激励计划涉及的员工认股权", note: "" },
    CodeRule { prefix: "038", typ: SecurityType::TypeWarrant, desc: "主板A股及中小企业股票认沽权证", note: "" },
    CodeRule { prefix: "039", typ: SecurityType::TypeWarrant, desc: "主板A股及中小企业股票认沽权证", note: "" },
    CodeRule { prefix: "070", typ: SecurityType::TypeWarrant, desc: "主板A股增发/可转债申购", note: "" },
    CodeRule { prefix: "071", typ: SecurityType::TypeWarrant, desc: "主板A股增发/可转债申购", note: "" },
    CodeRule { prefix: "072", typ: SecurityType::TypeWarrant, desc: "中小企业板增发/可转债申购", note: "" },
    CodeRule { prefix: "073", typ: SecurityType::TypeWarrant, desc: "中小企业板增发/可转债申购", note: "" },
    CodeRule { prefix: "074", typ: SecurityType::TypeWarrant, desc: "中小企业板增发/可转债申购", note: "" },
    CodeRule { prefix: "080", typ: SecurityType::TypeWarrant, desc: "A股配股", note: "" },
    CodeRule { prefix: "0", typ: SecurityType::TypeStock, desc: "股票", note: "" },
    CodeRule { prefix: "10", typ: SecurityType::TypeBond, desc: "国债", note: "" },
    CodeRule { prefix: "11", typ: SecurityType::TypeBond, desc: "企业债", note: "" },
    CodeRule { prefix: "120", typ: SecurityType::TypeBond, desc: "企业债券", note: "" },
    CodeRule { prefix: "123", typ: SecurityType::TypeBond, desc: "可转债", note: "" },
    CodeRule { prefix: "127", typ: SecurityType::TypeBond, desc: "可转债", note: "" },
    CodeRule { prefix: "128", typ: SecurityType::TypeBond, desc: "可转债", note: "" },
    CodeRule { prefix: "13", typ: SecurityType::TypeBond, desc: "债券回购", note: "" },
    CodeRule { prefix: "159", typ: SecurityType::TypeETF, desc: "深交所ETF", note: "" },
    CodeRule { prefix: "15", typ: SecurityType::TypeFund, desc: "ETF", note: "" },
    CodeRule { prefix: "16", typ: SecurityType::TypeFund, desc: "LOF", note: "" },
    CodeRule { prefix: "17", typ: SecurityType::TypeFund, desc: "传统投资基金", note: "" },
    CodeRule { prefix: "184", typ: SecurityType::TypeFund, desc: "封闭式基金", note: "" },
    CodeRule { prefix: "18", typ: SecurityType::TypeFund, desc: "封闭式基金", note: "" },
    CodeRule { prefix: "1", typ: SecurityType::TypeBond, desc: "债券", note: "" },
    CodeRule { prefix: "200", typ: SecurityType::TypeBStock, desc: "B股", note: "" },
    CodeRule { prefix: "238", typ: SecurityType::TypeOther, desc: "B股现金选择权", note: "" },
    CodeRule { prefix: "28", typ: SecurityType::TypeOther, desc: "B股配股优先权", note: "" },
    CodeRule { prefix: "2", typ: SecurityType::TypeBStock, desc: "B股", note: "" },
    CodeRule { prefix: "300", typ: SecurityType::TypeStock, desc: "创业板", note: "" },
    CodeRule { prefix: "301", typ: SecurityType::TypeStock, desc: "创业板注册制", note: "" },
    CodeRule { prefix: "30", typ: SecurityType::TypeStock, desc: "创业板", note: "" },
    CodeRule { prefix: "36", typ: SecurityType::TypeOther, desc: "投票", note: "" },
    CodeRule { prefix: "37", typ: SecurityType::TypeOther, desc: "增发/可转债申购", note: "" },
    CodeRule { prefix: "38", typ: SecurityType::TypeOther, desc: "配股/可转债优先权", note: "" },
    CodeRule { prefix: "50", typ: SecurityType::TypeBond, desc: "资产支持证券ABS", note: "" },
    CodeRule { prefix: "56", typ: SecurityType::TypeBond, desc: "资产支持证券ABS", note: "" },
    CodeRule { prefix: "5", typ: SecurityType::TypeBond, desc: "资产支持证券ABS", note: "" },
    CodeRule { prefix: "700", typ: SecurityType::TypeWarrant, desc: "B股增发", note: "" },
    CodeRule { prefix: "730", typ: SecurityType::TypeWarrant, desc: "跨市场申购", note: "" },
];

const BJSE_RULES: &[CodeRule] = &[
    CodeRule { prefix: "899", typ: SecurityType::TypeIndex, desc: "指数", note: "证券指数首三位代码为899" },
    CodeRule { prefix: "920", typ: SecurityType::TypeStock, desc: "北交所新上市", note: "2024-04-22 起新上市使用920号段；已上市公司继续沿用原代码直到统一切换" },
    CodeRule { prefix: "92", typ: SecurityType::TypeStock, desc: "上市公司普通股", note: "首两位92：上市公司普通股票；920号段自2024-04-22起用于新上市公司" },
    CodeRule { prefix: "400", typ: SecurityType::TypeStock, desc: "两网/退市A股", note: "两网公司及退市公司A股首三位代码为400" },
    CodeRule { prefix: "420", typ: SecurityType::TypeBStock, desc: "退市B股", note: "退市公司B股首三位代码为420" },
    CodeRule { prefix: "810", typ: SecurityType::TypeBond, desc: "可转换公司债", note: "向特定对象发行的可转换公司债券首三位代码为810" },
    CodeRule { prefix: "81", typ: SecurityType::TypeBond, desc: "优先股(极少)", note: "其他极少数代码" },
    CodeRule { prefix: "820", typ: SecurityType::TypeStock, desc: "优先股", note: "优先股票首三位代码为820" },
    CodeRule { prefix: "82", typ: SecurityType::TypeBond, desc: "优先股(极少)", note: "其他极少数代码" },
    CodeRule { prefix: "83", typ: SecurityType::TypeStock, desc: "挂牌公司普通股", note: "挂牌公司普通股票首两位为83" },
    CodeRule { prefix: "840", typ: SecurityType::TypeOther, desc: "要约收购", note: "要约收购证券代码首三位代码为840" },
    CodeRule { prefix: "841", typ: SecurityType::TypeOther, desc: "要约回购", note: "要约回购证券代码首三位代码为841" },
    CodeRule { prefix: "87", typ: SecurityType::TypeStock, desc: "挂牌公司普通股", note: "挂牌公司普通股票首两位为87" },
    CodeRule { prefix: "88", typ: SecurityType::TypeStock, desc: "挂牌公司普通股", note: "挂牌公司普通股票首两位为88" },
    CodeRule { prefix: "850", typ: SecurityType::TypeOption, desc: "股权激励期权", note: "股权激励期权首三位代码为850，简称后缀如 JLC1/JLC2 等" },
];

const HKSE_RULES: &[CodeRule] = &[
    CodeRule { prefix: "HSI", typ: SecurityType::TypeIndex, desc: "恒生指数", note: "" },
    CodeRule { prefix: "HSCEI", typ: SecurityType::TypeIndex, desc: "国企指数", note: "" },
    CodeRule { prefix: "HSCCI", typ: SecurityType::TypeIndex, desc: "红筹指数", note: "" },
    CodeRule { prefix: "028", typ: SecurityType::TypeETF, desc: "ETF", note: "" },
    CodeRule { prefix: "030", typ: SecurityType::TypeETF, desc: "ETF", note: "" },
    CodeRule { prefix: "031", typ: SecurityType::TypeETF, desc: "ETF", note: "" },
    CodeRule { prefix: "090", typ: SecurityType::TypeETF, desc: "ETF", note: "" },
    CodeRule { prefix: "091", typ: SecurityType::TypeETF, desc: "ETF", note: "" },
    CodeRule { prefix: "08", typ: SecurityType::TypeStock, desc: "港股", note: "GEM" },
    CodeRule { prefix: "0", typ: SecurityType::TypeStock, desc: "港股", note: "" },
    CodeRule { prefix: "1", typ: SecurityType::TypeBond, desc: "权证", note: "" },
    CodeRule { prefix: "2", typ: SecurityType::TypeBond, desc: "权证", note: "" },
    CodeRule { prefix: "4", typ: SecurityType::TypeBond, desc: "牛熊证", note: "" },
    CodeRule { prefix: "5", typ: SecurityType::TypeBond, desc: "牛熊证", note: "" },
    CodeRule { prefix: "6", typ: SecurityType::TypeBond, desc: "牛熊证", note: "" },
];

/// 根据给定的代码和规则列表匹配最符合的证券类型
///
/// 该函数通过比较代码前缀与规则列表中的前缀来匹配证券类型，
/// 返回匹配到的最长前缀对应的证券类型。
///
/// # 参数
/// * `code` - 待匹配的证券代码字符串
/// * `rules` - 证券代码规则列表，包含前缀和对应的证券类型
///
/// # 返回值
/// * `Some(SecurityType)` - 匹配到的最符合的证券类型
/// * `None` - 未匹配到任何规则
fn match_rule(code: &str, rules: &[CodeRule]) -> Option<SecurityType> {
    let mut best_len = 0usize;
    let mut matched: Option<SecurityType> = None;
    for r in rules.iter() {
        if code.starts_with(r.prefix) {
            let l = r.prefix.len();
            if l > best_len {
                best_len = l;
                matched = Some(r.typ);
            }
        }
    }
    matched
} // Closing brace for match_rule function

/// Detect 解析证券代码并返回 SecurityCode
pub fn detect(input: &str) -> SecurityCode {
    // Port of Go Detect: single-pass extraction then rule-based resolution
    let raw = input.trim();
    if raw.is_empty() {
        return SecurityCode::new(ExchangeId::ShangHai, "", SecurityType::TypeUnknown);
    }

    let pure_code = raw.to_lowercase();
    let mut symbol = String::new();
    let mut exchange_id = ExchangeId::Unknown;
    let mut typ = SecurityType::TypeUnknown;

    // All exchange flags (use exchange constants to stay in sync)
    let flags = [
        EXCHANGE_SSE.as_str(),
        EXCHANGE_SZSE.as_str(),
        EXCHANGE_BJSE.as_str(),
        EXCHANGE_HK.as_str(),
        EXCHANGE_US.as_str(),
    ];

    // 1. explicit market prefix
    if crate::std::strings::starts_with(&pure_code, &flags) {
        symbol = pure_code[2..].to_string();
        let flag = &pure_code[..2];
        exchange_id = if flag == EXCHANGE_SSE.as_str() {
            ExchangeId::ShangHai
        } else if flag == EXCHANGE_SZSE.as_str() {
            ExchangeId::ShenZhen
        } else if flag == EXCHANGE_BJSE.as_str() {
            ExchangeId::BeiJing
        } else if flag == EXCHANGE_HK.as_str() {
            ExchangeId::HongKong
        } else if flag == EXCHANGE_US.as_str() {
            ExchangeId::USA
        } else {
            ExchangeId::ShangHai
        };
    } else if crate::std::strings::ends_with(&pure_code, &flags)
        && pure_code.len() >= 3
        && pure_code.as_bytes()[pure_code.len() - 3] as char == '.'
    {
        // 2. explicit market suffix like 600000.sh or appl.us
        let len = pure_code.len();
        symbol = pure_code[..len - 3].to_string();
        let flag = &pure_code[len - 2..];
        exchange_id = if flag == EXCHANGE_SSE.as_str() {
            ExchangeId::ShangHai
        } else if flag == EXCHANGE_SZSE.as_str() {
            ExchangeId::ShenZhen
        } else if flag == EXCHANGE_BJSE.as_str() {
            ExchangeId::BeiJing
        } else if flag == EXCHANGE_HK.as_str() {
            ExchangeId::HongKong
        } else if flag == EXCHANGE_US.as_str() {
            ExchangeId::USA
        } else {
            ExchangeId::ShangHai
        };
    } else {
        // 3. plain form
        let code_len = pure_code.len();
        match code_len {
            4 => {
                if pure_code.chars().all(|c| c.is_ascii_lowercase()) {
                    exchange_id = ExchangeId::USA;
                    symbol = pure_code.clone();
                    typ = SecurityType::TypeStock;
                } else {
                    exchange_id = ExchangeId::Unknown;
                    symbol.clear();
                    typ = SecurityType::TypeUnknown;
                }
            }
            5 => {
                exchange_id = ExchangeId::HongKong;
                symbol = pure_code.clone();
            }
            6 => {
                // 6-digit: global rules first, then szse, bjse, sse (ordering per Go)
                if let Some(t) = match_rule(&pure_code, GLOBAL_RULES) {
                    return SecurityCode::new(ExchangeId::ShangHai, &pure_code, t);
                }
                if let Some(t) = match_rule(&pure_code, SZSE_RULES) {
                    return SecurityCode::new(ExchangeId::ShenZhen, &pure_code, t);
                }
                if let Some(t) = match_rule(&pure_code, BJSE_RULES) {
                    return SecurityCode::new(ExchangeId::BeiJing, &pure_code, t);
                }
                if let Some(t) = match_rule(&pure_code, SSE_RULES) {
                    return SecurityCode::new(ExchangeId::ShangHai, &pure_code, t);
                }
                // no match -- leave exchange_id unknown
                symbol = pure_code.clone();
            }
            _ => {}
        }
    }

    if exchange_id == ExchangeId::Unknown {
        return SecurityCode::new(ExchangeId::Unknown, "", SecurityType::TypeUnknown);
    }

    if typ == SecurityType::TypeUnknown {
        // derive type based on market rules
        let rules = match exchange_id {
            ExchangeId::ShangHai => SSE_RULES,
            ExchangeId::ShenZhen => SZSE_RULES,
            ExchangeId::BeiJing => BJSE_RULES,
            ExchangeId::HongKong => HKSE_RULES,
            ExchangeId::USA => &[],
            _ => &[],
        };
        if exchange_id == ExchangeId::USA {
            typ = SecurityType::TypeStock;
            return SecurityCode::new(exchange_id, &symbol, typ);
        }
        if let Some(t) = match_rule(&symbol, rules) {
            typ = t;
            return SecurityCode::new(exchange_id, &symbol, typ);
        } else {
            return SecurityCode::new(ExchangeId::Unknown, "", SecurityType::TypeUnknown);
        }
    } else {
        return SecurityCode::new(exchange_id, &symbol, typ);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::exchange::{ExchangeId, SecurityType};
    #[test]
    fn test_detect_scenarios_match_go() {
        let tests = vec![
            // From Go TestDetect_Scenarios: (name, in, expected Market, Symbol, Type)
            (
                "sh prefix",
                "sh600000",
                ExchangeId::ShangHai,
                "600000",
                SecurityType::TypeStock,
            ),
            (
                "plain 6-digit SSE",
                "600000",
                ExchangeId::ShangHai,
                "600000",
                SecurityType::TypeStock,
            ),
            (
                "sz prefix",
                "sz000001",
                ExchangeId::ShenZhen,
                "000001",
                SecurityType::TypeStock,
            ),
            (
                "hk suffix",
                "00700.hk",
                ExchangeId::HongKong,
                "00700",
                SecurityType::TypeStock,
            ),
            (
                "us suffix",
                "appl.us",
                ExchangeId::USA,
                "appl",
                SecurityType::TypeStock,
            ),
            (
                "us upper suffix",
                "APPL.US",
                ExchangeId::USA,
                "appl",
                SecurityType::TypeStock,
            ),
            // invalid / error formats
            (
                "too short numeric",
                "123",
                ExchangeId::Unknown,
                "",
                SecurityType::TypeUnknown,
            ),
            (
                "four digits numeric",
                "6006",
                ExchangeId::Unknown,
                "",
                SecurityType::TypeUnknown,
            ),
            (
                "four digits numeric dup",
                "6006",
                ExchangeId::Unknown,
                "",
                SecurityType::TypeUnknown,
            ),
            (
                "000001 (sz)",
                "000001",
                ExchangeId::ShenZhen,
                "000001",
                SecurityType::TypeStock,
            ),
            (
                "880005 (block->sh)",
                "880005",
                ExchangeId::ShangHai,
                "880005",
                SecurityType::TypeBlock,
            ),
            (
                "five digits -> hk",
                "60060",
                ExchangeId::HongKong,
                "60060",
                SecurityType::TypeBond,
            ),
            // From rule table
            (
                "global 880",
                "880000",
                ExchangeId::ShangHai,
                "880000",
                SecurityType::TypeBlock,
            ),
            (
                "global 881",
                "881000",
                ExchangeId::ShangHai,
                "881000",
                SecurityType::TypeBlock,
            ),
            // SSE
            (
                "sse ETF 51",
                "510000",
                ExchangeId::ShangHai,
                "510000",
                SecurityType::TypeETF,
            ),
            (
                "sse ETF 588",
                "588000",
                ExchangeId::ShangHai,
                "588000",
                SecurityType::TypeETF,
            ),
            (
                "sse fund 50",
                "500000",
                ExchangeId::ShangHai,
                "500000",
                SecurityType::TypeFund,
            ),
            (
                "sse fund 52",
                "520000",
                ExchangeId::ShangHai,
                "520000",
                SecurityType::TypeFund,
            ),
            (
                "sse stock 688",
                "688000",
                ExchangeId::ShangHai,
                "688000",
                SecurityType::TypeStock,
            ),
            (
                "sse stock 689",
                "689000",
                ExchangeId::ShangHai,
                "689000",
                SecurityType::TypeStock,
            ),
            (
                "sse bstock 900",
                "900000",
                ExchangeId::ShangHai,
                "900000",
                SecurityType::TypeBStock,
            ),
            (
                "sse ipo 730",
                "730000",
                ExchangeId::ShangHai,
                "730000",
                SecurityType::TypeIPO,
            ),
            // SZSE
            (
                "sz index 399",
                "399000",
                ExchangeId::ShenZhen,
                "399000",
                SecurityType::TypeIndex,
            ),
            (
                "sz etf 159",
                "159000",
                ExchangeId::ShenZhen,
                "159000",
                SecurityType::TypeETF,
            ),
            (
                "sz fund 150",
                "150000",
                ExchangeId::ShenZhen,
                "150000",
                SecurityType::TypeFund,
            ),
            (
                "sz gem 300",
                "300000",
                ExchangeId::ShenZhen,
                "300000",
                SecurityType::TypeStock,
            ),
            (
                "sz bstock 200",
                "200000",
                ExchangeId::ShenZhen,
                "200000",
                SecurityType::TypeBStock,
            ),
            // BJSE
            (
                "bj new 920",
                "920000",
                ExchangeId::BeiJing,
                "920000",
                SecurityType::TypeStock,
            ),
            (
                "bj 83",
                "830000",
                ExchangeId::BeiJing,
                "830000",
                SecurityType::TypeStock,
            ),
            (
                "bj 87",
                "870000",
                ExchangeId::BeiJing,
                "870000",
                SecurityType::TypeStock,
            ),
            (
                "bj bond 82",
                "820000",
                ExchangeId::BeiJing,
                "820000",
                SecurityType::TypeBond,
            ),
            // HK (5-digit)
            (
                "hk etf 028",
                "02800",
                ExchangeId::HongKong,
                "02800",
                SecurityType::TypeETF,
            ),
            (
                "hk stock 0",
                "00000",
                ExchangeId::HongKong,
                "00000",
                SecurityType::TypeStock,
            ),
        ];

        for (name, input, exp_market, exp_symbol, exp_type) in tests {
            let got = detect(input);
            assert_eq!(
                got.market, exp_market,
                "{}: market mismatch for {}",
                name, input
            );
            assert_eq!(
                got.symbol, exp_symbol,
                "{}: symbol mismatch for {}",
                name, input
            );
            assert_eq!(got.typ, exp_type, "{}: type mismatch for {}", name, input);
        }
    }
}
