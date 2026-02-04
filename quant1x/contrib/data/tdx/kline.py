# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
from typing import Optional, Any, List
from datetime import datetime
from quant1x.contrib.calendar import next_trading_day
from quant1x.data import adapter
from quant1x.data.base import BASE_KLINE, BASE_RAW_DAILY_KLINE, MarketCnFirstListTime
from quant1x.data.frequency import Frequency, TimeUnit
from quant1x.data.market import Instrument, detect_symbol
from quant1x.data import config, Timestamp, KLine, MaxCachedDaysToDropOnIncrementalUpdate, CumulativeAdjustment, XdxrInfo
from .client import get_std_conn
from . import protocol
from .level1 import KLineType, SecurityBar, SecurityBarsRequest, SecurityBarsResponse, SECURITY_BARS_PRE_REQUEST_MAX
import pandas as pd
from quant1x.log import logger
from .kline_raw import KLineRaw, checkout_kline_raw, fetch_kline_raw


def apply_forward_adjustment_for_event(klines: List[KLine], 
                                       current_start_date: Timestamp, 
                                       dividends: List[XdxrInfo]):
    if not klines:
        return
        
    # 最后一根K线的日期
    last_day = klines[-1].date
    # 转成时间戳且对齐时间
    ts_last_day = Timestamp.parse(last_day).get_pre_market_time()
    # 计算最后一根K线的下一个交易日的日期
    last_trading_day_ts = next_trading_day(ts_last_day)
    last_day_next = last_trading_day_ts.only_date()
    start_date_str = current_start_date.only_date()
    
    # Filter dividends
    xdxr_infos = [
        x for x in dividends 
        if x.Date <= last_day_next and x.Category == 1
    ]
    
    # Sort by date? C++ uses std::views::filter, order depends on input.
    # Assuming dividends are sorted by date.
    
    times = len(xdxr_infos)
    
    for info in xdxr_infos:
        if info.Date <= start_date_str:
            # IPO check logic in C++ is commented out or specific?
            # "除权除息数据在日线第一条数据之前... continue" is commented out in C++.
            pass
        else:
            m, a = info.adjust_factor()
            share_ratio = info.compute_share_adjustment_ratio()
            
            for kline in klines:
                if kline.date >= info.Date:
                    break
                
                if kline.date < info.Date:
                    kline.open = kline.open * m + a
                    kline.close = kline.close * m + a
                    kline.high = kline.high * m + a
                    kline.low = kline.low * m + a
                    
                    if kline.volume != 0:
                        ap = kline.amount / kline.volume
                        ap_adjusted = ap * m + a
                        kline.volume *= (1 + share_ratio)
                        kline.amount = kline.volume * ap_adjusted
                    
                    kline.adjustment_count += 1
        times -= 1


def save_kline(filename: str, values: List[KLine]):
    if not values:
        return
        
    dirname = os.path.dirname(filename)
    if dirname:
        os.makedirs(dirname, exist_ok=True)
    
    data = [
        {
            "date": v.date,
            "open": v.open,
            "close": v.close,
            "high": v.high,
            "low": v.low,
            "volume": v.volume,
            "amount": v.amount,
            "up": v.up,
            "down": v.down,
            "datetime": v.datetime,
            "adjustment_count": v.adjustment_count
        }
        for v in values
    ]
    
    df = pd.DataFrame(data, columns=KLine.headers())
    df.to_csv(filename, index=False)

def read_kline_from_csv(filename: str) -> List[KLine]:
    klines = []
    if not os.path.exists(filename):
        return klines
        
    try:
        df = pd.read_csv(filename)
        # Ensure columns exist
        required_cols = KLine.headers()
        if not all(col in df.columns for col in required_cols):
            return klines
            
        for _, row in df.iterrows():
            kline = KLine(
                date=str(row['date']),
                open=float(row['open']),
                close=float(row['close']),
                high=float(row['high']),
                low=float(row['low']),
                volume=float(row['volume']),
                amount=float(row['amount']),
                up=int(row['up']),
                down=int(row['down']),
                datetime=str(row['datetime']),
                adjustment_count=int(row['adjustment_count'])
            )
            klines.append(kline)
    except Exception as e:
        logger.error(f"Failed to read kline csv {filename}: {e}")
    
    return klines

def get_kline_filename(inst: Instrument, freq: Frequency=Frequency(1, TimeUnit.DAY)) -> str:
    module_name = freq.cache_key()
    symbol = inst.symbol()
    symbol_path = symbol[:-3]
    return f'{config.data_path}/{module_name}/{symbol_path}/{symbol}.csv' 
    
def load_kline(inst: Instrument, freq: Frequency=Frequency(1, TimeUnit.DAY)) -> List[KLine]:
    filename = get_kline_filename(inst, freq)
    logger.debug(f"[dataset::KLine] kline file: {filename}")
    return read_kline_from_csv(filename)



from .xdxr import get_xdxr_list

class DataKLine(adapter.DataAdapter):
    def kind(self) -> int:
        return BASE_KLINE
        
    def owner(self) -> str:
        return adapter.DEFAULT_DATA_PROVIDER
        
    def key(self) -> str:
        return "day"
        
    def name(self) -> str:
        return "前复权K线"
        
    def usage(self) -> str:
        return "前复权K线数据"
        
    def print(self, inst: Instrument, date: Optional[Timestamp] = None) -> None:
        pass
        
    def update(self, inst: Instrument, date: Optional[Timestamp] = None) -> None:
        # 1. Determine start date from local cache
        current_start_date = Timestamp.parse(MarketCnFirstListTime) # market_first_date
        freq = Frequency(num=1, unit=TimeUnit.DAY)
        cache_filename = get_kline_filename(inst, freq)
        cache_klines = read_kline_from_csv(cache_filename)
        
        klines_length = len(cache_klines)
        klines_offset_days = MaxCachedDaysToDropOnIncrementalUpdate
        adjust_times = 0
        
        if klines_length > 0:
            if klines_offset_days > klines_length:
                klines_offset_days = klines_length
            
            kline = cache_klines[klines_length - klines_offset_days]
            current_start_date = Timestamp.parse(kline.date)
            adjust_times = kline.adjustment_count
            
        # 2. Determine end date
        current_end_date = Timestamp.now().get_pre_market_time()
        logger.debug(f"[dataset::KLine] [{inst.symbol()}]: from {current_start_date.only_date()} to {current_end_date.only_date()}")
        
        step = SECURITY_BARS_PRE_REQUEST_MAX
        start = 0
        hs: List[List[SecurityBar]] = []
        element_count = 0
        
        while True:
            count = step
            reply = fetch_kline_raw(inst, start, count, freq)
            if not reply:
                break
                
            element_count += len(reply)
            hs.append(reply)
            
            last_bar = reply[-1]
            last_bar_date = Timestamp.parse(f"{last_bar.Year}-{last_bar.Month:02d}-{last_bar.Day:02d}").get_pre_market_time()
            
            if last_bar_date < current_start_date:
                break
                
            if len(reply) < count:
                break
                
            start += count
            
        hs.reverse()
        
        incremental_klines: List[KLine] = []
        
        for vec in hs:
            for row in vec:
                date_time = Timestamp.parse(f"{row.Year}-{row.Month:02d}-{row.Day:02d}").get_pre_market_time()
                if date_time < current_start_date or date_time > current_end_date:
                    continue
                    
                kx = KLine(
                    date=date_time.only_date(),
                    open=row.Open,
                    close=row.Close,
                    high=row.High,
                    low=row.Low,
                    volume=row.Vol * 100, # Convert to shares
                    amount=row.Amount,
                    up=row.UpCount,
                    down=row.DownCount,
                    datetime=row.DateTime,
                    adjustment_count=0
                )
                incremental_klines.append(kx)
                
        # 6. Adjustment logic
        is_fresh_fetch_require_adjustment = (adjust_times == 1)
        dividends = get_xdxr_list(inst)
        
        if is_fresh_fetch_require_adjustment:
            apply_forward_adjustment_for_event(incremental_klines, current_start_date, dividends)
            
        # 7. Merge
        klines = []
        if klines_length > klines_offset_days:
            klines.extend(cache_klines[:klines_length - klines_offset_days])
            
        klines.extend(incremental_klines)
        
        # 8. Forward adjust
        if not is_fresh_fetch_require_adjustment:
            apply_forward_adjustment_for_event(klines, current_start_date, dividends)
            
        # 9. Save
        save_kline(cache_filename, klines)


# 注册插件
_data_kline_plugin = adapter.register(DataKLine)
    

def check_kline_offset(klines: List[Any], date: str, freq: Frequency=Frequency(1, TimeUnit.DAY)) -> int:
    rows = len(klines)
    offset = 0
    for i in range(rows):
        kline_date = klines[rows - 1 - i].date
        if kline_date < date:
            return -1
        elif kline_date == date:
            break
        else:
            offset += 1
    if offset+1 >= rows:
        return -1
    return offset

def ipo_date_from_xdxrs(xdxr_list: List[XdxrInfo]) -> Optional[str]:
    """
    从除权除息的列表提取IPO日期
    """
    for v in xdxr_list:
        if v.Category != 5:
            continue
        # 如果首次, 前流通前总股本为0且后流通后总股本大于0, 即为上市日期
        if v.QianLiuTong == 0 and v.QianZongGuBen == 0 and v.HouLiuTong > 0 and v.HouZongGuBen > 0:
            return v.Date
    return None

def combine_adjustments_in_period(xdxr_list: List[XdxrInfo],
                                  start_date: Timestamp,
                                  end_date: Timestamp) -> List[CumulativeAdjustment]:
    """
    聚合给定一个时间范围内的复权因子
    """
    result: List[CumulativeAdjustment] = []
    
    for info in xdxr_list:
        if not info.is_adjust():
            continue

        # 统一盘前时间
        event_date = datetime.strptime(info.Date, '%Y-%m-%d')
        event_ts = Timestamp.pre_market_time(event_date.year, event_date.month, event_date.day)
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


def apply_forward_adjustment_incrementally(klines: List[KLine],
                                           xdxr_list: List[XdxrInfo],
                                           last_adjusted_date: Timestamp,
                                           as_of_date: Timestamp,
                                           truncate_to_as_of_date: bool = True):
    """
    对K线数据进行增量式前复权处理，按时间顺序逐步应用复权因子。
    
    Args:
        klines (List[Any]): 待复权的K线数据列表，会被原地修改
        xdxr_list (List[XdxrInfo]): 除权除息信息列表
        last_adjusted_date (Timestamp): 复权开始时间
        as_of_date (Timestamp): 复权结束时间
        truncate_to_as_of_date (bool, optional): 是否截断处理后的数据到as_of_date，默认为True
    
    Note:
        1. 会自动将时间统一转换为盘前时间
        2. 当遇到不再需要复权的数据且truncate_to_as_of_date为False时会提前终止循环
        3. 会原地修改klines列表中的数据
        4. 如果时间范围内没有需要处理的除权记录，则直接返回
    """
    if not klines:
        return

    # 强制统一为盘前时间
    ts_start = last_adjusted_date
    ts_end = as_of_date
    factors = combine_adjustments_in_period(xdxr_list, ts_start, ts_end)
    
    # 如果在时间范围内没有需要除权处理的记录, 则返回
    if not factors:
        return

    factors_count = len(factors)
    i = 0  # 除权因子从第一个记录开始
    rows = 0
    klines_count = len(klines)
    
    for idx in range(klines_count):
        kline = klines[idx]
        current_date_dt = datetime.strptime(kline.date, '%Y-%m-%d')
        current_date = Timestamp.pre_market_time(current_date_dt.year, current_date_dt.month, current_date_dt.day)
        
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
            elif not truncate_to_as_of_date:
                # 如果不截断数据, 那么, 对于已经没有需要复权的因子来说，后面的klines数据就没必要继续循环了
                break
        
        rows += 1

    if truncate_to_as_of_date:
        del klines[rows:]

def calculate_pre_adjust(klines: List[KLine], xdxr_list: List[XdxrInfo]):
    """
    对K线数据进行前复权计算
    """
    if not klines:
        return
        
    # 使用apply_forward_adjustments_once进行前复权
    start_date = datetime.strptime(klines[0].date, '%Y-%m-%d')
    end_date = datetime.strptime(klines[-1].date, '%Y-%m-%d')
    start_ts = Timestamp.pre_market_time(start_date.year, start_date.month, start_date.day)
    end_ts = Timestamp.pre_market_time(end_date.year, end_date.month, end_date.day)
    apply_forward_adjustment_incrementally(klines, xdxr_list, start_ts, end_ts, True)


def get_cross_section_forward_adjusted_klines(code: str, as_of_date: str) -> List[Any]:
    """
    获取指定证券代码截至指定日期的前复权K线数据
    
    Args:
        code (str): 证券代码，支持多种格式输入
        as_of_date (str): 截止日期，格式为YYYY-MM-DD
    
    Returns:
        List[KLine]: 从上市首日至截止日期的所有前复权K线记录列表，包含日期、开盘价、收盘价、最高价、最低价、成交量等字段
    
    Note:
        1. 会自动处理证券代码格式转换
        2. 会对原始K线数据进行日期对齐和过滤
        3. 会应用前复权计算调整价格数据
    """
    inst = detect_symbol(code)
    ts = Timestamp.parse(as_of_date)
    fixed_date = ts.only_date()
    
    # 获取所有原始K线数据
    raw_klines = checkout_kline_raw(inst)
    
    if not raw_klines:
        return []
    
    # 检查是否最新数据
    last_kline = raw_klines[-1]
    if last_kline.date < fixed_date:
        # 数据太旧, 重新加载 (但checkout_kline_raw应该已经处理)
        raw_klines = checkout_kline_raw(inst)
    
    # 对齐数据缓存的日期, 过滤可能存在停牌没有数据的情况
    offset = check_kline_offset(raw_klines, fixed_date)
    if offset < 0:
        return []
    
    fixed_count = len(raw_klines) - offset
    filtered_klines = raw_klines[:fixed_count]
    
    if not filtered_klines:
        return []
    
    # 将KLineRaw转换为KLine
    klines = []
    for raw_kline in filtered_klines:
        kline = KLine(
            date=raw_kline.date,
            open=raw_kline.open,
            close=raw_kline.close,
            high=raw_kline.high,
            low=raw_kline.low,
            volume=raw_kline.volume,
            amount=raw_kline.amount,
            up=raw_kline.up,
            down=raw_kline.down,
            datetime=raw_kline.datetime,
            adjustment_count=0
        )
        klines.append(kline)
    
    # 获取XDXR数据
    xdxr_list = get_xdxr_list(inst)
    
    # 确定前复权的时间范围
    start_date = datetime.strptime(klines[0].date, '%Y-%m-%d')
    end_date = datetime.strptime(klines[-1].date, '%Y-%m-%d')
    start_ts = Timestamp.pre_market_time(start_date.year, start_date.month, start_date.day)
    end_ts = Timestamp.pre_market_time(end_date.year, end_date.month, end_date.day)
    
    # 应用前复权
    apply_forward_adjustment_incrementally(klines, xdxr_list, start_ts, end_ts, True)
    
    return klines


if __name__ == "__main__":
    import pandas as pd
    
    # 获取未复权K线数据
    code = "300773"
    inst = detect_symbol(code)
    cache = DataKLine()
    cache.update(inst)
    symbol = inst.symbol()
    
    raw_klines = checkout_kline_raw(inst)
    klines = [KLine(
        date=k.date, open=k.open, close=k.close, high=k.high, low=k.low,
        volume=k.volume, amount=k.amount, up=k.up, down=k.down, datetime=k.datetime, adjustment_count=0
    ) for k in raw_klines]
    print(f"Loaded {len(klines)} raw kline records for {code}")

    if not klines:
        print("No kline data available")
        exit(1)

    # 获取除权除息数据
    xdxr_list = get_xdxr_list(inst)
    print(f"Loaded {len(xdxr_list)} xdxr records for {symbol}")

    # 创建原始数据的副本用于对比
    original_klines = [KLine(
        date=k.date, open=k.open, close=k.close, high=k.high, low=k.low,
        volume=k.volume, amount=k.amount, up=k.up, down=k.down, datetime=k.datetime
    ) for k in klines]

    # 进行前复权
    calculate_pre_adjust(klines, xdxr_list)

    print(f"After adjustment: {len(klines)} klines")

    # 转换为pandas DataFrame进行显示
    def klines_to_dataframe(kline_list, prefix=""):
        data = []
        for k in kline_list:
            data.append({
                f"{prefix}date": k.date,
                f"{prefix}open": k.open,
                f"{prefix}high": k.high,
                f"{prefix}low": k.low,
                f"{prefix}close": k.close,
                f"{prefix}volume": k.volume,
                f"{prefix}amount": k.amount,
                f"{prefix}up": k.up,
                f"{prefix}down": k.down
            })
        return pd.DataFrame(data)

    # 创建DataFrame
    original_df = klines_to_dataframe(original_klines[-20:], "raw_")  # 显示最近20条
    adjusted_df = klines_to_dataframe(klines[-20:], "adj_")  # 显示最近20条

    # 合并显示
    comparison_df = pd.concat([original_df, adjusted_df], axis=1)

    print("\n=== 复权前后对比 (2024年样本数据) ===")
    # 查找2024年的数据进行对比
    sample_indices = []
    for i, kline in enumerate(original_klines):
        if kline.date.startswith('2024'):
            sample_indices.append(i)
        if len(sample_indices) >= 5:  # 收集5个样本
            break
    
    for idx in sample_indices:
        orig = original_klines[idx]
        adj = klines[idx]
        print(f"日期: {orig.date}")
        print(f"  原始: 开={orig.open:.2f}, 高={orig.high:.2f}, 低={orig.low:.2f}, 收={orig.close:.2f}")
        print(f"  复权: 开={adj.open:.2f}, 高={adj.high:.2f}, 低={adj.low:.2f}, 收={adj.close:.2f}")
        if abs(orig.close - adj.close) > 0.01:  # 如果有差异，显示调整因子
            factor = adj.close / orig.close if orig.close != 0 else 1.0
            print(f"  调整因子: {factor:.4f}")
        print()

    # 显示复权因子信息
    if xdxr_list:
        print(f"\n=== 除权除息记录 ({len(xdxr_list)} 条) ===")
        # 显示不同类别的记录
        categories = {}
        for xdxr in xdxr_list:
            cat = xdxr.Category
            if cat not in categories:
                categories[cat] = []
            categories[cat].append(xdxr)
        
        for cat, records in categories.items():
            print(f"类别 {cat} ({len(records)} 条): {records[0].Name}")
            for xdxr in records[-2:]:  # 每类显示最近2条
                print(f"  日期: {xdxr.Date}, 分红:{xdxr.FenHong}, 送转:{xdxr.SongZhuanGu}, 配股:{xdxr.PeiGu}")

    # 对比 get_cross_section_forward_adjusted_klines 与 datasets.kline 缓存
    print(f"\n=== 对比 get_cross_section_forward_adjusted_klines 与 datasets.kline 缓存 ===")
    
    # 获取 datasets.kline 缓存数据
    #from data.kline import load_kline
    cached_klines = load_kline(inst)
    
    if cached_klines:
        first_cached_date = cached_klines[0].date
        last_cached_date = cached_klines[-1].date
        print(f"datasets.kline 缓存日期范围: {first_cached_date} 到 {last_cached_date}")
        
        # 使用 get_cross_section_forward_adjusted_klines 获取相同日期范围的复权数据
        # 使用最后一条数据的日期作为截止日期
        adjusted_klines = get_cross_section_forward_adjusted_klines(code, last_cached_date)
        
        if adjusted_klines and len(adjusted_klines) > 0:
            # 找到相同日期的第一条数据进行对比
            first_adjusted = None
            first_cached = cached_klines[0]
            
            for kline in adjusted_klines:
                if kline.date == first_cached.date:
                    first_adjusted = kline
                    break
            
            if first_adjusted:
                print(f"\n在 {first_cached.date} 的数据对比:")
                print(f"get_cross_section_forward_adjusted_klines:")
                print(f"  开盘: {first_adjusted.open:.4f}, 最高: {first_adjusted.high:.4f}, 最低: {first_adjusted.low:.4f}, 收盘: {first_adjusted.close:.4f}")
                print(f"  成交量: {first_adjusted.volume:.0f}, 成交额: {first_adjusted.amount:.0f}")
                
                print(f"datasets.kline 缓存:")
                print(f"  开盘: {first_cached.open:.4f}, 最高: {first_cached.high:.4f}, 最低: {first_cached.low:.4f}, 收盘: {first_cached.close:.4f}")
                print(f"  成交量: {first_cached.volume:.0f}, 成交额: {first_cached.amount:.0f}")
                
                # 对比数据
                print(f"\n差异:")
                print(f"  开盘价: {abs(first_adjusted.open - first_cached.open):.6f}")
                print(f"  收盘价: {abs(first_adjusted.close - first_cached.close):.6f}")
                print(f"  最高价: {abs(first_adjusted.high - first_cached.high):.6f}")
                print(f"  最低价: {abs(first_adjusted.low - first_cached.low):.6f}")
                print(f"  成交量: {abs(first_adjusted.volume - first_cached.volume):.0f}")
                print(f"  成交额: {abs(first_adjusted.amount - first_cached.amount):.0f}")
                
                if all([
                    abs(first_adjusted.open - first_cached.open) < 0.0001,
                    abs(first_adjusted.close - first_cached.close) < 0.0001,
                    abs(first_adjusted.high - first_cached.high) < 0.0001,
                    abs(first_adjusted.low - first_cached.low) < 0.0001,
                    abs(first_adjusted.volume - first_cached.volume) < 1,
                    abs(first_adjusted.amount - first_cached.amount) < 1
                ]):
                    print("SUCCESS: 数据完全匹配！")
                else:
                    print("ERROR: 数据存在差异")
                    
                    # 检查调整次数
                    print(f"调整次数对比:")
                    print(f"  get_cross_section_forward_adjusted_klines: {getattr(first_adjusted, 'adjustment_count', 'N/A')}")
                    print(f"  datasets.kline: {getattr(first_cached, 'adjustment_count', 'N/A')}")
            else:
                print(f"get_cross_section_forward_adjusted_klines 中找不到日期 {first_cached.date} 的数据")
                print(f"adjusted_klines 长度: {len(adjusted_klines)}")
                if len(adjusted_klines) > 0:
                    print(f"第一条: {adjusted_klines[0].date}, 最后一条: {adjusted_klines[-1].date}")
        else:
            print("get_cross_section_forward_adjusted_klines 返回空数据")
    else:
        print("datasets.kline 缓存为空")