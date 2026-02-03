# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
from typing import Optional, List, Tuple, Any
from dataclasses import dataclass
from enum import Enum
from datetime import datetime
from .timestamp import Timestamp

def get_today_initialized_time() -> Timestamp:
    """获取今天初始化时间"""
    now = Timestamp.now()
    return now.get_pre_market_time()

def get_filename_modified_time(fname: str) -> Timestamp:
    """获取文件最后修改时间"""
    if not os.path.exists(fname):
        return Timestamp.zero()
    try:
        info = os.lstat(fname)
        dt = datetime.fromtimestamp(info.st_mtime)
        return Timestamp.from_datetime(dt)
    except OSError:
        # 可能因权限、竞争条件（文件被删除）等导致 stat 失败
        return Timestamp.zero()


MaxCachedDaysToDropOnIncrementalUpdate = 1
"""
    是增量更新缓存清理的最大天数。
    该机制确保在 A 股除权除息日等场景下，当日数据能被正确覆盖。
    由于 A 股的复权处理以交易日为单位，且同一天内可能多次更新数据，
    因此需先删除缓存中已有的当日记录，再插入最新增量数据。
"""


class XdxrCategory(Enum):
    EX_DIVIDEND = 1                       # 除权除息
    BONUS_SHARES_LISTING = 2              # 送股上市（无偿）
    RESTRICTED_SHARES_LISTING = 3         # 非流通股上市（受限股解禁）
    UNSPECIFIED_CAPITAL_ADJUSTMENT = 4    # 未知股本变动
    GENERAL_CAPITAL_ADJUSTMENT = 5        # 股本变化（保留，但慎用）
    NEW_SHARE_ISSUANCE = 6                # 增发新股
    SHARE_REPURCHASE = 7                  # 股份回购
    NEW_SHARES_LISTING = 8                # 增发新股上市
    TRANSFERRED_RIGHTS_SHARES_LISTING = 9 # 转配股上市（中国特有）
    CONVERTIBLE_BOND_LISTING = 10         # 可转债上市
    STOCK_SPLIT_OR_REVERSE_SPLIT = 11     # 拆股或合股
    RESTRICTED_SHARES_CONSOLIDATION = 12  # 非流通股缩股
    ISSUE_CALL_WARRANTS = 13              # 送认购权证
    ISSUE_PUT_WARRANTS = 14               # 送认沽权证

    @staticmethod
    def to_string(category: int) -> str:
        try:
            return {
                1: "除权除息",
                2: "送配股上市",
                3: "非流通股上市",
                4: "未知股本变动",
                5: "股本变化",
                6: "增发新股",
                7: "股份回购",
                8: "增发新股上市",
                9: "转配股上市",
                10: "可转债上市",
                11: "扩缩股",
                12: "非流通股缩股",
                13: "送认购权证",
                14: "送认沽权证"
            }.get(category, f"Unknown({category})")
        except:
            return f"Unknown({category})"
@dataclass
class XdxrInfo:
    Date: str = ""           # 日期 YYYY-MM-DD格式
    Category: int = 0        # 类型编号
    Name: str = ""           # 类型名称
    FenHong: float = 0.0     # 分红(元)
    PeiGuJia: float = 0.0    # 配股价(元)
    SongZhuanGu: float = 0.0 # 送转股(股)
    PeiGu: float = 0.0       # 配股(股)
    SuoGu: float = 0.0       # 缩股(股)
    QianLiuTong: float = 0.0 # 除权前流通股(万股)
    HouLiuTong: float = 0.0  # 除权后流通股(万股)
    QianZongGuBen: float = 0.0 # 除权前总股本(万股)
    HouZongGuBen: float = 0.0  # 除权后总股本(万股)
    FenShu: float = 0.0      # 权证份数
    XingQuanJia: float = 0.0 # 行权价格(元)

    def is_adjust(self) -> bool:
        count = self.FenHong
        count += self.PeiGu
        count += self.SongZhuanGu
        count += self.SuoGu
        count += self.FenShu
        return count > 0.00

    def adjust_factor(self) -> Tuple[float, float]:
        m = 0.0
        a = 0.0
        
        A = self.compute_monetary_adjustment()
        B = self.compute_share_adjustment_ratio()
        
        if abs(1.0 + B) > 1e-10:
            m = 1.0 / (1.0 + B)
            a = A * m
        else:
            m = 1.0
            a = 0.0
            
        return m, a

    def compute_monetary_adjustment(self) -> float:
        return (self.PeiGu * self.PeiGuJia - self.FenHong + self.FenShu * self.XingQuanJia) / 10.0

    def compute_share_adjustment_ratio(self) -> float:
        return (self.SongZhuanGu + self.PeiGu - self.SuoGu + self.FenShu) / 10.0

    def is_capital_change(self) -> bool:
        if self.Category in [
            XdxrCategory.EX_DIVIDEND.value,
            XdxrCategory.STOCK_SPLIT_OR_REVERSE_SPLIT.value,
            XdxrCategory.RESTRICTED_SHARES_CONSOLIDATION.value,
            XdxrCategory.ISSUE_CALL_WARRANTS.value,
            XdxrCategory.ISSUE_PUT_WARRANTS.value
        ]:
            return False
        
        if self.HouLiuTong > 0 and self.HouZongGuBen > 0:
            return True
        return False

    def adjust(self):
        """
        生成复权计算函数
        返回: 计算复权价格的函数对象 (callable)
        """
        song_zhuangu = self.SongZhuanGu
        pei_gu = self.PeiGu
        suo_gu = self.SuoGu
        xdxr_gu_shu = (song_zhuangu + pei_gu - suo_gu) / 10.0
        
        fen_hong = self.FenHong
        pei_gu_jia = self.PeiGuJia
        xdxr_fen_hong = (pei_gu_jia * pei_gu - fen_hong) / 10.0

        def calculator(p: float) -> float:
            return (p + xdxr_fen_hong) / (1 + xdxr_gu_shu)
            
        return calculator

@dataclass
class CumulativeAdjustment:
    """复权数据结构体"""
    timestamp: Timestamp # 复权日期
    m: float = 0.0 # 乘性因子（Multiplier），处理比例调整（如送股）
    a: float = 0.0 # 加性因子（Additive），处理平移调整（如分红）
    monetary_adjustment: float = 0.0 # 货币调整，用于价格复权（P' = P * (1 + ratio)）
    share_adjustment_ratio: float = 0.0 # 股本调整比率，用于成交量复权（V' = V * (1 + ratio)）
    no: int = 0 # 本次复权调整的序号（从1开始），用于追踪应用顺序

    def to_string(self) -> str:
        return (f"{{no={self.no},timestamp={self.timestamp.only_date()},"
                f"m={self.m},a={self.a},"
                f"monetary_adjustment={self.monetary_adjustment},"
                f"shareAshare_adjustment_ratio={self.share_adjustment_ratio}}}")

    def apply(self, price: float) -> float:
        """复权"""
        return price * self.m + self.a

    def inverse(self, adjusted_price: float) -> float:
        """还权"""
        return (adjusted_price - self.a) / self.m

@dataclass
class KLine:
    """K线数据结构体"""
    date: str = ""
    open: float = 0.0
    close: float = 0.0
    high: float = 0.0
    low: float = 0.0
    volume: float = 0.0
    amount: float = 0.0
    up: int = 0
    down: int = 0
    datetime: str = ""
    adjustment_count: int = 0

    def adjust(self, adj: CumulativeAdjustment):
        """复权"""
        self.open = self.open * adj.m + adj.a
        self.close = self.close * adj.m + adj.a
        self.high = self.high * adj.m + adj.a
        self.low = self.low * adj.m + adj.a
        
        # 成交量复权
        if self.volume != 0:
            # 1. 计算均价
            ap = self.amount / self.volume
            # 2. 均价复权
            ap_adjusted = ap * adj.m + adj.a
            # 3. 成交量复权
            self.volume *= (1 + adj.share_adjustment_ratio)
            # 4. 以新成交量*均价计算成交额
            self.amount = self.volume * ap_adjusted
        
        # 5. 更新除权除息次数
        self.adjustment_count = adj.no

    @staticmethod
    def headers() -> List[str]:
        """K线数据头部"""
        return ["date", "open", "close", "high", "low", "volume", "amount", "up", "down", "datetime", "adjustment_count"]

class Direction(Enum):
    """交易方向"""
    BUY = 0 # 主动买入
    SELL = 1 # 主动卖出
    NEUTRAL = 2 # 中性盘

@dataclass
class Transaction:
    """交易数据结构体"""
    time: str = "" # 时间
    price: float = 0.0 # 价格
    volume: int = 0 # 成交量
    num: int = 0 # 成交笔数
    amount: float = 0.0 # 成交额
    direction: int = 2 # 交易方向

    @staticmethod
    def headers() -> List[str]:
        """逐笔交易数据头部"""
        return ["time", "price", "volume", "num", "amount", "direction"]    

