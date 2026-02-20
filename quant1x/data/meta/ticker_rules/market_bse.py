from quant1x.std.numeric import NumberRange
from .rule import CodeRule
from ..exchange import Exchange
from ..instrument import InstrumentType

# BSE 北京证券交易所规则
bse_rules = [
    CodeRule(Exchange.BSE, "899", InstrumentType.INDEX, "指数", "证券指数首三位代码为899"),
    
    CodeRule(Exchange.BSE, "920", InstrumentType.STOCK, "北交所新上市", "2024-04-22 起新上市使用920号段; 已上市公司继续沿用原代码直到统一切换"),
    CodeRule(Exchange.BSE, "92", InstrumentType.STOCK, "上市公司普通股", "首两位92: 上市公司普通股票; 920号段自2024-04-22起用于新上市公司"),
    
    CodeRule(Exchange.BSE, "400", InstrumentType.STOCK, "两网/退市A股", "两网公司及退市公司A股首三位代码为400"),
    CodeRule(Exchange.BSE, "420", InstrumentType.BSTOCK, "退市B股", "退市公司B股首三位代码为420"),
    
    CodeRule(Exchange.BSE, "810", InstrumentType.BOND, "可转换公司债", "向特定对象发行的可转换公司债券首三位代码为810"),
    CodeRule(Exchange.BSE, "81", InstrumentType.BOND, "优先股(极少)", "其他极少数代码"),
    CodeRule(Exchange.BSE, "820", InstrumentType.BOND, "优先股", "优先股票首三位代码为820"),
    CodeRule(Exchange.BSE, "821", InstrumentType.BOND, "优先股", "优先股票首三位代码为820"),
    CodeRule(Exchange.BSE, "82", InstrumentType.BOND, "优先股(极少)", "其他极少数代码"),
    CodeRule(Exchange.BSE, "83", InstrumentType.STOCK, "挂牌公司普通股", "挂牌公司普通股票首两位为83"),
    CodeRule(Exchange.BSE, "840", InstrumentType.OTHER, "要约收购", "要约收购证券代码首三位代码为84"),
    CodeRule(Exchange.BSE, "841", InstrumentType.OTHER, "要约回购", "要约回购证券代码首三位代码为841"),
    CodeRule(Exchange.BSE, "87", InstrumentType.STOCK, "挂牌公司普通股", "挂牌公司普通股票首两位为87"),
    CodeRule(Exchange.BSE, "88", InstrumentType.STOCK, "挂牌公司普通股", "挂牌公司普通股票首两位为88"),
    CodeRule(Exchange.BSE, "850", InstrumentType.OPTION, "股权激励期权", "股权激励期权首三位代码为850"),
]

