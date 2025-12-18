# -*- coding: UTF-8 -*-
from dataclasses import dataclass
from enum import Enum
import re
from .code import ExchangeId, ExchangeSSE, ExchangeSZSE, ExchangeBJSE, ExchangeHK, ExchangeUS


class SecurityType(Enum):
    Unknown = 0
    Stock = 1
    ETF = 2
    Fund = 3
    Bond = 4
    BStock = 5
    IPO = 6
    Index = 7
    Block = 8
    Option = 9
    Future = 10
    Warrant = 11
    Forex = 12
    Commodity = 13
    Other = 255


@dataclass
class SecurityCode:
    exchange: ExchangeId
    symbol: str
    typ: SecurityType


# Exchange code strings used by detection
ALL_EXCHANGE_CODES = {
    ExchangeSSE: ExchangeId.ShangHai,
    ExchangeSZSE: ExchangeId.ShenZhen,
    ExchangeBJSE: ExchangeId.BeiJing,
    ExchangeHK: ExchangeId.HongKong,
    ExchangeUS: ExchangeId.USA,
}


# Rule tables (copied from C++ rule.cpp)
global_rules = [
    ("880", SecurityType.Block, "板块指数", "通达信"),
    ("881", SecurityType.Block, "板块指数", "通达信"),
]

# SSE rules aligned with exchange/rule.go
sse_rules = [
    ("000", SecurityType.Index, "上证指数", "上证指数系列；000680-000689 用于科创板相关指数"),
    ("009", SecurityType.Bond, "国债", "国债（2000年前发行）"),
    ("010", SecurityType.Bond, "国债", "国债（2000-2009年发行）"),
    ("018", SecurityType.Bond, "政策性银行债", "政策性银行金融债"),
    ("019", SecurityType.Bond, "国债", "国债（2010年及以后发行）"),
    ("020", SecurityType.Bond, "记账式贴现国债", "记账式贴现国债"),
    ("090", SecurityType.Bond, "国债质押回购出入库", "国债质押式回购质押券出入库"),
    ("091", SecurityType.Bond, "国债质押回购出入库", "对应019***"),
    ("099", SecurityType.Bond, "国债质押回购出入库", "对应009***"),

    ("100", SecurityType.Bond, "债券回售/可转债", "100000-100899 用于可转换公司债券（对应600***）；100900-100999 用于债券回售（不再增用部分）"),
    ("101", SecurityType.Bond, "地方政府债", "地方政府债券"),
    ("102", SecurityType.Bond, "企业债质押出入库", "对应127000-127999"),
    ("103", SecurityType.Bond, "企业债质押出入库", "对应124000-124999"),
    ("104", SecurityType.Bond, "公司/企业债质押出入库", "104000-104499 用于公司债质押（对应122000-122499）；104500-104999 用于企业债质押（对应122500-122999）"),
    ("105", SecurityType.Bond, "债券质押出入库", "105000-105699 分离交易的可转债质押（对应126***）；105700-105799 债券ETF质押；105800-105899 可转债质押（对应110***、113***）；105900-105999 企业债质押（对应120***、129***）"),
    ("106", SecurityType.Bond, "地方政府债质押出入库", "对应130***"),
    ("107", SecurityType.Bond, "记账式贴现国债质押出入库", "对应020***"),
    ("108", SecurityType.Bond, "政策性银行债质押出入库", "对应018***"),
    ("109", SecurityType.Bond, "地方政府债", "地方政府债券"),

    ("110", SecurityType.Bond, "可转换公司债", "110000-110799 上市公司公开发行可转债（对应600***）；110800-110999 非公开发行"),
    ("111", SecurityType.Bond, "可转换公司债", "111000-111499 对应605***"),
    ("112", SecurityType.Bond, "资产支持证券", "资产支持证券"),
    ("113", SecurityType.Bond, "可转换公司债", "113000-113499 对应601***；113500-113999 对应603***"),
    ("114", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("115", SecurityType.Bond, "公开公司债", "公开发行公司债券"),
    ("118", SecurityType.Bond, "科创板可转债", "118000-118499 用于科创板上市公司公开发行可转债"),

    ("120", SecurityType.Bond, "企业/公司债", "122000-122499 用于公司债券；122500-122999 用于企业债券（见122）"),
    ("121", SecurityType.Bond, "资产支持证券", "资产支持证券"),
    ("122", SecurityType.Bond, "公司债/企业债", "122000-122499 用于公司债券；122500-122999 用于企业债券"),
    ("123", SecurityType.Bond, "公司/企业债/ABS", "123000-123499 用于企业/公司债；123500-123999 用于资产支持证券"),
    ("124", SecurityType.Bond, "企业债质押出入库", "对应124000-124999"),
    ("125", SecurityType.Bond, "中小企业私募债/非公开公司债", "中小企业私募债券、非公开发行公司债券"),
    ("126", SecurityType.Bond, "分离交易可转债", "分离交易的可转换公司债券"),
    ("127", SecurityType.Bond, "企业债", "127000-127899 用于企业债券；127900-127999 用于政府支持债（中国铁路建设债专用）"),
    ("128", SecurityType.Bond, "信贷资产支持证券", "信贷资产支持证券"),
    ("129", SecurityType.Bond, "企业债", "企业债券"),

    ("130", SecurityType.Bond, "地方政府债", "地方政府债券(对应130***)"),
    ("131", SecurityType.Bond, "资产支持证券", "资产支持证券"),
    ("132", SecurityType.Bond, "可交换公司债", "可交换公司债券"),
    ("133", SecurityType.Bond, "可交换债质押出入库", "对应132***"),
    ("134", SecurityType.Bond, "公开公司债质押出入库", "对应136***"),
    ("135", SecurityType.Bond, "证券公司短期债/并购私募债", "证券公司短期债、并购重组私募债券、非公开发行公司债券"),
    ("136", SecurityType.Bond, "公开公司债质押出入库", "对应136***"),
    ("137", SecurityType.Bond, "可交换/公开公司债", "137000-137499 非公开可交换；137500-137999 公开公司债"),
    ("138", SecurityType.Bond, "可交换换股/公开公司债", "138000-138499 非公开可交换换股(对应137000-137499)；138500-138999 公开公司债"),
    ("139", SecurityType.Bond, "企业债", "企业债券"),

    ("140", SecurityType.Bond, "地方政府债质押出入库", "对应140***"),
    ("141", SecurityType.Bond, "地方政府债", "地方政府债券"),
    ("142", SecurityType.Bond, "资产支持证券", "资产支持证券"),
    ("143", SecurityType.Bond, "公开公司债质押出入库", "对应143***"),
    ("144", SecurityType.Bond, "公开公司债", "公开发行公司债券"),
    ("145", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("146", SecurityType.Bond, "资产支持证券", "资产支持证券"),
    ("147", SecurityType.Bond, "地方政府债质押出入库", "对应147***"),
    ("148", SecurityType.Bond, "地方政府债", "地方政府债券"),
    ("149", SecurityType.Bond, "资产支持证券", "资产支持证券"),

    ("150", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("151", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("152", SecurityType.Bond, "企业债质押出入库", "对应152***"),
    ("153", SecurityType.Bond, "企业债", "企业债券"),
    ("154", SecurityType.Bond, "公司债质押出入库", "对应155***"),
    ("155", SecurityType.Bond, "公司债质押出入库", "对应155***"),
    ("156", SecurityType.Bond, "公司债", "公司债券"),
    ("157", SecurityType.Bond, "地方政府债质押出入库", "对应157***"),
    ("158", SecurityType.Bond, "地方政府债", "地方政府债券"),
    ("159", SecurityType.Bond, "资产支持证券", "资产支持证券"),

    ("160", SecurityType.Bond, "地方政府债", "地方政府债券"),
    ("161", SecurityType.Bond, "地方政府债质押出入库", "对应160***"),
    ("162", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("163", SecurityType.Bond, "公开公司债质押出入库", "对应163***"),
    ("164", SecurityType.Bond, "公开公司债", "公开发行公司债券"),
    ("165", SecurityType.Bond, "资产支持证券", "资产支持证券"),
    ("166", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("167", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("168", SecurityType.Bond, "资产支持证券", "资产支持证券"),
    ("169", SecurityType.Bond, "资产支持证券", "资产支持证券"),

    ("170", SecurityType.Bond, "信用保护工具", "170000-170499 用于信用保护凭证；170900-170999 用于组合型信用保护合约"),
    ("171", SecurityType.Bond, "地方政府债质押出入库", "对应171***"),
    ("172", SecurityType.Bond, "地方政府债", "地方政府债券"),
    ("173", SecurityType.Bond, "地方政府债质押出入库", "对应173***"),
    ("174", SecurityType.Bond, "地方政府债", "地方政府债券"),
    ("175", SecurityType.Bond, "公开公司债质押出入库", "对应175***"),
    ("176", SecurityType.Bond, "公开公司债", "公开发行公司债券"),
    ("177", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("178", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("179", SecurityType.Bond, "资产支持证券", "资产支持证券"),

    ("180", SecurityType.Bond, "资产支持证券", "资产支持证券"),
    ("181", SecurityType.Bond, "可转债转股/非公开公司债", "对应600*** 的转股等/182000 系列为回售或非公开"),
    ("182", SecurityType.Bond, "债券回售/非公开公司债", "182000-182299 用于债券回售；182300-182999 用于非公开发行公司债券"),
    ("183", SecurityType.Bond, "资产支持证券", "资产支持证券"),
    ("184", SecurityType.Bond, "企业债/政府支持债", "184000-184799 企业债券；184800-184999 政府支持债（中国铁路建设债专用）"),
    ("185", SecurityType.Bond, "公开公司债", "公开发行公司债券"),
    ("186", SecurityType.Bond, "地方政府债", "地方政府债券"),
    ("187", SecurityType.Bond, "公开公司债质押出入库", "对应188***"),
    ("188", SecurityType.Bond, "公开公司债质押出入库", "对应188***"),
    ("189", SecurityType.Bond, "资产支持证券", "资产支持证券"),

    ("190", SecurityType.Bond, "可转债转股", "对应600***"),
    ("191", SecurityType.Bond, "可转债转股", "191000-191499 对应601***；191500-191999 对应603***"),
    ("192", SecurityType.Bond, "可交换债换股", "对应132***"),
    ("193", SecurityType.Bond, "创新创业转股/ABS", "193000-193099 创新创业公司非公开可转债转股（对应145900-145999）；193100-193999 用于资产支持证券"),
    ("194", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("195", SecurityType.Bond, "可转债转股", "195000-195499 用于可转债转股，对应605***"),
    ("196", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("197", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("198", SecurityType.Bond, "地方政府债", "地方政府债券"),
    ("199", SecurityType.Bond, "资产支持证券", "资产支持证券"),

    ("201", SecurityType.Bond, "国债回购", "国债回购（席位托管方式）"),
    ("202", SecurityType.Bond, "企业债回购", "企业债回购（席位托管方式）"),
    ("203", SecurityType.Bond, "国债买断式回购", "国债买断式回购"),
    ("204", SecurityType.Bond, "债券质押式回购(账户托管)", "债券质押式回购（账户托管方式）"),
    ("205", SecurityType.Bond, "质押式报价回购", "质押式报价回购"),
    ("206", SecurityType.Bond, "质押式协议回购", "债券质押式协议回购"),
    ("207", SecurityType.Bond, "质押式三方回购", "债券质押式三方回购"),
    ("208", SecurityType.Bond, "债券借贷", "208000-208009 用于债券借贷业务"),

    ("230", SecurityType.Bond, "地方政府债", "地方政府债券"),
    ("231", SecurityType.Bond, "地方政府债", "地方政府债券"),
    ("232", SecurityType.Bond, "地方政府债", "地方政府债券"),
    ("233", SecurityType.Bond, "地方政府债", "地方政府债券"),

    ("240", SecurityType.Bond, "公开公司债", "公开发行公司债券"),
    ("241", SecurityType.Bond, "公开公司债", "公开发行公司债券"),

    ("250", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("251", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("252", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("253", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("254", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("255", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("256", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),
    ("257", SecurityType.Bond, "非公开公司债", "非公开发行公司债券"),

    ("260", SecurityType.Bond, "资产支持证券", "资产支持证券"),
    ("261", SecurityType.Bond, "资产支持证券", "资产支持证券"),
    ("262", SecurityType.Bond, "资产支持证券", "资产支持证券"),
    ("263", SecurityType.Bond, "资产支持证券", "资产支持证券"),

    ("270", SecurityType.Bond, "企业债", "企业债券"),
    ("271", SecurityType.Bond, "企业债", "企业债券"),
    ("272", SecurityType.Bond, "企业债", "企业债券"),

    ("310", SecurityType.Bond, "国债期货", "国债期货（已暂停）"),
    ("330", SecurityType.IPO, "优先股(公开)", "公开发行优先股"),
    ("360", SecurityType.Other, "非公开优先股", "非公开发行优先股"),

    ("500", SecurityType.Fund, "封闭式基金", "契约型封闭式基金"),
    ("501", SecurityType.Fund, "上市开放式基金", "上市开放式基金"),
    ("502", SecurityType.Fund, "上市开放式基金", "上市开放式基金"),
    ("505", SecurityType.Fund, "创新封闭式基金", "505800-505899 用于创新型封闭式证券投资基金"),
    ("506", SecurityType.Fund, "科创板LOF", "506000-506099 用于科创板相关 LOF"),
    ("508", SecurityType.Fund, "公募REITs", "508000-508099 用于公募 REITs"),
    ("510", SecurityType.ETF, "交易型开放式指数证券投资基金", "标的指数为沪市指数、跨市场指数或跨境指数"),
    ("511", SecurityType.ETF, "债券ETF/货基", "511000-511299 单市场债券（沪）ETF；511300-511599 现金申赎类债券ETF；511600-511999 交易型货币基金"),
    ("512", SecurityType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    ("513", SecurityType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨境指数"),
    ("515", SecurityType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    ("516", SecurityType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    ("517", SecurityType.ETF, "跨市场股票ETF", "517000-517999 用于跨市场股票（沪港深京）ETF"),
    ("518", SecurityType.ETF, "商品交易型开放式证券投资基金", "商品类 ETF"),
    ("519", SecurityType.Fund, "开放式基金申赎/认购", "519*** 系列用于开放式基金的申赎/认购/跨市场转托管/分红/转换等；5198** 用于实时申赎货币基金（实时申赎）"),
    ("520", SecurityType.ETF, "跨境ETF", "520500-520999 用于跨境 ETF"),
    ("521", SecurityType.Fund, "开放式基金认购", "对应519*** 系列的认购业务"),
    ("522", SecurityType.Fund, "开放式基金跨市场转托管", "对应519*** 系列的跨市场转托管业务"),
    ("523", SecurityType.Fund, "开放式基金分红", "对应519*** 系列的分红业务"),
    ("524", SecurityType.Fund, "开放式基金基金转换", "对应519*** 系列的基金转换业务"),
    ("530", SecurityType.ETF, "交易型开放式指数证券投资基金", "标的指数为沪市指数"),
    ("550", SecurityType.Fund, "基金", ""),
    ("560", SecurityType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    ("561", SecurityType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    ("562", SecurityType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    ("563", SecurityType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    ("580", SecurityType.Warrant, "权证", "含股改权证、公司权证"),
    ("582", SecurityType.Warrant, "权证行权", "用于权证行权/行权相关代码"),
    ("588", SecurityType.ETF, "科创板ETF", "588000-588299 单市场（科创板）ETF；588300-588699 跨市场（含科创板）ETF；588700-588999 单市场（科创板）ETF"),
    ("5", SecurityType.ETF, "基金/REITs/权证", "首位 5：基金、公募 REITs、权证"),

    ("600", SecurityType.Stock, "主板A股", "主板 A 股"),
    ("601", SecurityType.Stock, "主板A股", "主板 A 股"),
    ("603", SecurityType.Stock, "主板A股", "主板 A 股"),
    ("605", SecurityType.Stock, "主板A股", "主板 A 股（配套号段）"),
    ("688", SecurityType.Stock, "科创板", "科创板股票"),
    ("689", SecurityType.Stock, "科创板存托凭证", "科创板存托凭证"),

    ("700", SecurityType.Other, "配股", "配股（对应600***）"),
    ("701", SecurityType.Other, "转配股", "转配股"),
    ("702", SecurityType.Other, "职工股配股", "对应600***"),
    ("703", SecurityType.Other, "配售", "配售"),
    ("704", SecurityType.Other, "可转债配债", "可转换公司债券持股配债（对应600***）"),
    ("706", SecurityType.Other, "要约收购/现金选择权", "706000-706599 主板；706600-706999 科创板"),
    ("707", SecurityType.Other, "网上按市值申购/增发", "对应605***"),
    ("708", SecurityType.Other, "网上按市值申购配号", "对应605***"),
    ("713", SecurityType.Other, "可转债申购", "对应605***"),
    ("714", SecurityType.Other, "可转债申购配号", "对应605***"),
    ("715", SecurityType.Other, "可转债持股配债", "对应605***"),
    ("718", SecurityType.Other, "科创板可转债申购", "对应118000-118499"),
    ("726", SecurityType.Other, "科创板可转债配债", "对应118000-118499"),
    ("730", SecurityType.IPO, "新股申购", "新股申购/网上申购"),
    ("758", SecurityType.Other, "可交换债配号", "758000-758099"),
    ("759", SecurityType.Other, "可交换债申购", "759000-759099"),
    ("786", SecurityType.Other, "科创板配售/存托配售", "786000-786899 科创板股票配售；786900-786999 科创板存托凭证配售"),
    ("799", SecurityType.Other, "特殊业务代码", "指定交易/融资融券/网络投票/资金前端控制/身份认证等（见799xxx 具体编码）"),

    ("880", SecurityType.Block, "板块指数", "通达信"),
    ("881", SecurityType.Block, "板块指数", "通达信"),
    ("888", SecurityType.Bond, "标准券", "888880 为新标准券，用于债券回购转换成标准券"),

    ("900", SecurityType.BStock, "B股", "B 股"),
    ("901", SecurityType.BStock, "B转H", "901000-901099 用于 B 转 H"),
    ("938", SecurityType.Other, "网络投票", "对应 B 股（不再增用）"),
    ("939", SecurityType.Other, "密码服务", "939988 用于 B 股网络投票密码服务"),

    ("0", SecurityType.Index, "指数/国债", "首位 0：指数、国债"),
    ("1", SecurityType.Bond, "债券现券", "首位 1：债券现券"),
    ("2", SecurityType.Bond, "债券回购/借贷", "首位 2：债券回购、债券借贷等"),
    ("3", SecurityType.Other, "优先股/国债期货", "首位 3：优先股、国债期货（已暂停）"),
    ("4", SecurityType.Other, "备用", "首位 4：备用"),
    ("5", SecurityType.Fund, "基金/REITs/权证", "首位 5：基金、公募 REITs、权证"),
    ("6", SecurityType.Stock, "A股/存托凭证", "首位 6：A 股、存托凭证"),
    ("7", SecurityType.Other, "非交易业务", "首位 7：非交易业务"),
    ("8", SecurityType.Bond, "标准券/备用", "首位 8：标准券、备用"),
    ("9", SecurityType.BStock, "B股", "首位 9：B 股"),
]

szse_rules = [
    ("395", SecurityType.Index, "成交量统计指数", ""),
    ("399", SecurityType.Index, "深证指数", ""),
    ("000", SecurityType.Stock, "主板A股", ""),
    ("001", SecurityType.Stock, "主板A股", ""),
    ("002", SecurityType.Stock, "主板A股", ""),
    ("003", SecurityType.Stock, "主板A股", ""),
    ("030", SecurityType.Warrant, "权证", ""),
    ("031", SecurityType.Warrant, "权证", ""),
    ("032", SecurityType.Warrant, "权证", ""),
    ("036", SecurityType.Warrant, "创业板股权激励计划涉及的员工认股权", ""),
    ("0370", SecurityType.Warrant, "主板A股股权激励计划涉及的员工认股权", ""),
    ("0371", SecurityType.Warrant, "主板A股股权激励计划涉及的员工认股权", ""),
    ("0372", SecurityType.Warrant, "创业板股权激励计划审计的员工认股权", ""),
    ("0373", SecurityType.Warrant, "主板A股股权激励计划涉及的员工认股权", ""),
    ("0374", SecurityType.Warrant, "主板A股股权激励计划涉及的员工认股权", ""),
    ("0375", SecurityType.Warrant, "中小企业板股权激励计划涉及的员工认股权", ""),
    ("0376", SecurityType.Warrant, "中小企业板股权激励计划涉及的员工认股权", ""),
    ("0377", SecurityType.Warrant, "中小企业板股权激励计划涉及的员工认股权", ""),
    ("0378", SecurityType.Warrant, "中小企业板股权激励计划涉及的员工认股权", ""),
    ("0379", SecurityType.Warrant, "中小企业板股权激励计划涉及的员工认股权", ""),
    ("038", SecurityType.Warrant, "主板A股及中小企业股票认沽权证", ""),
    ("039", SecurityType.Warrant, "主板A股及中小企业股票认沽权证", ""),
    ("070", SecurityType.Warrant, "主板A股增发/可转债申购", ""),
    ("071", SecurityType.Warrant, "主板A股增发/可转债申购", ""),
    ("072", SecurityType.Warrant, "中小企业板增发/可转债申购", ""),
    ("073", SecurityType.Warrant, "中小企业板增发/可转债申购", ""),
    ("074", SecurityType.Warrant, "中小企业板增发/可转债申购", ""),
    ("080", SecurityType.Warrant, "A股配股", ""),
    ("0", SecurityType.Stock, "股票", ""),
    ("10", SecurityType.Bond, "国债", ""),
    ("11", SecurityType.Bond, "企业债", ""),
    ("120", SecurityType.Bond, "企业债券", ""),
    ("123", SecurityType.Bond, "可转债", ""),
    ("127", SecurityType.Bond, "可转债", ""),
    ("128", SecurityType.Bond, "可转债", ""),
    ("13", SecurityType.Bond, "债券回购", ""),
    ("159", SecurityType.ETF, "深交所ETF", ""),
    ("15", SecurityType.Fund, "ETF", ""),
    ("16", SecurityType.Fund, "LOF", ""),
    ("17", SecurityType.Fund, "传统投资基金", ""),
    ("184", SecurityType.Fund, "封闭式基金", ""),
    ("18", SecurityType.Fund, "封闭式基金", ""),
    ("1", SecurityType.Bond, "债券", ""),
    ("200", SecurityType.BStock, "B股", ""),
    ("238", SecurityType.Other, "B股现金选择权", ""),
    ("28", SecurityType.Other, "B股配股优先权", ""),
    ("2", SecurityType.BStock, "B股", ""),
    ("300", SecurityType.Stock, "创业板", ""),
    ("301", SecurityType.Stock, "创业板注册制", ""),
    ("30", SecurityType.Stock, "创业板", ""),
    ("36", SecurityType.Other, "投票", ""),
    ("37", SecurityType.Other, "增发/可转债申购", ""),
    ("38", SecurityType.Other, "配股/可转债优先权", ""),
    #("50", SecurityType.Bond, "资产支持证券ABS", ""),
    #("56", SecurityType.Bond, "资产支持证券ABS", ""),
    #("5", SecurityType.Bond, "资产支持证券ABS", ""),
    #("700", SecurityType.Warrant, "B股增发", ""),
    #("730", SecurityType.Warrant, "跨市场申购", ""),
]

bjse_rules = [
    ("899", SecurityType.Index, "指数", "证券指数首三位代码为899"),
    ("920", SecurityType.Stock, "北交所新上市", "2024-04-22 起新上市使用920号段；已上市公司继续沿用原代码直到统一切换"),
    ("92", SecurityType.Stock, "上市公司普通股", "首两位92：上市公司普通股票；920号段自2024-04-22起用于新上市公司"),
    ("400", SecurityType.Stock, "两网/退市A股", "两网公司及退市公司A股首三位代码为400"),
    ("420", SecurityType.BStock, "退市B股", "退市公司B股首三位代码为420"),
    ("810", SecurityType.Bond, "可转换公司债", "向特定对象发行的可转换公司债券首三位代码为810"),
    ("81", SecurityType.Bond, "优先股(极少)", "其他极少数代码"),
    ("820", SecurityType.Bond, "优先股", "优先股票首三位代码为820"),
    ("821", SecurityType.Bond, "优先股", "优先股票首三位代码为820"),
    ("82", SecurityType.Bond, "优先股(极少)", "其他极少数代码"),
    ("83", SecurityType.Stock, "挂牌公司普通股", "挂牌公司普通股票首两位为83"),
    ("840", SecurityType.Other, "要约收购", "要约收购证券代码首三位代码为84"),
    ("841", SecurityType.Other, "要约回购", "要约回购证券代码首三位代码为841"),
    ("87", SecurityType.Stock, "挂牌公司普通股", "挂牌公司普通股票首两位为87"),
    ("88", SecurityType.Stock, "挂牌公司普通股", "挂牌公司普通股票首两位为88"),
    ("850", SecurityType.Option, "股权激励期权", "股权激励期权首三位代码为850"),
]

hkse_rules = [
    ("HSI", SecurityType.Index, "恒生指数", ""),
    ("HSCEI", SecurityType.Index, "国企指数", ""),
    ("HSCCI", SecurityType.Index, "红筹指数", ""),
    ("028", SecurityType.ETF, "ETF", ""),
    ("030", SecurityType.ETF, "ETF", ""),
    ("031", SecurityType.ETF, "ETF", ""),
    ("090", SecurityType.ETF, "ETF", ""),
    ("091", SecurityType.ETF, "ETF", ""),
    ("08", SecurityType.Stock, "港股", "GEM"),
    ("0", SecurityType.Stock, "港股", ""),
    ("1", SecurityType.Bond, "权证", ""),
    ("2", SecurityType.Bond, "权证", ""),
    ("4", SecurityType.Bond, "牛熊证", ""),
    ("5", SecurityType.Bond, "牛熊证", ""),
    ("6", SecurityType.Bond, "牛熊证", ""),
]


def match_rule(code: str, rules):
    """Return (SecurityType, desc) for the longest matching prefix, or (Unknown, "")"""
    best_len = 0
    matched = SecurityType.Unknown
    desc = ""
    for entry in rules:
        if not entry:
            continue
        prefix = entry[0]
        typ = entry[1]
        d = entry[2] if len(entry) > 2 else ""
        if code.startswith(prefix) and len(prefix) > best_len:
            best_len = len(prefix)
            matched = typ
            desc = d
    return (matched, desc) if best_len > 0 else (SecurityType.Unknown, "")


def detect(input_str: str) -> SecurityCode:
    s = (input_str or "").strip()
    if not s:
        return SecurityCode(ExchangeId.ShangHai, "", SecurityType.Unknown)
    s = s.lower()
    pure_code = s

    symbol = ""
    exchange_id = ExchangeId.Unknown
    typ = SecurityType.Unknown

    # 1. explicit market flag prefix: sh600000
    if len(pure_code) >= 2 and pure_code[:2] in ALL_EXCHANGE_CODES:
        symbol = pure_code[2:]
        exchange_id = ALL_EXCHANGE_CODES[pure_code[:2]]
    # suffix form: 600000.sh or APPL.us
    elif len(pure_code) >= 3 and pure_code[-3] == '.' and pure_code[-2:] in ALL_EXCHANGE_CODES:
        symbol = pure_code[:-3]
        exchange_id = ALL_EXCHANGE_CODES[pure_code[-2:]]

    # 2. infer market if not set
    if exchange_id == ExchangeId.Unknown:
        if re.fullmatch(r"\d{6}", pure_code):
            symbol = pure_code
            if pure_code.startswith("6") or pure_code.startswith("5") or pure_code.startswith("9") or pure_code.startswith("7") or pure_code.startswith("000"):
                exchange_id = ExchangeId.ShangHai
            elif pure_code.startswith("0") or pure_code.startswith("3") or pure_code.startswith("1") or pure_code.startswith("2"):
                exchange_id = ExchangeId.ShenZhen
            elif pure_code.startswith("8") or pure_code.startswith("92"):
                exchange_id = ExchangeId.BeiJing
            else:
                return SecurityCode(ExchangeId.Unknown, "", SecurityType.Unknown)
        elif re.fullmatch(r"\d{5}", pure_code):
            symbol = pure_code
            exchange_id = ExchangeId.HongKong
        else:
            symbol = pure_code
    elif not symbol:
        symbol = pure_code

    # 4-letter all-alpha -> US stock (pure form)
    if exchange_id == ExchangeId.Unknown and len(symbol) == 4 and symbol.isalpha():
        exchange_id = ExchangeId.USA
        typ = SecurityType.Stock

    # 4. global rules priority
    if re.fullmatch(r"\d{6}", symbol):
        typ_, _ = match_rule(symbol, global_rules)
        if typ_ != SecurityType.Unknown:
            # global rules belong to SSE
            return SecurityCode(ExchangeId.ShangHai, symbol, typ_)

    # 5. match market rules
    if exchange_id == ExchangeId.Unknown:
        return SecurityCode(ExchangeId.Unknown, "", SecurityType.Unknown)

    if typ == SecurityType.Unknown:
        rules = None
        if exchange_id == ExchangeId.ShangHai:
            rules = sse_rules
        elif exchange_id == ExchangeId.ShenZhen:
            rules = szse_rules
        elif exchange_id == ExchangeId.BeiJing:
            rules = bjse_rules
        elif exchange_id == ExchangeId.HongKong:
            rules = hkse_rules
        elif exchange_id == ExchangeId.USA:
            return SecurityCode(exchange_id, symbol, SecurityType.Stock)
        else:
            return SecurityCode(ExchangeId.Unknown, "", SecurityType.Unknown)

        typ_, _ = match_rule(symbol, rules)
        if typ_ != SecurityType.Unknown:
            return SecurityCode(exchange_id, symbol, typ_)
        else:
            return SecurityCode(ExchangeId.Unknown, "", SecurityType.Unknown)
    else:
        return SecurityCode(exchange_id, symbol, typ)


__all__ = ["detect", "SecurityCode", "ExchangeId", "SecurityType"]
