# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
import pandas as pd
from dataclasses import dataclass
from typing import List, Optional
from quant1x.contrib.data.tdx.instruments import get_instrument_info
from quant1x.log import logger

from quant1x.config import config
from quant1x.data.meta import Timestamp
from quant1x.data.schema import Bar
from quant1x.data import adapter, MaxCachedDaysToDropOnIncrementalUpdate
from . import protocol
from .client import get_std_conn
from .level1 import SecurityBarsRequest, SecurityBarsResponse, KLineType, SECURITY_BARS_PRE_REQUEST_MAX
from quant1x.data.adapter import DataAdapter, PLUGIN_MASK_BASEDATA_DATA, register, DEFAULT_DATA_PROVIDER
from quant1x.data.base import BASEDATA_RAW_DAILY_KLINE, MarketCnFirstListTime
from quant1x.data.meta import Instrument, Frequency, TimeUnit, FREQ_DAILY
from quant1x.data.market import detect_symbol

def frequency_to_kline_type(freq: Frequency) -> KLineType:
    """
    将时间频率转换为对应的K线类型
    
    Args:
        freq (Frequency): 时间频率对象，包含单位和数值
    
    Returns:
        KLineType: 对应的K线类型枚举值
    
    Raises:
        ValueError: 当传入不支持的频率时抛出
    """
    if freq.unit == TimeUnit.MINUTE:
        if freq.num == 1:
            return KLineType._1MIN
        elif freq.num == 5:
            return KLineType._5MIN
        elif freq.num == 15:
            return KLineType._15MIN
        elif freq.num == 30:
            return KLineType._30MIN
    elif freq.unit == TimeUnit.HOUR:
        if freq.num == 1:
            return KLineType._1HOUR
    elif freq.unit == TimeUnit.DAY:
        if freq.num == 1:
            return KLineType.DAILY
    elif freq.unit == TimeUnit.WEEK:
        if freq.num == 1:
            return KLineType.WEEKLY
    elif freq.unit == TimeUnit.MONTH:
        if freq.num == 1:
            return KLineType.MONTHLY
        elif freq.num == 3:
            return KLineType._3MONTH
        elif freq.num == 12:
            return KLineType.YEARLY

    raise ValueError(f"unsupported frequency: {freq}")

def validate_csv_columns(df: pd.DataFrame, required_columns: List[str],
                        filename: str = "", strict_order: bool = False) -> bool:
    """
    通用CSV列名验证函数

    Args:
        df: pandas DataFrame
        required_columns: 必需的列名列表
        filename: 文件名（用于日志）
        strict_order: 是否要求列的顺序严格匹配

    Returns:
        bool: 验证是否通过
    """
    # 清理列名（去除空格），但保持原始大小写
    actual_columns = clean_column_names(df)

    # 方法1: 检查必需列是否存在
    missing_cols = [col for col in required_columns if col not in actual_columns]
    if missing_cols:
        logger.error(f"CSV文件 {filename} 缺少必需列: {missing_cols}")
        logger.error(f"实际列名: {actual_columns}")
        logger.error(f"期望列名: {required_columns}")
        return False

    # 方法2: 检查列顺序（可选）
    if strict_order and actual_columns != required_columns:
        logger.warning(f"CSV文件 {filename} 列名顺序不匹配")
        logger.warning(f"期望顺序: {required_columns}")
        logger.warning(f"实际顺序: {actual_columns}")
        return False

    # 方法3: 检查额外列
    extra_cols = [col for col in actual_columns if col not in required_columns]
    if extra_cols:
        logger.info(f"CSV文件 {filename} 包含额外列: {extra_cols}")

    return True

def clean_column_names(df: pd.DataFrame) -> List[str]:
    """
    清理列名：只去除空格和不可见字符，保持原始大小写格式

    Args:
        df: 输入DataFrame

    Returns:
        清理后的列名列表
    """
    return df.columns.str.strip().tolist()

@dataclass
class BarRaw:
    date: str = ""
    open: float = 0.0
    close: float = 0.0
    high: float = 0.0
    low: float = 0.0
    volume: float = 0.0
    amount: float = 0.0
    up: int = 0
    down: int = 0
    timestamp: str = ""

    @staticmethod
    def headers() -> List[str]:
        return ["date", "open", "close", "high", "low", "volume", "amount", "up", "down", "timestamp"]

    def adjust(self, factor):
        """
        根据复权因子调整K线价格数据
        
        Args:
            factor: CumulativeAdjustment对象，包含复权因子
        """
        if hasattr(factor, 'apply'):
            self.open = factor.apply(self.open)
            self.close = factor.apply(self.close)
            self.high = factor.apply(self.high)
            self.low = factor.apply(self.low)
        else:
            # 如果factor没有apply方法，假设它有m和a属性
            self.open = self.open * factor.m + factor.a
            self.close = self.close * factor.m + factor.a
            self.high = self.high * factor.m + factor.a
            self.low = self.low * factor.m + factor.a

def save_kline_raw(filename: str, values: List[BarRaw]):
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
            "timestamp": v.timestamp
        }
        for v in values
    ]
    
    df = pd.DataFrame(data, columns=BarRaw.headers())
    df.to_csv(filename, index=False)

def read_kline_raw_from_csv(filename: str) -> List[BarRaw]:
    """
    从CSV文件读取K线原始数据, 包含完整的列名验证
    """
    klines = []
    if not os.path.exists(filename):
        return klines

    try:
        df = pd.read_csv(filename)

        # 使用通用列名验证函数，默认保持原始格式
        required_cols = BarRaw.headers()
        if not validate_csv_columns(df, required_cols, filename, strict_order=True):
            return klines

        # 可选：如果需要处理列名问题，可以启用清理
        # df = clean_column_names(df)

        # 数据类型验证和转换
        for _, row in df.iterrows():
            try:
                kline = BarRaw(
                    date=str(row['date']),
                    open=float(row['open']),
                    close=float(row['close']),
                    high=float(row['high']),
                    low=float(row['low']),
                    volume=float(row['volume']),
                    amount=float(row['amount']),
                    up=int(row['up']),
                    down=int(row['down']),
                    timestamp=str(row['timestamp'])
                )
                klines.append(kline)
            except (ValueError, TypeError) as e:
                logger.warning(f"跳过无效数据行: {e}")

    except Exception as e:
        logger.error(f"读取K线原始数据CSV文件失败 {filename}: {e}")

    return klines

def get_kline_raw_filename(inst: Instrument, freq: Frequency=FREQ_DAILY) -> str:
    module_name = freq.cache_key()
    symbol = inst.symbol()
    symbol_path = symbol[:-3]
    return f'{config.data_path}/{module_name}/{symbol_path}/{symbol}.raw' 

def load_kline_raw(inst: Instrument, freq: Frequency=FREQ_DAILY) -> List[BarRaw]:
    """
    从缓存文件加载指定证券代码的K线原始数据
    
    Args:
        code (str): 证券代码
        
    Returns:
        List[BarRaw]: K线原始数据列表
    """
    cache_filename = get_kline_raw_filename(inst, freq)
    return read_kline_raw_from_csv(cache_filename)

def ensure_kline_raw_updated(inst: Instrument, freq: Frequency=FREQ_DAILY):
    """
    确保指定证券代码的K线原始数据是最新的
    
    Args:
        code (str): 证券代码
    """
    # 使用DataKLineRaw的update方法来确保数据是最新的
    data_adapter = DataKLineRaw()
    data_adapter.update(inst)

def checkout_kline_raw(inst: Instrument, freq: Frequency=FREQ_DAILY) -> List[BarRaw]:
    """
    获取指定证券的未复权K线数据，如果数据不存在则下载
    
    Args:
        code (str): 证券代码
        
    Returns:
        List[BarRaw]: 未复权K线数据列表
    """
    # 确保数据是最新的
    ensure_kline_raw_updated(inst, freq)
    # 从缓存加载数据
    return load_kline_raw(inst, freq)

def fetch_kline_raw(inst: Instrument, start: int, count: int, freq: Frequency) -> list[Bar]:
    try:
        kline_type = frequency_to_kline_type(freq)
        with get_std_conn() as conn:
            req = SecurityBarsRequest(inst.exchange, inst.ticker, kline_type, start, count)
            resp = SecurityBarsResponse(inst.type.is_index(), kline_type)
            protocol.process(conn, req, resp)
            return resp.list
    except Exception as e:
        logger.error(f"[basedata::KLine] fetch_kline_raw error: {e}")
        return []

class DataKLineRaw(DataAdapter):
    def kind(self) -> int:
        return BASEDATA_RAW_DAILY_KLINE  # 基础数据-未复权K线

    def owner(self) -> str:
        return DEFAULT_DATA_PROVIDER

    def key(self) -> str:
        return "day_raw"

    def name(self) -> str:
        return "日K线RAW"

    def usage(self) -> str:
        return "日K线RAW数据适配器"

    def print(self, inst: Instrument, date: Optional[Timestamp] = None) -> None:
        """控制台打印K线数据"""
        klines = checkout_kline_raw(inst)
        if not klines:
            print(f"No kline data found for {code}")
            return
            
        print(f"K线数据 for {code}:")
        for kline in klines[-10:]:  # 显示最近10条
            print(f"  {kline.date}: O:{kline.open:.2f} H:{kline.high:.2f} L:{kline.low:.2f} C:{kline.close:.2f} V:{kline.volume:.0f}")

    def update(self, inst: Instrument, date: Optional[Timestamp] = None) -> None:
        symbol = inst.symbol()
        # 1. Determine start date from local cache
        current_start_date = Timestamp.parse(MarketCnFirstListTime)  # market_first_date
        freq = Frequency(num=1, unit=TimeUnit.DAY)
        cache_filename = get_kline_raw_filename(inst, freq)
        cache_klines = read_kline_raw_from_csv(cache_filename)
        
        klines_length = len(cache_klines)
        klines_offset_days = MaxCachedDaysToDropOnIncrementalUpdate
        
        if klines_length > 0:
            if klines_offset_days > klines_length:
                klines_offset_days = klines_length
            
            kline = cache_klines[klines_length - klines_offset_days]
            current_start_date = Timestamp.parse(kline.date)
        
        # 2. Determine end date
        current_end_date = Timestamp.now().get_pre_market_time()
        logger.debug(f"[basedata::BarRaw] [{symbol}]: from {current_start_date.only_date()} to {current_end_date.only_date()}")
        
        step = SECURITY_BARS_PRE_REQUEST_MAX
        start = 0
        hs: List[List[Bar]] = []
        element_count = 0
        
        while True:
            count = step
            reply = fetch_kline_raw(inst, start, count, freq)
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
        
        incremental_klines: List[BarRaw] = []
        
        for vec in hs:
            for row in vec:
                date_time = Timestamp.parse(row.date).get_pre_market_time()
                if date_time < current_start_date or date_time > current_end_date:
                    continue
                    
                kx = BarRaw(
                    date=date_time.only_date(),
                    open=row.open,
                    close=row.close,
                    high=row.high,
                    low=row.low,
                    volume=row.volume * 100,  # Convert to shares
                    amount=row.amount,
                    up=row.up,
                    down=row.down,
                    timestamp=row.timestamp
                )
                incremental_klines.append(kx)
                
        # 7. Merge
        klines = []
        if klines_length > klines_offset_days:
            klines.extend(cache_klines[:klines_length - klines_offset_days])
            
        klines.extend(incremental_klines)
        
        # 9. Save
        save_kline_raw(cache_filename, klines)


# 自动注册DataKLineRaw插件
from quant1x.data.adapter import register

# 创建并注册DataKLineRaw插件
_data_kline_raw_plugin = adapter.register(DataKLineRaw)

if __name__ == "__main__":
    # Example usage
    code = "sh600004"
    inst = detect_symbol(code)
    symbol = inst.symbol()
    inst = get_instrument_info(symbol)
    print(inst)
    klines = checkout_kline_raw(inst)
    print(f"Loaded {len(klines)} kline records for {code}")
    if klines:
        # 显示最近5条记录
        print("Recent 5 records:")
        for kline in klines[-5:]:
            print(f"  {kline.date}: O:{kline.open:.2f} H:{kline.high:.2f} L:{kline.low:.2f} C:{kline.close:.2f} V:{kline.volume:.0f}")