# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from enum import IntEnum

# 1. 申赎机制(最宏观), 2bit -> 高位 [15:14]
class FundOpenness(IntEnum):
    OPEN        = 0   # 开放式
    CLOSED      = 1   # 封闭式
    INTERVAL    = 2   # 定期开放(可选)
    
    
# 2. 投资策略, 1bit -> [13]
class FundStrategy(IntEnum):
    ACTIVE      = 0   # 主动管理
    PASSIVE     = 1   # 被动(指数跟踪)

# 3. 发行类型, 1bit -> [12]
class OfferingType(IntEnum):
    PUBLIC  = 0   # 公募
    PRIVATE = 1   # 私募

# 4. 基金结构, 2bit -> [11:10]
class FundStructure(IntEnum):
    DIRECT      = 0 # 直接投资型, 基金直接买入股票, 债券, 存款等底层资产
    FOF         = 1 # 基金中基金(Fund of Funds),基金不直接买股票/债券, 而是投资于其他公募基金. 例如: “南方全天候策略 FOF”, 其持仓是多个子基金. 风险分散, 但有双重收费. 
    MOM         = 2 # 管理人中管理人(Manager of Managers), 基金将资金分配给多个外部投顾或子管理人, 由他们分别管理子账户

# 5. 投资范围, 2bit -> [9:8]
class FundInvestmentScope(IntEnum):
    DOMESTIC    = 0 # 境内投资
    QDII        = 1 # 合格境内机构投资者(Qualified Domestic Institutional Investor)
    QDLP        = 2 # 合格境内有限合伙人(Qualified Domestic Limited Partner)

# 6. 交易机制, 3bit -> [7:5]
class FundTradingMechanism(IntEnum):
    OTC_ONLY      = 0   # 仅场外(普通基金)
    LISTED        = 1   # 上市交易(LOF, 封闭式等)
    TRADABLE      = 2   # 交易型(ETF, 支持实物申赎+IOPV)
    
# 7. 资产类别(最微观), 5bit -> 低位 [4:0]
class FundAssetClass(IntEnum):
    UNKNOWN     = 0 # 未知
    MONEY       = 1 # 货币
    BOND        = 2 # 债券
    STOCK       = 3 # 股票
    MIXED       = 4 # 混合
    COMMODITY   = 5 # 商品
    REITs       = 6 # 不动产投资信托基金
    OTHER       = 7 # 原 255 改为 7, 因只用 4 位


def encode_fund_type(
    openness=FundOpenness.OPEN,
    strategy=FundStrategy.ACTIVE,
    offering=OfferingType.PUBLIC,
    structure=FundStructure.DIRECT,
    scope=FundInvestmentScope.DOMESTIC,
    trading=FundTradingMechanism.OTC_ONLY,
    asset=FundAssetClass.STOCK
) -> int:
    return (
        ((int(openness) & 0x3) << 14) |
        ((int(strategy) & 0x1) << 13) |
        ((int(offering) & 0x1) << 12) |
        ((int(structure) & 0x3) << 10) |
        ((int(scope) & 0x3) << 8) |
        ((int(trading) & 0x7) << 5) |
        (int(asset) & 0x1F)
    )

def decode_fund_type(code: int) -> dict:
    return {
        "openness": FundOpenness((code >> 14) & 0x3),
        "strategy": FundStrategy((code >> 13) & 0x1),
        "offering": OfferingType((code >> 12) & 0x1),
        "structure": FundStructure((code >> 10) & 0x3),
        "scope": FundInvestmentScope((code >> 8) & 0x3),
        "trading": FundTradingMechanism((code >> 5) & 0x7),
        "asset": FundAssetClass(code & 0x1F),
    }