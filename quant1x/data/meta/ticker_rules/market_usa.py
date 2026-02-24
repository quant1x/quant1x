from quant1x.std.numeric import NumberRange
from .rule import CodeRule
from ..exchange import Exchange
from ..instrument import InstrumentType

# USA 美国证券交易所规则
usa_rules = [
    CodeRule(Exchange.OFFSHORE, "A_IXIC", InstrumentType.INDEX, "指数", "纳斯达克指数"),
    CodeRule(Exchange.OFFSHORE, "", InstrumentType.STOCK, "挂牌公司普通股", ""),
]

