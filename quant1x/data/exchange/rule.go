package exchange

import (
	"regexp"
	"strings"
)

// CodeRule 表示一条证券代码前缀规则
type CodeRule struct {
	Prefix string       // 前缀，如 "600", "920"
	Type   SecurityType // 类型
	Desc   string       // 描述(用于调试或日志)
	Note   string       // 备注
}

// ========== 全局规则(跨市场，优先匹配)==========
var globalRules = []CodeRule{
	{"880", SecurityTypeBlock, "板块指数", "通达信"},
	{"881", SecurityTypeBlock, "板块指数", "通达信"},
}

// ========== 上交所规则(SSE)==========
var sseRules = []CodeRule{
	// 0xx
	{"000", SecurityTypeIndex, "上证指数", "上证指数系列；000680-000689 用于科创板相关指数"},
	{"009", SecurityTypeBond, "国债", "国债（2000年前发行）"},
	{"010", SecurityTypeBond, "国债", "国债（2000-2009年发行）"},
	{"018", SecurityTypeBond, "政策性银行债", "政策性银行金融债"},
	{"019", SecurityTypeBond, "国债", "国债（2010年及以后发行）"},
	{"020", SecurityTypeBond, "记账式贴现国债", "记账式贴现国债"},
	{"090", SecurityTypeBond, "国债质押回购出入库", "国债质押式回购质押券出入库"},
	{"091", SecurityTypeBond, "国债质押回购出入库", "对应019***"},
	{"099", SecurityTypeBond, "国债质押回购出入库", "对应009***"},
	{"0", SecurityTypeIndex, "指数/国债", "首位 0：指数、国债"},

	// 1xx（按表逐项补全，Desc 简洁，Note 为备注）
	{"100", SecurityTypeBond, "债券回售/可转债", "100000-100899 用于可转换公司债券（对应600***）；100900-100999 用于债券回售（不再增用部分）"},
	{"101", SecurityTypeBond, "地方政府债", "地方政府债券"},
	{"102", SecurityTypeBond, "企业债质押出入库", "对应127000-127999"},
	{"103", SecurityTypeBond, "企业债质押出入库", "对应124000-124999"},
	{"104", SecurityTypeBond, "公司/企业债质押出入库", "104000-104499 用于公司债质押（对应122000-122499）；104500-104999 用于企业债质押（对应122500-122999）"},
	{"105", SecurityTypeBond, "债券质押出入库", "105000-105699 分离交易的可转债质押（对应126***）；105700-105799 债券ETF质押；105800-105899 可转债质押（对应110***、113***）；105900-105999 企业债质押（对应120***、129***）"},
	{"106", SecurityTypeBond, "地方政府债质押出入库", "对应130***"},
	{"107", SecurityTypeBond, "记账式贴现国债质押出入库", "对应020***"},
	{"108", SecurityTypeBond, "政策性银行债质押出入库", "对应018***"},
	{"109", SecurityTypeBond, "地方政府债", "地方政府债券"},

	{"110", SecurityTypeBond, "可转换公司债", "110000-110799 上市公司公开发行可转债（对应600***）；110800-110999 非公开发行"},
	{"111", SecurityTypeBond, "可转换公司债", "111000-111499 对应605***"},
	{"112", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"113", SecurityTypeBond, "可转换公司债", "113000-113499 对应601***；113500-113999 对应603***"},
	{"114", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"115", SecurityTypeBond, "公开公司债", "公开发行公司债券"},
	{"118", SecurityTypeBond, "科创板可转债", "118000-118499 用于科创板上市公司公开发行可转债"},

	{"120", SecurityTypeBond, "企业/公司债", "122000-122499 用于公司债券；122500-122999 用于企业债券（见122）"},
	{"121", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"122", SecurityTypeBond, "公司债/企业债", "122000-122499 用于公司债券；122500-122999 用于企业债券"},
	{"123", SecurityTypeBond, "公司/企业债/ABS", "123000-123499 用于企业/公司债；123500-123999 用于资产支持证券"},
	{"124", SecurityTypeBond, "企业债质押出入库", "对应124000-124999"},
	{"125", SecurityTypeBond, "中小企业私募债/非公开公司债", "中小企业私募债券、非公开发行公司债券"},
	{"126", SecurityTypeBond, "分离交易可转债", "分离交易的可转换公司债券"},
	{"127", SecurityTypeBond, "企业债", "127000-127899 用于企业债券；127900-127999 用于政府支持债（中国铁路建设债专用）"},
	{"128", SecurityTypeBond, "信贷资产支持证券", "信贷资产支持证券"},
	{"129", SecurityTypeBond, "企业债", "企业债券"},

	{"130", SecurityTypeBond, "地方政府债", "地方政府债券(对应130***)"},
	{"131", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"132", SecurityTypeBond, "可交换公司债", "可交换公司债券"},
	{"133", SecurityTypeBond, "可交换债质押出入库", "对应132***"},
	{"134", SecurityTypeBond, "公开公司债质押出入库", "对应136***"},
	{"135", SecurityTypeBond, "证券公司短期债/并购私募债", "证券公司短期债、并购重组私募债券、非公开发行公司债券"},
	{"136", SecurityTypeBond, "公开公司债质押出入库", "对应136***"},
	{"137", SecurityTypeBond, "可交换/公开公司债", "137000-137499 非公开可交换；137500-137999 公开公司债"},
	{"138", SecurityTypeBond, "可交换换股/公开公司债", "138000-138499 非公开可交换换股(对应137000-137499)；138500-138999 公开公司债"},
	{"139", SecurityTypeBond, "企业债", "企业债券"},

	{"140", SecurityTypeBond, "地方政府债质押出入库", "对应140***"},
	{"141", SecurityTypeBond, "地方政府债", "地方政府债券"},
	{"142", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"143", SecurityTypeBond, "公开公司债质押出入库", "对应143***"},
	{"144", SecurityTypeBond, "公开公司债", "公开发行公司债券"},
	{"145", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"146", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"147", SecurityTypeBond, "地方政府债质押出入库", "对应147***"},
	{"148", SecurityTypeBond, "地方政府债", "地方政府债券"},
	{"149", SecurityTypeBond, "资产支持证券", "资产支持证券"},

	{"150", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"151", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"152", SecurityTypeBond, "企业债质押出入库", "对应152***"},
	{"153", SecurityTypeBond, "企业债", "企业债券"},
	{"154", SecurityTypeBond, "公司债质押出入库", "对应155***"},
	{"155", SecurityTypeBond, "公司债质押出入库", "对应155***"},
	{"156", SecurityTypeBond, "公司债", "公司债券"},
	{"157", SecurityTypeBond, "地方政府债质押出入库", "对应157***"},
	{"158", SecurityTypeBond, "地方政府债", "地方政府债券"},
	{"159", SecurityTypeBond, "资产支持证券", "资产支持证券"},

	{"160", SecurityTypeBond, "地方政府债", "地方政府债券"},
	{"161", SecurityTypeBond, "地方政府债质押出入库", "对应160***"},
	{"162", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"163", SecurityTypeBond, "公开公司债质押出入库", "对应163***"},
	{"164", SecurityTypeBond, "公开公司债", "公开发行公司债券"},
	{"165", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"166", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"167", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"168", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"169", SecurityTypeBond, "资产支持证券", "资产支持证券"},

	{"170", SecurityTypeBond, "信用保护工具", "170000-170499 用于信用保护凭证；170900-170999 用于组合型信用保护合约"},
	{"171", SecurityTypeBond, "地方政府债质押出入库", "对应171***"},
	{"172", SecurityTypeBond, "地方政府债", "地方政府债券"},
	{"173", SecurityTypeBond, "地方政府债质押出入库", "对应173***"},
	{"174", SecurityTypeBond, "地方政府债", "地方政府债券"},
	{"175", SecurityTypeBond, "公开公司债质押出入库", "对应175***"},
	{"176", SecurityTypeBond, "公开公司债", "公开发行公司债券"},
	{"177", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"178", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"179", SecurityTypeBond, "资产支持证券", "资产支持证券"},

	{"180", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"181", SecurityTypeBond, "可转债转股/非公开公司债", "对应600*** 的转股等/182000 系列为回售或非公开"},
	{"182", SecurityTypeBond, "债券回售/非公开公司债", "182000-182299 用于债券回售；182300-182999 用于非公开发行公司债券"},
	{"183", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"184", SecurityTypeBond, "企业债/政府支持债", "184000-184799 企业债券；184800-184999 政府支持债（中国铁路建设债专用）"},
	{"185", SecurityTypeBond, "公开公司债", "公开发行公司债券"},
	{"186", SecurityTypeBond, "地方政府债", "地方政府债券"},
	{"187", SecurityTypeBond, "公开公司债质押出入库", "对应188***"},
	{"188", SecurityTypeBond, "公开公司债质押出入库", "对应188***"},
	{"189", SecurityTypeBond, "资产支持证券", "资产支持证券"},

	{"190", SecurityTypeBond, "可转债转股", "对应600***"},
	{"191", SecurityTypeBond, "可转债转股", "191000-191499 对应601***；191500-191999 对应603***"},
	{"192", SecurityTypeBond, "可交换债换股", "对应132***"},
	{"193", SecurityTypeBond, "创新创业转股/ABS", "193000-193099 创新创业公司非公开可转债转股（对应145900-145999）；193100-193999 用于资产支持证券"},
	{"194", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"195", SecurityTypeBond, "可转债转股", "195000-195499 用于可转债转股，对应605***"},
	{"196", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"197", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"198", SecurityTypeBond, "地方政府债", "地方政府债券"},
	{"199", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"1", SecurityTypeBond, "债券现券", "首位 1：债券现券"},

	// 2xx
	{"201", SecurityTypeBond, "国债回购", "国债回购（席位托管方式）"},
	{"202", SecurityTypeBond, "企业债回购", "企业债回购（席位托管方式）"},
	{"203", SecurityTypeBond, "国债买断式回购", "国债买断式回购"},
	{"204", SecurityTypeBond, "债券质押式回购(账户托管)", "债券质押式回购（账户托管方式）"},
	{"205", SecurityTypeBond, "质押式报价回购", "质押式报价回购"},
	{"206", SecurityTypeBond, "质押式协议回购", "债券质押式协议回购"},
	{"207", SecurityTypeBond, "质押式三方回购", "债券质押式三方回购"},
	{"208", SecurityTypeBond, "债券借贷", "208000-208009 用于债券借贷业务"},

	{"230", SecurityTypeBond, "地方政府债", "地方政府债券"},
	{"231", SecurityTypeBond, "地方政府债", "地方政府债券"},
	{"232", SecurityTypeBond, "地方政府债", "地方政府债券"},
	{"233", SecurityTypeBond, "地方政府债", "地方政府债券"},

	{"240", SecurityTypeBond, "公开公司债", "公开发行公司债券"},
	{"241", SecurityTypeBond, "公开公司债", "公开发行公司债券"},

	{"250", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"251", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"252", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"253", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"254", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"255", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"256", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},
	{"257", SecurityTypeBond, "非公开公司债", "非公开发行公司债券"},

	{"260", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"261", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"262", SecurityTypeBond, "资产支持证券", "资产支持证券"},
	{"263", SecurityTypeBond, "资产支持证券", "资产支持证券"},

	{"270", SecurityTypeBond, "企业债", "企业债券"},
	{"271", SecurityTypeBond, "企业债", "企业债券"},
	{"272", SecurityTypeBond, "企业债", "企业债券"},
	{"2", SecurityTypeBond, "债券回购/借贷", "首位 2：债券回购、债券借贷等"},

	// 3xx
	{"310", SecurityTypeBond, "国债期货", "国债期货（已暂停）"},
	{"330", SecurityTypeIPO, "优先股(公开)", "公开发行优先股"},
	{"360", SecurityTypeOther, "非公开优先股", "非公开发行优先股"},
	{"3", SecurityTypeOther, "优先股/国债期货", "首位 3：优先股、国债期货（已暂停）"},

	// 4xx 备用
	{"4", SecurityTypeOther, "备用", "首位 4：备用"},

	// 5xx 基金/ETF/REITs/权证（保留已整理）
	{"500", SecurityTypeFund, "封闭式基金", "契约型封闭式基金"},
	{"501", SecurityTypeFund, "上市开放式基金", "上市开放式基金"},
	{"502", SecurityTypeFund, "上市开放式基金", "上市开放式基金"},
	{"505", SecurityTypeFund, "创新封闭式基金", "505800-505899 用于创新型封闭式证券投资基金"},
	{"506", SecurityTypeFund, "科创板LOF", "506000-506099 用于科创板相关 LOF"},
	{"508", SecurityTypeFund, "公募REITs", "508000-508099 用于公募 REITs"},
	{"510", SecurityTypeETF, "交易型开放式指数证券投资基金", "标的指数为沪市指数、跨市场指数或跨境指数"},
	{"511", SecurityTypeETF, "债券交易型指数基金 / 交易型货币基金", "511000-511299 用于单市场债券（沪）ETF；511300-511599 用于现金申赎类债券ETF；511600-511999 用于交易型货币市场基金"},
	{"512", SecurityTypeETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"},
	{"513", SecurityTypeETF, "交易型开放式指数证券投资基金", "标的指数为跨境指数"},
	{"515", SecurityTypeETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"},
	{"516", SecurityTypeETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"},
	{"517", SecurityTypeETF, "跨市场股票ETF", "517000-517999 用于跨市场股票（沪港深京）ETF"},
	{"518", SecurityTypeETF, "商品交易型开放式证券投资基金", "商品类 ETF"},
	{"519", SecurityTypeFund, "开放式基金申赎/认购", "519*** 系列用于开放式基金的申赎/认购/跨市场转托管/分红/转换等；5198** 用于实时申赎货币基金（实时申赎）"},
	{"520", SecurityTypeETF, "跨境ETF", "520500-520999 用于跨境 ETF"},
	{"521", SecurityTypeFund, "开放式基金认购", "对应519*** 系列的认购业务"},
	{"522", SecurityTypeFund, "开放式基金跨市场转托管", "对应519*** 系列的跨市场转托管业务"},
	{"523", SecurityTypeFund, "开放式基金分红", "对应519*** 系列的分红业务"},
	{"524", SecurityTypeFund, "开放式基金基金转换", "对应519*** 系列的基金转换业务"},
	{"530", SecurityTypeETF, "交易型开放式指数证券投资基金", "标的指数为沪市指数"},
	{"550", SecurityTypeFund, "基金", ""},
	{"560", SecurityTypeETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"},
	{"561", SecurityTypeETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"},
	{"562", SecurityTypeETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"},
	{"563", SecurityTypeETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"},
	{"580", SecurityTypeWarrant, "权证", "含股改权证、公司权证"},
	{"582", SecurityTypeWarrant, "权证行权", "用于权证行权/行权相关代码"},
	{"588", SecurityTypeETF, "科创板ETF", "588000-588299 单市场（科创板）ETF；588300-588699 跨市场（含科创板）ETF；588700-588999 单市场（科创板）ETF"},
	{"5", SecurityTypeETF, "基金/REITs/权证", "首位 5：基金、公募 REITs、权证"},

	// 6xx A股/科创板
	{"600", SecurityTypeStock, "主板A股", "主板 A 股"},
	{"601", SecurityTypeStock, "主板A股", "主板 A 股"},
	{"603", SecurityTypeStock, "主板A股", "主板 A 股"},
	{"605", SecurityTypeStock, "主板A股", "主板 A 股（配套号段）"},
	{"688", SecurityTypeStock, "科创板", "科创板股票"},
	{"689", SecurityTypeStock, "科创板存托凭证", "科创板存托凭证"},
	{"6", SecurityTypeStock, "A股/存托凭证", "首位 6：A 股、存托凭证"},

	// 7xx 非交易/配售/申购等
	{"700", SecurityTypeOther, "配股", "配股（对应600***）"},
	{"701", SecurityTypeOther, "转配股", "转配股"},
	{"702", SecurityTypeOther, "职工股配股", "对应600***"},
	{"703", SecurityTypeOther, "配售", "配售"},
	{"704", SecurityTypeOther, "可转债配债", "可转换公司债券持股配债（对应600***）"},
	{"706", SecurityTypeOther, "要约收购/现金选择权", "706000-706599 主板；706600-706999 科创板"},
	{"707", SecurityTypeOther, "网上按市值申购/增发", "对应605***"},
	{"708", SecurityTypeOther, "网上按市值申购配号", "对应605***"},
	{"709", SecurityTypeOther, "按市值配售配股", "不再增用"},
	{"713", SecurityTypeOther, "可转债申购", "对应605***"},
	{"714", SecurityTypeOther, "可转债申购配号", "对应605***"},
	{"715", SecurityTypeOther, "可转债持股配债", "对应605***"},
	{"716", SecurityTypeOther, "增发款", "对应605***"},
	{"717", SecurityTypeOther, "配股", "对应605***"},
	{"718", SecurityTypeOther, "科创板可转债申购", "对应118000-118499"},
	{"719", SecurityTypeOther, "科创板可转债申购配号", "对应118000-118499"},
	{"726", SecurityTypeOther, "科创板可转债配债", "对应118000-118499"},
	{"730", SecurityTypeIPO, "新股申购", "新股申购/网上申购"},
	{"731", SecurityTypeOther, "持股增发", "对应600***"},
	{"732", SecurityTypeOther, "网上按市值申购或增发", "对应603***"},
	{"733", SecurityTypeOther, "可转换公司债券申购", "对应600***"},
	{"734", SecurityTypeOther, "增发款", "对应603***"},
	{"735", SecurityTypeOther, "基金申购", "不再增用"},
	{"736", SecurityTypeOther, "网上按市值申购或增发配号", "对应603***"},
	{"737", SecurityTypeOther, "按市值配售", "不再增用"},
	{"738", SecurityTypeOther, "网络投票", "对应600***（技术调整不再增用）"},
	{"739", SecurityTypeOther, "按市值配售申购", "不再增用"},
	{"740", SecurityTypeOther, "增发款", "对应600***"},
	{"741", SecurityTypeOther, "网上按市值申购或增发配号", "对应600***"},
	{"742", SecurityTypeOther, "配股", "对应603***"},
	{"743", SecurityTypeOther, "可转换公司债券申购款", "对应600***"},
	{"744", SecurityTypeOther, "可转换公司债券配号", "对应600***"},
	{"745", SecurityTypeOther, "基金申购款", "不再增用"},
	{"746", SecurityTypeOther, "基金申购配号", "不再增用"},
	{"747", SecurityTypeOther, "按市值配售", "不再增用"},
	{"748", SecurityTypeOther, "按市值配售", "不再增用"},
	{"749", SecurityTypeOther, "按市值配售配号", "不再增用"},
	{"750", SecurityTypeOther, "国债承销发行", ""},
	{"751", SecurityTypeOther, "国债预发行及债券分销", "751000-751199用于国债分销；751200-751399用于政策性银行金融债券分销；751400-751599用于地方政府债券网上分销；751600-751799用于国债分销；751800-751809用于利率招标国债预发行交易；751810-751819用于价格招标国债预发行交易；751850-751899用于面向专业投资者公开发行公司债券网上分销；751900-751969用于地方政府债券网上分销；751970-751999用于公司债券及企业债分销"},
	{"752", SecurityTypeOther, "网络投票", "对应603***，由于技术调整不再增用"},
	{"753", SecurityTypeOther, "可转换公司债券持股配债", "对应603***"},
	{"754", SecurityTypeOther, "可转换公司债券申购", "对应603***"},
	{"755", SecurityTypeOther, "可转换公司债券申购款", "对应603***"},
	{"756", SecurityTypeOther, "可转换公司债券配号", "对应603***"},
	{"758", SecurityTypeOther, "可交换公司债券网上发行配号", "758000-758099 用于可交换公司债券网上发行配号"},
	{"759", SecurityTypeOther, "可交换公司债券网上发行申购", "759000-759099 用于可交换公司债券网上发行申购"},
	{"760", SecurityTypeOther, "配股", "对应601***"},
	{"762", SecurityTypeOther, "职工股配股", "对应601***"},
	{"764", SecurityTypeOther, "可转换公司债券持股配债", "对应601***"},
	{"770", SecurityTypeOther, "公开发行优先股申购", "对应330***"},
	{"771", SecurityTypeOther, "公开发行优先股配股、配售", "对应330***"},
	{"772", SecurityTypeOther, "公开发行优先股申购款", "对应330***"},
	{"773", SecurityTypeOther, "公开发行优先股申购配号", "对应330***"},
	{"780", SecurityTypeOther, "网上按市值申购或增发", "对应601***"},
	{"781", SecurityTypeOther, "持股增发", "对应601***"},
	{"783", SecurityTypeOther, "可转换公司债券申购", "对应601***"},
	{"785", SecurityTypeOther, "科创板股票配股", "对应688***"},
	{"786", SecurityTypeOther, "科创板上市公司股东以配售方式减持股份业务（简称科创板配售业务）", "786000-786899用于科创板股票配售；786900-786999用于科创板存托凭证配售"},
	{"787", SecurityTypeOther, "科创板股票网上申购", "对应688***"},
	{"788", SecurityTypeOther, "网络投票", "对应601***（技术调整不再增用）"},
	{"789", SecurityTypeOther, "科创板股票网上申购配号", "对应688***"},
	{"790", SecurityTypeOther, "增发款", "对应601***"},
	{"791", SecurityTypeOther, "网上按市值申购或增发配号", "对应601***"},
	{"793", SecurityTypeOther, "可转换公司债券申购款", "对应601***"},
	{"794", SecurityTypeOther, "可转换公司债券配号", "对应601***"},
	{"795", SecurityTypeOther, "科创板存托凭证网上申购", "对应689***"},
	{"796", SecurityTypeOther, "科创板存托凭证网上申购配号", "对应689***"},
	{"799", SecurityTypeOther, "特殊业务代码", "指定交易/融资融券/网络投票/资金前端控制/身份认证等（见799xxx 具体编码）"},
	{"7", SecurityTypeOther, "非交易业务", "首位 7：非交易业务"},

	// 8xx 标准券
	{"880", SecurityTypeBlock, "板块指数", "通达信"},
	{"881", SecurityTypeBlock, "板块指数", "(通达信"},
	{"888", SecurityTypeBond, "标准券", "888880 为新标准券，用于债券回购转换成标准券"},
	{"8", SecurityTypeBond, "标准券/备用", "首位 8：标准券、备用"},

	// 9xx B股
	{"900", SecurityTypeStockB, "B股", "B 股"},
	{"901", SecurityTypeStockB, "B转H", "901000-901099 用于 B 转 H"},
	{"938", SecurityTypeOther, "网络投票", "对应 B 股（不再增用）"},
	{"939", SecurityTypeOther, "密码服务", "939988 用于 B 股网络投票密码服务"},
	{"970", SecurityTypeWarrant, "B股配股权证", ""},
	{"9", SecurityTypeStockB, "B股", "首位 9：B 股"},
}

// ========== 深交所规则(SZSE)==========
var szseRules = []CodeRule{
	// 指数
	{"395", SecurityTypeIndex, "成交量统计指数", ""},
	{"399", SecurityTypeIndex, "深证指数", ""},
	// A股(主板 + 创业板)
	{"000", SecurityTypeStock, "主板A股", ""},
	{"001", SecurityTypeStock, "主板A股", ""},
	{"002", SecurityTypeStock, "主板A股", ""},
	{"003", SecurityTypeStock, "主板A股", ""},
	// 认购权证
	{"030", SecurityTypeWarrant, "权证", ""},
	{"031", SecurityTypeWarrant, "权证", ""},
	{"032", SecurityTypeWarrant, "权证", ""},
	// 股权激励计划
	{"036", SecurityTypeWarrant, "创业板股权激励计划涉及的员工认股权", ""},
	{"0370", SecurityTypeWarrant, "主板A股股权激励计划涉及的员工认股权", ""},
	{"0371", SecurityTypeWarrant, "主板A股股权激励计划涉及的员工认股权", ""},
	{"0372", SecurityTypeWarrant, "创业板股权激励计划审计的员工认股权", ""},
	{"0373", SecurityTypeWarrant, "主板A股股权激励计划涉及的员工认股权", ""},
	{"0374", SecurityTypeWarrant, "主板A股股权激励计划涉及的员工认股权", ""},
	{"0375", SecurityTypeWarrant, "中小企业板股权激励计划涉及的员工认股权", ""},
	{"0376", SecurityTypeWarrant, "中小企业板股权激励计划涉及的员工认股权", ""},
	{"0377", SecurityTypeWarrant, "中小企业板股权激励计划涉及的员工认股权", ""},
	{"0378", SecurityTypeWarrant, "中小企业板股权激励计划涉及的员工认股权", ""},
	{"0379", SecurityTypeWarrant, "中小企业板股权激励计划涉及的员工认股权", ""},
	// 认沽权证
	{"038", SecurityTypeWarrant, "主板A股及中小企业股票认沽权证", ""},
	{"039", SecurityTypeWarrant, "主板A股及中小企业股票认沽权证", ""},
	// 增发/可转债申购
	{"070", SecurityTypeWarrant, "主板A股增发/可转债申购", ""},
	{"071", SecurityTypeWarrant, "主板A股增发/可转债申购", ""},
	{"072", SecurityTypeWarrant, "中小企业板增发/可转债申购", ""},
	{"073", SecurityTypeWarrant, "中小企业板增发/可转债申购", ""},
	{"074", SecurityTypeWarrant, "中小企业板增发/可转债申购", ""},
	{"080", SecurityTypeWarrant, "A股配股", ""},
	// 0开头为A股
	{"0", SecurityTypeStock, "股票", ""},
	// 债券
	{"10", SecurityTypeBond, "国债", ""},
	{"11", SecurityTypeBond, "企业债", ""},
	{"120", SecurityTypeBond, "企业债券", ""},
	{"123", SecurityTypeBond, "可转债", ""},
	{"127", SecurityTypeBond, "可转债", ""},
	{"128", SecurityTypeBond, "可转债", ""},
	{"13", SecurityTypeBond, "债券回购", ""},
	// ETF
	{"159", SecurityTypeETF, "深交所ETF", ""},
	{"15", SecurityTypeFund, "ETF", ""},
	// 其他基金
	{"16", SecurityTypeFund, "LOF", ""},
	{"17", SecurityTypeFund, "传统投资基金", ""},
	{"184", SecurityTypeFund, "封闭式基金", ""},
	{"18", SecurityTypeFund, "封闭式基金", ""},
	// 1开头为债券
	{"1", SecurityTypeBond, "债券", ""},

	// B股
	{"200", SecurityTypeStockB, "B股", ""},
	{"238", SecurityTypeOther, "B股现金选择权", ""},
	{"28", SecurityTypeOther, "B股配股优先权", ""},
	// 2开头为B股
	{"2", SecurityTypeStockB, "B股", ""},
	// 创业板
	{"300", SecurityTypeStock, "创业板", ""},
	{"301", SecurityTypeStock, "创业板注册制", ""},
	{"30", SecurityTypeStock, "创业板", ""},
	// 其它
	{"36", SecurityTypeOther, "投票", ""},
	{"37", SecurityTypeOther, "增发/可转债申购", ""},
	{"38", SecurityTypeOther, "配股/可转债优先权", ""},

	// 资产支持证券ABS
	{"50", SecurityTypeBond, "资产支持证券ABS", ""},
	{"56", SecurityTypeBond, "资产支持证券ABS", ""},
	// 5开头为资产支持证券ABS
	{"5", SecurityTypeBond, "资产支持证券ABS", ""},

	{"700", SecurityTypeWarrant, "B股增发", ""},
	{"730", SecurityTypeWarrant, "跨市场申购", ""},
}

// ========== 北交所规则(BSE)==========
var bseRules = []CodeRule{
	// 北京证券交易所 & 全国股转系统 指引要点
	{"899", SecurityTypeIndex, "指数", "证券指数首三位代码为899"},
	{"920", SecurityTypeStock, "北交所新上市", "2024-04-22 起新上市使用920号段；已上市公司继续沿用原代码直到统一切换"},
	{"92", SecurityTypeStock, "上市公司普通股", "首两位92：上市公司普通股票；920号段自2024-04-22起用于新上市公司"},
	{"400", SecurityTypeStock, "两网/退市A股", "两网公司及退市公司A股首三位代码为400"},
	{"420", SecurityTypeStockB, "退市B股", "退市公司B股首三位代码为420"},
	{"810", SecurityTypeBond, "可转换公司债", "向特定对象发行的可转换公司债券首三位代码为810"},
	{"81", SecurityTypeBond, "优先股(极少)", "其他极少数代码"},
	{"821", SecurityTypeBond, "优先股", "优先股票首三位代码为820"},
	{"82", SecurityTypeBond, "优先股(极少)", "其他极少数代码"},
	{"83", SecurityTypeStock, "挂牌公司普通股", "挂牌公司普通股票首两位为83"},
	{"840", SecurityTypeOther, "要约收购", "要约收购证券代码首三位代码为840"},
	{"841", SecurityTypeOther, "要约回购", "要约回购证券代码首三位代码为841"},
	{"87", SecurityTypeStock, "挂牌公司普通股", "挂牌公司普通股票首两位为87"},
	{"88", SecurityTypeStock, "挂牌公司普通股", "挂牌公司普通股票首两位为88"},
	{"850", SecurityTypeOption, "股权激励期权", "股权激励期权首三位代码为850，简称后缀如 JLC1/JLC2 等"},
	//{"89", SecurityTypeBond, "可转债(极少)", "其他极少数代码"},
}

// ========== 港交所规则(HKEX)==========
var hkexRules = []CodeRule{
	// 指数
	{"HSI", SecurityTypeIndex, "恒生指数", ""},
	{"HSCEI", SecurityTypeIndex, "国企指数", ""},
	{"HSCCI", SecurityTypeIndex, "红筹指数", ""},
	// ETF
	{"028", SecurityTypeETF, "ETF", ""},
	{"030", SecurityTypeETF, "ETF", ""},
	{"031", SecurityTypeETF, "ETF", ""},
	{"090", SecurityTypeETF, "ETF", ""},
	{"091", SecurityTypeETF, "ETF", ""},
	// 股票 (5位数字)
	{"08", SecurityTypeStock, "港股", "GEM"},
	{"0", SecurityTypeStock, "港股", ""},
	// 权证/牛熊证 (5位数字)
	{"1", SecurityTypeBond, "权证", ""},
	{"2", SecurityTypeBond, "权证", ""},
	{"4", SecurityTypeBond, "牛熊证", ""},
	{"5", SecurityTypeBond, "牛熊证", ""},
	{"6", SecurityTypeBond, "牛熊证", ""},
}

var (
	reSixDigits  = regexp.MustCompile(`^\d{6}$`) // 6位数字
	reFiveDigits = regexp.MustCompile(`^\d{5}$`) // 5位数字
)

// matchRule 在规则列表中匹配最长前缀
func matchRule(code string, rules []CodeRule) (SecurityType, string) {
	bestLen := 0
	var matchedType SecurityType
	var matchedDesc string

	for _, rule := range rules {
		if strings.HasPrefix(code, rule.Prefix) {
			if len(rule.Prefix) > bestLen {
				bestLen = len(rule.Prefix)
				matchedType = rule.Type
				matchedDesc = rule.Desc
			}
		}
	}
	if bestLen > 0 {
		return matchedType, matchedDesc
	}
	return SecurityTypeUnknown, ""
}

// DetectSecurity 解析证券代码，返回(市场, 类型, 描述)
func DetectSecurity(input string) (Exchange, SecurityType, string) {
	// 标准化：去除空格、点，转小写
	s := strings.ToLower(strings.ReplaceAll(strings.TrimSpace(input), ".", ""))

	var market Exchange
	var ticker string

	// 1. 尝试解析显式市场标识(前缀或后缀)
	if len(s) >= 7 {
		if strings.HasPrefix(s, "sh") || strings.HasPrefix(s, "sz") || strings.HasPrefix(s, "bj") || strings.HasPrefix(s, "hk") {
			market = ParseExchangeCode(s[:2])
			ticker = s[2:]
		} else if strings.HasSuffix(s, "sh") || strings.HasSuffix(s, "sz") || strings.HasSuffix(s, "bj") || strings.HasSuffix(s, "hk") {
			market = ParseExchangeCode(s[len(s)-2:])
			ticker = s[:len(s)-2]
		}
	}

	// 2. 若无市场标识，自动推断市场
	if market == "" {
		if reSixDigits.MatchString(s) {
			// 6位数字，按市场划分, 为内地三个市场
			ticker = s
			switch {
			case strings.HasPrefix(ticker, "6") || strings.HasPrefix(ticker, "5") ||
				strings.HasPrefix(ticker, "9") || strings.HasPrefix(ticker, "7") ||
				strings.HasPrefix(ticker, "000"):
				market = ExchangeSSE
			case strings.HasPrefix(ticker, "0") || strings.HasPrefix(ticker, "3") ||
				strings.HasPrefix(ticker, "1") || strings.HasPrefix(ticker, "2"):
				market = ExchangeSZSE
			case strings.HasPrefix(ticker, "8") || strings.HasPrefix(ticker, "92"):
				market = ExchangeBSE
			default:
				return "", SecurityTypeUnknown, "无法识别市场"
			}
		} else if reFiveDigits.MatchString(s) {
			// 5位数字，默认为港交所
			ticker = s
			market = ExchangeHKEX
		} else {
			ticker = s
		}
	} else if ticker == "" {
		ticker = s
	}

	// 3. 验证 ticker 为5或6位纯数字
	if !regexp.MustCompile(`^\d{5,6}$`).MatchString(ticker) {
		return "", SecurityTypeUnknown, "代码格式错误(应为5或6位数字)"
	}

	// 4. 全局规则优先(如板块指数)
	if typ, desc := matchRule(ticker, globalRules); typ != SecurityTypeUnknown {
		return ExchangeSSE, typ, desc // 板块指数归属上证体系
	}

	// 5. 按市场匹配规则
	var rules []CodeRule
	switch market {
	case ExchangeSSE:
		rules = sseRules
	case ExchangeSZSE:
		rules = szseRules
	case ExchangeBSE:
		rules = bseRules
	case ExchangeHKEX:
		rules = hkexRules
	default:
		return market, SecurityTypeUnknown, "不支持的市场"
	}

	if typ, desc := matchRule(ticker, rules); typ != SecurityTypeUnknown {
		return market, typ, desc
	}

	return market, SecurityTypeUnknown, "未匹配到规则"
}
