# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from enum import Enum, auto
from re import A
from typing import Set

class AssetClass(Enum):
    """
    证券资产类别标准枚举
    设计原则: 
    1. 与通达信 category 字段精确映射
    2. 符合 ISO 15022/FIX Protocol 国际标准
    3. 支持中国特有资产类型(如股转系统)
    4. 区分交易资产与非交易资产(指标/指数)
    """
    
    # === 权益类 (Equity) ===
    EQUITY = "EQUITY"           # 普通股票(A股/港股/美股等)
    """股票"""
    EQUITY_PREFERRED = "PREFERRED"     # 优先股
    """优先股"""
    EQUITY_DEPOSITORY_RECEIPT = "DEPOSITORY_RECEIPT"  # 存托凭证(CDR/ADR)
    """存托凭证"""
    EQUITY_GEM = "GEN" # 创业板
    """创业板"""
    EQUITY_CONNECT = "CONNECT"
    """跨市场合约"""
    
    # === 固定收益类 (Fixed Income) ===
    BOND = "BOND"               # 债券(国债/企业债/可转债)
    """债券"""
    TREASURY = "TREASURY"       # 国债(细分)
    """国债"""
    CONVERTIBLE_BOND = "CONVERTIBLE_BOND"  # 可转债
    """可转债"""
    
    # === 基金类 (Fund) ===
    MUTUAL_FUND = "MUTUAL_FUND" # 公募基金(含股票/债券/混合型)
    """公募基金"""
    MONEY_MARKET_FUND = "MONEY_MARKET_FUND"  # 货币基金
    """货币基金"""
    ETF = "ETF"                 # 交易所交易基金
    """交易所交易基金"""
    LOF = "LOF"                 # 上市开放式基金
    """上市开放式基金"""
    PRIVATE_EQUITY = "PRIVATE_EQUITY"  # 私募基金(阳光私募/券商资管)
    """私募基金"""
    
    # === 衍生品类 (Derivatives) ===
    FUTURE = "FUTURE"           # 期货(商品/金融)
    """期货"""
    OPTION = "OPTION"           # 期权(商品/股票/股指)
    """期权"""
    WARRANT = "WARRANT"         # 权证(港股特色)
    """权证"""
    SWAP = "SWAP"               # 互换
    """互换"""
    
    # === 商品与外汇 (Commodities & FX) ===
    COMMODITY = "COMMODITY"     # 现货商品(黄金/白银等)
    FX_SPOT = "FX_SPOT"         # 外汇即期(基本/交叉汇率)
    FX_FORWARD = "FX_FORWARD"   # 外汇远期
    
    # === 指数类 (Index) - 非交易资产 ===
    INDEX = "INDEX"             # 价格指数(中证/国证/国际指数)
    BENCHMARK = "BENCHMARK"     # 基准指数(静态成分股列表)
    
    # === 另类资产 (Alternative) ===
    OTC = "OTC"                 # 场外交易品种(协议转让)
    REPO = "REPO"               # 回购
    
    # === 非交易资产 (Non-Tradable) ===
    MACRO_INDICATOR = "MACRO_INDICATOR"  # 宏观经济指标
    STATISTICS = "STATISTICS"   # 统计数据(如大宗连续)
    
    # === 中国特有类别 ===
    NEEQ = "NEEQ"             # 新三板/股转系统(National Equities Exchange and Quotations)
    BOND_PRE_ISSUE = "BOND_PRE_ISSUE"  # 国债预发行
    
    # === 虚拟合约 (Synthetic) ===
    SYNTHETIC = "SYNTHETIC"     # 主力合约/连续合约(系统合成)
    
    # === 其他类别 ===
    ALL = "ALL"             # 全部, 所有资产类别, 非特殊需要不适用
    
    @property
    def is_tradable(self) -> bool:
        """是否可交易资产"""
        non_tradable = {
            AssetClass.INDEX,
            AssetClass.BENCHMARK,
            AssetClass.MACRO_INDICATOR,
            AssetClass.STATISTICS
        }
        return self not in non_tradable
    
    @property
    def display_name(self) -> str:
        """中文显示名称"""
        names = {
            AssetClass.EQUITY: "股票",
            AssetClass.EQUITY_GEM: "创业板",
            AssetClass.EQUITY_CONNECT: "跨市场合约",
            AssetClass.EQUITY_PREFERRED: "优先股",
            AssetClass.EQUITY_DEPOSITORY_RECEIPT: "存托凭证",
            AssetClass.BOND: "债券",
            AssetClass.MUTUAL_FUND: "基金",
            AssetClass.MONEY_MARKET_FUND: "货币基金",
            AssetClass.ETF: "ETF",
            AssetClass.FUTURE: "期货",
            AssetClass.OPTION: "期权",
            AssetClass.WARRANT: "权证",
            AssetClass.COMMODITY: "商品",
            AssetClass.FX_SPOT: "外汇",
            AssetClass.INDEX: "指数",
            AssetClass.MACRO_INDICATOR: "宏观指标",
            AssetClass.OTC: "场外交易",
            AssetClass.NEEQ: "股转系统",
            AssetClass.SYNTHETIC: "合成合约",
            AssetClass.REPO: "回购",
            AssetClass.STATISTICS: "统计数据",
            AssetClass.BENCHMARK: "基准指数",
            AssetClass.TREASURY: "国债",
            AssetClass.CONVERTIBLE_BOND: "可转债",
            AssetClass.FX_FORWARD: "外汇远期",
            AssetClass.LOF: "上市开放式基金",
            AssetClass.PRIVATE_EQUITY: "私募基金",
            AssetClass.BOND_PRE_ISSUE: "国债预发行",
            AssetClass.SWAP: "互换",
        }
        return names.get(self, self.value)