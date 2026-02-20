from dataclasses import dataclass
from quant1x.std.numeric import NumberRange
from ..exchange import Exchange
from ..instrument import InstrumentType


@dataclass
class CodeRule:
    """证券代码规则"""
    exchange: Exchange       # 交易所
    prefix: str |NumberRange # 代码前缀
    type: InstrumentType     # 证券类型
    name: str                # 证券类型名称
    desc: str                # 规则描述

# 全局规则
global_rules = [
    CodeRule(Exchange.SSE, "880", InstrumentType.SECTOR, "板块指数", "通达信"),
    CodeRule(Exchange.SSE, "881", InstrumentType.SECTOR, "板块指数", "通达信"),
]
