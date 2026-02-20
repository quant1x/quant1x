from quant1x.std.numeric import NumberRange
from .rule import CodeRule
from ..exchange import Exchange
from ..instrument import InstrumentType

# SZSE 深圳证券交易所规则
szse_rules = [
    CodeRule(Exchange.SZSE, "395", InstrumentType.INDEX, "成交量统计指数", ""),
    CodeRule(Exchange.SZSE, "399", InstrumentType.INDEX, "深证指数", ""),
    
    CodeRule(Exchange.SZSE, "000", InstrumentType.STOCK, "主板A股", ""),
    CodeRule(Exchange.SZSE, "001", InstrumentType.STOCK, "主板A股", ""),
    CodeRule(Exchange.SZSE, "002", InstrumentType.STOCK, "主板A股", ""),
    CodeRule(Exchange.SZSE, "003", InstrumentType.STOCK, "主板A股", ""),
    CodeRule(Exchange.SZSE, "030", InstrumentType.WARRANT, "权证", ""),
    CodeRule(Exchange.SZSE, "031", InstrumentType.WARRANT, "权证", ""),
    CodeRule(Exchange.SZSE, "032", InstrumentType.WARRANT, "权证", ""),
    CodeRule(Exchange.SZSE, "036", InstrumentType.WARRANT, "创业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0370", InstrumentType.WARRANT, "主板A股股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0371", InstrumentType.WARRANT, "主板A股股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0372", InstrumentType.WARRANT, "创业板股权激励计划审计的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0373", InstrumentType.WARRANT, "主板A股股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0374", InstrumentType.WARRANT, "主板A股股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0375", InstrumentType.WARRANT, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0376", InstrumentType.WARRANT, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0377", InstrumentType.WARRANT, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0378", InstrumentType.WARRANT, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0379", InstrumentType.WARRANT, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "038", InstrumentType.WARRANT, "主板A股及中小企业股票认沽权证", ""),
    CodeRule(Exchange.SZSE, "039", InstrumentType.WARRANT, "主板A股及中小企业股票认沽权证", ""),
    CodeRule(Exchange.SZSE, "070", InstrumentType.WARRANT, "主板A股增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "071", InstrumentType.WARRANT, "主板A股增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "072", InstrumentType.WARRANT, "中小企业板增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "073", InstrumentType.WARRANT, "中小企业板增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "074", InstrumentType.WARRANT, "中小企业板增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "080", InstrumentType.WARRANT, "A股配股", ""),
    
    CodeRule(Exchange.SZSE, "0", InstrumentType.STOCK, "股票", ""),
    
    CodeRule(Exchange.SZSE, "10", InstrumentType.BOND, "国债", ""),
    CodeRule(Exchange.SZSE, "11", InstrumentType.BOND, "企业债", ""),
    CodeRule(Exchange.SZSE, "120", InstrumentType.BOND, "企业债券", ""),
    CodeRule(Exchange.SZSE, "123", InstrumentType.BOND, "可转债", ""),
    CodeRule(Exchange.SZSE, "127", InstrumentType.BOND, "可转债", ""),
    CodeRule(Exchange.SZSE, "128", InstrumentType.BOND, "可转债", ""),
    CodeRule(Exchange.SZSE, "13", InstrumentType.BOND, "债券回购", ""),
    CodeRule(Exchange.SZSE, "159", InstrumentType.ETF, "深交所ETF", ""),
    CodeRule(Exchange.SZSE, "15", InstrumentType.FUND, "ETF", ""),
    CodeRule(Exchange.SZSE, "16", InstrumentType.FUND, "LOF", ""),
    CodeRule(Exchange.SZSE, "17", InstrumentType.FUND, "传统投资基金", ""),
    CodeRule(Exchange.SZSE, "184", InstrumentType.FUND, "封闭式基金", ""),
    CodeRule(Exchange.SZSE, "18", InstrumentType.FUND, "封闭式基金", ""),
    
    CodeRule(Exchange.SZSE, "1", InstrumentType.BOND, "债券", ""),
    
    CodeRule(Exchange.SZSE, "200", InstrumentType.BSTOCK, "B股", ""),
    CodeRule(Exchange.SZSE, "238", InstrumentType.OTHER, "B股现金选择权", ""),
    CodeRule(Exchange.SZSE, "28", InstrumentType.OTHER, "B股配股优先权", ""),
    
    CodeRule(Exchange.SZSE, "2", InstrumentType.BSTOCK, "B股", ""),
    
    CodeRule(Exchange.SZSE, "300", InstrumentType.STOCK, "创业板", ""),
    CodeRule(Exchange.SZSE, "301", InstrumentType.STOCK, "创业板注册制", ""),
    CodeRule(Exchange.SZSE, "30", InstrumentType.STOCK, "创业板", ""),
    CodeRule(Exchange.SZSE, "36", InstrumentType.OTHER, "投票", ""),
    CodeRule(Exchange.SZSE, "37", InstrumentType.OTHER, "增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "38", InstrumentType.OTHER, "配股/可转债优先权", ""),
    
    CodeRule(Exchange.SZSE, "50", InstrumentType.BOND, "资产支持证券ABS", ""),
    CodeRule(Exchange.SZSE, "56", InstrumentType.BOND, "资产支持证券ABS", ""),
    
    CodeRule(Exchange.SZSE, "5", InstrumentType.BOND, "资产支持证券ABS", ""),
    
    CodeRule(Exchange.SZSE, "700", InstrumentType.WARRANT, "B股增发", ""),
    CodeRule(Exchange.SZSE, "730", InstrumentType.WARRANT, "跨市场申购", ""),
]

