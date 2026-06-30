# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
from typing import Optional, Any, List
from datetime import datetime

from quant1x.data.meta.calendar import next_trading_day
from quant1x.data import adapter
from quant1x.data.base import BASEDATA_KLINE, MarketCnFirstListTime
from quant1x.data.meta import Timestamp, Frequency, TimeUnit, FREQ_DAILY
from quant1x.data.meta import Instrument, Exchange
from quant1x.data.market import detect_symbol
from quant1x.config import config
from quant1x.data import MaxCachedDaysToDropOnIncrementalUpdate
from quant1x.data.schema import Bar, CumulativeAdjustment, XdxrInfo
from .client import get_std_conn
from . import protocol
from .level1 import BarFreq, SECURITY_BARS_PRE_REQUEST_MAX
import pandas as pd
from quant1x.log import logger
from .bar_raw import BarRaw, checkout_bar_raw, fetch_bar_raw
from .instruments import get_instrument_info


def apply_forward_adjustment_for_event(bars: List[Bar], 
                                       current_start_date: Timestamp, 
                                       dividends: List[XdxrInfo]):
    if not bars:
        return
        
    # 最后一根K线的日期
    last_day = bars[-1].date
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
            
            for bar in bars:
                if bar.date >= info.Date:
                    break
                
                if bar.date < info.Date:
                    bar.open = bar.open * m + a
                    bar.close = bar.close * m + a
                    bar.high = bar.high * m + a
                    bar.low = bar.low * m + a
                    
                    if bar.volume != 0:
                        ap = bar.amount / bar.volume
                        ap_adjusted = ap * m + a
                        bar.volume *= (1 + share_ratio)
                        bar.amount = bar.volume * ap_adjusted
                    
                    bar.adjustment_count += 1
        times -= 1


def save_bar(filename: str, values: List[Bar]):
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
            "timestamp": v.timestamp,
            "adjustment_count": v.adjustment_count
        }
        for v in values
    ]
    
    df = pd.DataFrame(data, columns=Bar.headers())
    df.to_csv(filename, index=False)

def read_bar_from_csv(filename: str) -> List[Bar]:
    bars = []
    if not os.path.exists(filename):
        return bars
        
    try:
        df = pd.read_csv(filename)
        # Ensure columns exist
        required_cols = Bar.headers()
        if not all(col in df.columns for col in required_cols):
            return bars
            
        for _, row in df.iterrows():
            bar = Bar(
                date=str(row['date']),
                open=float(row['open']),
                close=float(row['close']),
                high=float(row['high']),
                low=float(row['low']),
                volume=float(row['volume']),
                amount=float(row['amount']),
                up=int(row['up']),
                down=int(row['down']),
                timestamp=str(row['timestamp']),
                adjustment_count=int(row['adjustment_count'])
            )
            bars.append(bar)
    except Exception as e:
        logger.error(f"Failed to read bar csv {filename}: {e}")
    
    return bars

def get_bar_filename(inst: Instrument, freq: Frequency=FREQ_DAILY) -> str:
    module_name = freq.cache_key()
    symbol = inst.symbol()
    sub=f"{module_name}/{inst.cache_dir()}"
    return f'{config.data_path}/{sub}/{symbol}.csv' 
    
def load_bar(inst: Instrument, freq: Frequency=FREQ_DAILY) -> List[Bar]:
    filename = get_bar_filename(inst, freq)
    logger.debug(f"[dataset::Bar] bar file: {filename}")
    return read_bar_from_csv(filename)


from .xdxr import get_xdxr_list

class DataKLine(adapter.DataAdapter):
    def kind(self) -> int:
        return BASEDATA_KLINE
        
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
        cache_filename = get_bar_filename(inst, freq)
        cache_bars = read_bar_from_csv(cache_filename)
        
        bars_length = len(cache_bars)
        bars_offset_days = MaxCachedDaysToDropOnIncrementalUpdate
        adjust_times = 0
        
        if bars_length > 0:
            if bars_offset_days > bars_length:
                bars_offset_days = bars_length
            
            bar = cache_bars[bars_length - bars_offset_days]
            current_start_date = Timestamp.parse(bar.date)
            adjust_times = bar.adjustment_count
            
        # 2. Determine end date
        current_end_date = Timestamp.now().get_pre_market_time()
        logger.debug(f"[dataset::Bar] [{inst.symbol()}]: from {current_start_date.only_date()} to {current_end_date.only_date()}")
        
        step = SECURITY_BARS_PRE_REQUEST_MAX
        start = 0
        hs: List[List[Bar]] = []
        element_count = 0
        
        while True:
            count = step
            reply = fetch_bar_raw(inst, start, count, freq)
            if not reply:
                break
                
            element_count += len(reply)
            hs.append(reply)
            
            last_bar = reply[-1]
            last_bar_date = Timestamp.parse(last_bar.date).get_pre_market_time()
            
            if last_bar_date < current_start_date:
                break
                
            if len(reply) < count:
                break
                
            start += count
            
        hs.reverse()
        
        incremental_bars: List[Bar] = []
        
        for vec in hs:
            for row in vec:
                date_time = Timestamp.parse(row.date).get_pre_market_time()
                if date_time < current_start_date or date_time > current_end_date:
                    continue
                    
                kx = Bar(
                    date=date_time.only_date(),
                    open=row.open,
                    close=row.close,
                    high=row.high,
                    low=row.low,
                    volume=row.volume * 100, # Convert to shares
                    amount=row.amount,
                    up=row.up,
                    down=row.down,
                    timestamp=row.timestamp,
                    adjustment_count=0
                )
                incremental_bars.append(kx)
                
        # 6. Adjustment logic
        is_fresh_fetch_require_adjustment = (adjust_times == 1)
        dividends = get_xdxr_list(inst)
        
        if is_fresh_fetch_require_adjustment:
            apply_forward_adjustment_for_event(incremental_bars, current_start_date, dividends)
            
        # 7. Merge
        bars = []
        if bars_length > bars_offset_days:
            bars.extend(cache_bars[:bars_length - bars_offset_days])
            
        bars.extend(incremental_bars)
        
        # 8. Forward adjust
        if not is_fresh_fetch_require_adjustment:
            apply_forward_adjustment_for_event(bars, current_start_date, dividends)
            
        # 9. Save
        save_bar(cache_filename, bars)


# 注册插件
_data_kline_plugin = adapter.register(DataKLine)
    

def check_bar_offset(bars: List[Any], as_of_date: str, freq: Frequency=FREQ_DAILY) -> int:
    """
    检查给定日期在K线数据中的偏移位置
    
    Args:
        bars (List[Any]): K线数据列表, 每个元素应包含date字段, 元素类型是鸭子类型, 可以是Bar, BarRaw, SecurityBar
        as_of_date (str): 要查找的目标日期
        freq (Frequency): K线频率, 默认为日线
    
    Returns:
        int: 目标日期在K线中的偏移量(从最新数据开始计数), 
             如果未找到或日期早于最早数据则返回-1
    """
    rows = len(bars)
    offset = 0
    for i in range(rows):
        bar_date = bars[rows - 1 - i].date
        if bar_date < as_of_date:
            return -1
        elif bar_date == as_of_date:
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
            logger.debug(f"[combine_adjustments_in_period]: {info.Date} is not an adjustment")
            continue
        # 统一盘前时间
        event_ts = Timestamp.parse(info.Date).get_pre_market_time()
        if event_ts < start_date or event_ts > end_date:
            logger.debug(f"[combine_adjustments_in_period]: {event_ts} is not in range")
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

        # 将当前事件作为新的累计因子条目加入, 并设置其货币/股本字段
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


def apply_forward_adjustment_incrementally(bars: List[Bar],
                                           xdxr_list: List[XdxrInfo],
                                           last_adjusted_date: Timestamp,
                                           as_of_date: Timestamp,
                                           truncate_to_as_of_date: bool = True):
    """
    对K线数据进行增量式前复权处理, 按时间顺序逐步应用复权因子. 
    
    Args:
        bars (List[Any]): 待复权的K线数据列表, 会被原地修改
        xdxr_list (List[XdxrInfo]): 除权除息信息列表
        last_adjusted_date (Timestamp): 复权开始时间
        as_of_date (Timestamp): 复权结束时间
        truncate_to_as_of_date (bool, optional): 是否截断处理后的数据到as_of_date, 默认为True
    
    Note:
        1. 会自动将时间统一转换为盘前时间
        2. 当遇到不再需要复权的数据且truncate_to_as_of_date为False时会提前终止循环
        3. 会原地修改bars列表中的数据
        4. 如果时间范围内没有需要处理的除权记录, 则直接返回
    """
    if not bars:
        return

    # 强制统一为盘前时间
    ts_start = last_adjusted_date
    ts_end = as_of_date
    logger.debug(f'ts_start={ts_start}, ts_end={ts_end}')
    factors = combine_adjustments_in_period(xdxr_list, ts_start, ts_end)
    #print(factors)
    
    # 如果在时间范围内没有需要除权处理的记录, 则返回
    if not factors:
        return

    factors_count = len(factors)
    i = 0  # 除权因子从第一个记录开始
    rows = 0
    bars_count = len(bars)
    
    for idx in range(bars_count):
        bar = bars[idx]
        current_date_dt = datetime.strptime(bar.date, '%Y-%m-%d')
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
                # Assuming bar has an adjust method or we modify it directly
                # In C++, bar->adjust(factor) is called.
                # We need to ensure the Bar object has this method.
                if hasattr(bar, 'adjust'):
                    bar.adjust(factor)
            elif not truncate_to_as_of_date:
                # 如果不截断数据, 那么, 对于已经没有需要复权的因子来说, 后面的bars数据就没必要继续循环了
                break
        
        rows += 1

    if truncate_to_as_of_date:
        del bars[rows:]

def calculate_pre_adjust(bars: List[Bar], xdxr_list: List[XdxrInfo]):
    """
    对K线数据进行前复权计算
    """
    if not bars:
        return
        
    # 使用apply_forward_adjustments_once进行前复权
    start_date = datetime.strptime(bars[0].date, '%Y-%m-%d')
    end_date = datetime.strptime(bars[-1].date, '%Y-%m-%d')
    start_ts = Timestamp.pre_market_time(start_date.year, start_date.month, start_date.day)
    end_ts = Timestamp.pre_market_time(end_date.year, end_date.month, end_date.day)
    apply_forward_adjustment_incrementally(bars, xdxr_list, start_ts, end_ts, True)


def get_cross_section_forward_adjusted_bars(inst: Instrument, as_of_date: str) -> List[Any]:
    """
    获取指定证券代码截至指定日期的前复权K线数据
    
    Args:
        code (str): 证券代码, 支持多种格式输入
        as_of_date (str): 截止日期, 格式为YYYY-MM-DD
    
    Returns:
        List[Bar]: 从上市首日至截止日期的所有前复权K线记录列表, 包含日期, 开盘价, 收盘价, 最高价, 最低价, 成交量等字段
    
    Note:
        1. 会自动处理证券代码格式转换
        2. 会对原始K线数据进行日期对齐和过滤
        3. 会应用前复权计算调整价格数据
    """
    #inst = detect_symbol(code)
    logger.debug(f"Getting forward adjusted bars for instrument: {inst} as of {as_of_date}")
    if inst is None:
        logger.error(f"Instrument not found for code: {inst}")
        return []
    ts = Timestamp.parse(as_of_date)
    fixed_date = ts.only_date()
    
    # 获取所有原始K线数据
    raw_bars = checkout_bar_raw(inst)
    if not raw_bars:
        return []
    
    # 检查是否最新数据
    last_bar = raw_bars[-1]
    if last_bar.date < fixed_date:
        # 数据太旧, 重新加载 (但checkout_bar_raw应该已经处理)
        raw_bars = checkout_bar_raw(inst)
    
    # 对齐数据缓存的日期, 过滤可能存在停牌没有数据的情况
    offset = check_bar_offset(raw_bars, fixed_date)
    if offset < 0 and inst.exchange in(Exchange.SSE, Exchange.SZSE, Exchange.BSE):
        # 非A的获取全部数据
        return []
    logger.debug(f'offset={offset}')
    
    fixed_count = len(raw_bars) - offset
    filtered_bars = raw_bars[:fixed_count]
    
    if not filtered_bars:
        return []
    
    # 将BarRaw转换为Bar
    bars = []
    for raw_bar in filtered_bars:
        bar = Bar(
            date=raw_bar.date,
            open=raw_bar.open,
            close=raw_bar.close,
            high=raw_bar.high,
            low=raw_bar.low,
            volume=raw_bar.volume,
            amount=raw_bar.amount,
            up=raw_bar.up,
            down=raw_bar.down,
            timestamp=raw_bar.timestamp,
            adjustment_count=0
        )
        bars.append(bar)
    
    # 获取XDXR数据
    xdxr_list = get_xdxr_list(inst)
    
    # 确定前复权的时间范围
    start_date = datetime.strptime(bars[0].date, '%Y-%m-%d')
    end_date = datetime.strptime(bars[-1].date, '%Y-%m-%d')
    start_ts = Timestamp.pre_market_time(start_date.year, start_date.month, start_date.day)
    end_ts = Timestamp.pre_market_time(end_date.year, end_date.month, end_date.day)
    
    # 应用前复权
    apply_forward_adjustment_incrementally(bars, xdxr_list, start_ts, end_ts, True)
    
    return bars


if __name__ == "__main__":
    import pandas as pd
    from .instruments import get_instrument_info
    
    # 获取未复权K线数据
    code = "600600.SH"
    code = '00008.hk'
    inst = get_instrument_info(code)
    cache = DataKLine()
    cache.update(inst)
    symbol = inst.symbol()
    
    raw_bars = checkout_bar_raw(inst)
    bars = [Bar(
        date=k.date, open=k.open, close=k.close, high=k.high, low=k.low,
        volume=k.volume, amount=k.amount, up=k.up, down=k.down, timestamp=k.timestamp, adjustment_count=0
    ) for k in raw_bars]
    print(f"Loaded {len(bars)} raw bar records for {code}")

    if not bars:
        print("No bar data available")
        exit(1)

    # 获取除权除息数据
    xdxr_list = get_xdxr_list(inst)
    print(f"Loaded {len(xdxr_list)} xdxr records for {symbol}")

    # 创建原始数据的副本用于对比
    original_bars = [Bar(
        date=k.date, open=k.open, close=k.close, high=k.high, low=k.low,
        volume=k.volume, amount=k.amount, up=k.up, down=k.down, timestamp=k.timestamp
    ) for k in bars]

    # 进行前复权
    calculate_pre_adjust(bars, xdxr_list)

    print(f"After adjustment: {len(bars)} bars")

    # 转换为pandas DataFrame进行显示
    def bars_to_dataframe(bar_list, prefix=""):
        data = []
        for k in bar_list:
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
    original_df = bars_to_dataframe(original_bars[-20:], "raw_")  # 显示最近20条
    adjusted_df = bars_to_dataframe(bars[-20:], "adj_")  # 显示最近20条

    # 合并显示
    comparison_df = pd.concat([original_df, adjusted_df], axis=1)

    print("\n=== 复权前后对比 (2024年样本数据) ===")
    # 查找2024年的数据进行对比
    sample_indices = []
    for i, bar in enumerate(original_bars):
        if bar.date.startswith('2024'):
            sample_indices.append(i)
        if len(sample_indices) >= 5:  # 收集5个样本
            break
    
    for idx in sample_indices:
        orig = original_bars[idx]
        adj = bars[idx]
        print(f"日期: {orig.date}")
        print(f"  原始: 开={orig.open:.2f}, 高={orig.high:.2f}, 低={orig.low:.2f}, 收={orig.close:.2f}")
        print(f"  复权: 开={adj.open:.2f}, 高={adj.high:.2f}, 低={adj.low:.2f}, 收={adj.close:.2f}")
        if abs(orig.close - adj.close) > 0.01:  # 如果有差异, 显示调整因子
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

    # 对比 get_cross_section_forward_adjusted_bars 与 datasets.bar 缓存
    print(f"\n=== 对比 get_cross_section_forward_adjusted_bars 与 datasets.bar 缓存 ===")
    
    # 获取 datasets.bar 缓存数据
    #from data.bar import load_bar
    cached_bars = load_bar(inst)
    
    if cached_bars:
        first_cached_date = cached_bars[0].date
        last_cached_date = cached_bars[-1].date
        print(f"datasets.bar 缓存日期范围: {first_cached_date} 到 {last_cached_date}")
        
        # 使用 get_cross_section_forward_adjusted_bars 获取相同日期范围的复权数据
        # 使用最后一条数据的日期作为截止日期
        adjusted_bars = get_cross_section_forward_adjusted_bars(inst, last_cached_date)
        
        if adjusted_bars and len(adjusted_bars) > 0:
            # 找到相同日期的第一条数据进行对比
            first_adjusted = None
            first_cached = cached_bars[0]
            
            for bar in adjusted_bars:
                if bar.date == first_cached.date:
                    first_adjusted = bar
                    break
            
            if first_adjusted:
                print(f"\n在 {first_cached.date} 的数据对比:")
                print(f"get_cross_section_forward_adjusted_bars:")
                print(f"  开盘: {first_adjusted.open:.4f}, 最高: {first_adjusted.high:.4f}, 最低: {first_adjusted.low:.4f}, 收盘: {first_adjusted.close:.4f}")
                print(f"  成交量: {first_adjusted.volume:.0f}, 成交额: {first_adjusted.amount:.0f}")
                
                print(f"datasets.bar 缓存:")
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
                    print(f"  get_cross_section_forward_adjusted_bars: {getattr(first_adjusted, 'adjustment_count', 'N/A')}")
                    print(f"  datasets.bar: {getattr(first_cached, 'adjustment_count', 'N/A')}")
            else:
                print(f"get_cross_section_forward_adjusted_bars 中找不到日期 {first_cached.date} 的数据")
                print(f"adjusted_bars 长度: {len(adjusted_bars)}")
                if len(adjusted_bars) > 0:
                    print(f"第一条: {adjusted_bars[0].date}, 最后一条: {adjusted_bars[-1].date}")
        else:
            print("get_cross_section_forward_adjusted_bars 返回空数据")
    else:
        print("datasets.bar 缓存为空")