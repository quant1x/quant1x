# -*- coding: UTF-8 -*-
from dataclasses import dataclass
from typing import List, Optional, TypeVar, Generic, Any
import math

from quant1x.exchange import Timestamp
from quant1x.level1.xdxr_info import XdxrInfo
from quant1x.datasets import xdxr

T = TypeVar('T')

@dataclass
class CumulativeAdjustment:
    timestamp: Timestamp
    m: float = 0.0
    a: float = 0.0
    monetary_adjustment: float = 0.0
    share_adjustment_ratio: float = 0.0
    no: int = 0

    def to_string(self) -> str:
        return (f"{{no={self.no},timestamp={self.timestamp.only_date()},"
                f"m={self.m},a={self.a},"
                f"monetaryAdjustment={self.monetary_adjustment},"
                f"shareAdjRatio={self.share_adjustment_ratio}}}")

    def apply(self, price: float) -> float:
        return price * self.m + self.a

    def inverse(self, adjusted_price: float) -> float:
        return (adjusted_price - self.a) / self.m

def get_xdxr_list(security_code: str) -> List[XdxrInfo]:
    """
    通过证券代码获取最新的除权除息列表
    """
    return xdxr.load_xdxr(security_code)

def ipo_date_from_xdxrs(xdxrs: List[XdxrInfo]) -> Optional[str]:
    """
    从除权除息的列表提取IPO日期
    """
    for v in xdxrs:
        if v.Category != 5:
            continue
        # 如果首次, 前流通前总股本为0且后流通后总股本大于0, 即为上市日期
        if v.QianLiuTong == 0 and v.QianZongGuBen == 0 and v.HouLiuTong > 0 and v.HouZongGuBen > 0:
            return v.Date
    return None

def combine_adjustments_in_period(xdxrs: List[XdxrInfo],
                                  start_date: Timestamp,
                                  end_date: Timestamp) -> List[CumulativeAdjustment]:
    """
    聚合给定一个时间范围内的复权因子
    """
    result: List[CumulativeAdjustment] = []
    
    for info in xdxrs:
        if not info.is_adjust():
            continue

        # 统一盘前时间
        event_ts = Timestamp(info.Date).pre_market_time()
        if event_ts < start_date or event_ts > end_date:
            continue

        m, a = info.adjust_factor()
        # 使用 level1::XdxrInfo 中封装的 helper 计算事件级别的货币与股本调整
        event_monetary_adjustment = info.compute_monetary_adjustment()
        event_share_adjustment_ratio = info.compute_share_adjustment_ratio()

        for factor in result:
            # 叠加复权因子 (保持之前的 m/a 合并算法)
            factor.m *= m
            factor.a = m * factor.a + a
            factor.no += 1

            # 使用组合规则直接更新累计的货币/股本调整
            old_monetary_adjustment = factor.monetary_adjustment
            old_share_adjustment_ratio = factor.share_adjustment_ratio
            
            new_share_adjustment_ratio = (old_share_adjustment_ratio + event_share_adjustment_ratio +
                                          old_share_adjustment_ratio * event_share_adjustment_ratio)
            new_monetary_adjustment = (old_monetary_adjustment +
                                       event_monetary_adjustment * (1.0 + old_share_adjustment_ratio))
            
            factor.monetary_adjustment = new_monetary_adjustment
            factor.share_adjustment_ratio = new_share_adjustment_ratio

        # 将当前事件作为新的累计因子条目加入，并设置其货币/股本字段
        entry = CumulativeAdjustment(
            timestamp=event_ts,
            m=m,
            a=a,
            monetary_adjustment=event_monetary_adjustment,
            share_adjustment_ratio=event_share_adjustment_ratio,
            no=1
        )
        result.append(entry)
        
    return result

def apply_forward_adjustments_once(klines: List[Any],
                                   xdxrs: List[XdxrInfo],
                                   start_date: Timestamp,
                                   end_date: Timestamp,
                                   should_truncate: bool = True):
    """
    一次性复权, 只遍历一次
    """
    if not klines:
        return

    # 强制统一为盘前时间
    ts_start = start_date.pre_market_time()
    ts_end = end_date.pre_market_time()
    factors = combine_adjustments_in_period(xdxrs, ts_start, ts_end)
    
    # 如果在时间范围内没有需要除权处理的记录, 则返回
    if not factors:
        return

    factors_count = len(factors)
    i = 0  # 除权因子从第一个记录开始
    rows = 0
    klines_count = len(klines)
    
    for idx in range(klines_count):
        kline = klines[idx]
        current_date = Timestamp(kline.Date).pre_market_time()
        
        if i < factors_count:
            factor = factors[i]
            
            if current_date > ts_end:
                break
                
            # 如果日线日期大于因子的日期, 因子索引+, 自动切换下一个因子
            while i + 1 < factors_count and current_date >= factor.timestamp:
                i += 1
                factor = factors[i]
                
            if current_date < factor.timestamp:
                # Assuming kline has an adjust method or we modify it directly
                # In C++, kline->adjust(factor) is called.
                # We need to ensure the KLine object has this method.
                if hasattr(kline, 'adjust'):
                    kline.adjust(factor)
            elif not should_truncate:
                # 如果不截断数据, 那么, 对于已经没有需要复权的因子来说，后面的klines数据就没必要继续循环了
                break
        
        rows += 1

    if should_truncate:
        del klines[rows:]

def calculate_pre_adjust(klines: List[Any], dividends: List[XdxrInfo]):
    """
    对K线数据进行前复权计算
    """
    if not klines:
        return
        
    # 使用apply_forward_adjustments_once进行前复权
    start_ts = Timestamp(klines[0].Date).pre_market_time()
    end_ts = Timestamp(klines[-1].Date).pre_market_time()
    apply_forward_adjustments_once(klines, dividends, start_ts, end_ts, True)
