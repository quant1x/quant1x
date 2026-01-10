# -*- coding: UTF-8 -*-
import os
import pandas as pd
from dataclasses import dataclass
from typing import List, Optional
import logging

from quant1x.exchange import Timestamp
from quant1x.level1 import protocol
from quant1x.level1.client import get_std_conn
from quant1x.level1.security_bars import SecurityBarsRequest, SecurityBarsResponse, KLineType, SecurityBar, SECURITY_BARS_MAX
from quant1x.config import config
from quant1x.data import adapter
from quant1x.data.adapter import DataAdapter, PLUGIN_MASK_BASE_DATA, register, DEFAULT_DATA_PROVIDER
from quant1x.data.base import BASE_RAW_DAILY_KLINE

logger = logging.getLogger(__name__)

MAX_KLINE_LOOKBACK_DAYS = 1

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
class KLineRaw:
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

    @staticmethod
    def headers() -> List[str]:
        return ["date", "open", "close", "high", "low", "volume", "amount", "up", "down", "datetime"]

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

def save_kline_raw(filename: str, values: List[KLineRaw]):
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
            "datetime": v.datetime
        }
        for v in values
    ]
    
    df = pd.DataFrame(data, columns=KLineRaw.headers())
    df.to_csv(filename, index=False)

def read_kline_raw_from_csv(filename: str) -> List[KLineRaw]:
    """
    从CSV文件读取K线原始数据，包含完整的列名验证
    """
    klines = []
    if not os.path.exists(filename):
        return klines

    try:
        df = pd.read_csv(filename)

        # 使用通用列名验证函数，默认保持原始格式
        required_cols = KLineRaw.headers()
        if not validate_csv_columns(df, required_cols, filename, strict_order=True):
            return klines

        # 可选：如果需要处理列名问题，可以启用清理
        # df = clean_column_names(df)

        # 数据类型验证和转换
        for _, row in df.iterrows():
            try:
                kline = KLineRaw(
                    date=str(row['date']),
                    open=float(row['open']),
                    close=float(row['close']),
                    high=float(row['high']),
                    low=float(row['low']),
                    volume=float(row['volume']),
                    amount=float(row['amount']),
                    up=int(row['up']),
                    down=int(row['down']),
                    datetime=str(row['datetime'])
                )
                klines.append(kline)
            except (ValueError, TypeError) as e:
                logger.warning(f"跳过无效数据行: {e}")

    except Exception as e:
        logger.error(f"读取K线原始数据CSV文件失败 {filename}: {e}")

    return klines

def load_kline_raw(code: str) -> List[KLineRaw]:
    """
    从缓存文件加载指定证券代码的K线原始数据
    
    Args:
        code (str): 证券代码
        
    Returns:
        List[KLineRaw]: K线原始数据列表
    """
    cache_filename = config.get_kline_filename(code, forward=False)
    return read_kline_raw_from_csv(cache_filename)

def ensure_kline_raw_updated(code: str):
    """
    确保指定证券代码的K线原始数据是最新的
    
    Args:
        code (str): 证券代码
    """
    # 使用DataKLineRaw的update方法来确保数据是最新的
    data_adapter = DataKLineRaw()
    data_adapter.update(code)

def checkout_kline_raw(code: str) -> List[KLineRaw]:
    """
    获取指定证券的未复权K线数据，如果数据不存在则下载
    
    Args:
        code (str): 证券代码
        
    Returns:
        List[KLineRaw]: 未复权K线数据列表
    """
    # 确保数据是最新的
    ensure_kline_raw_updated(code)
    # 从缓存加载数据
    return load_kline_raw(code)

def fetch_kline(code: str, start: int, count: int, kline_type: KLineType = KLineType.DAILY) -> List[SecurityBar]:
    try:
        with get_std_conn() as conn:
            req = SecurityBarsRequest(code, kline_type.value, start, count)
            resp = SecurityBarsResponse(req.is_index, kline_type.value)
            protocol.process(conn, req, resp)
            return resp.list
    except Exception as e:
        logger.error(f"[dataset::KLineRaw] fetch_kline error: {e}")
        return []

class DataKLineRaw(DataAdapter):
    def kind(self) -> int:
        return BASE_RAW_DAILY_KLINE  # 基础数据-未复权K线

    def owner(self) -> str:
        return DEFAULT_DATA_PROVIDER

    def key(self) -> str:
        return "day_raw"

    def name(self) -> str:
        return "日K线RAW"

    def usage(self) -> str:
        return "日K线RAW数据适配器"

    def print(self, code: str, dates: Optional[List[Timestamp]] = None) -> None:
        """控制台打印K线数据"""
        klines = checkout_kline_raw(code)
        if not klines:
            print(f"No kline data found for {code}")
            return
            
        print(f"K线数据 for {code}:")
        for kline in klines[-10:]:  # 显示最近10条
            print(f"  {kline.date}: O:{kline.open:.2f} H:{kline.high:.2f} L:{kline.low:.2f} C:{kline.close:.2f} V:{kline.volume:.0f}")

    def update(self, code: str, date: Optional[Timestamp] = None) -> None:
        # 1. Determine start date from local cache
        current_start_date = Timestamp.parse("1990-12-19")  # market_first_date
        cache_filename = config.get_kline_filename(code, forward=False)
        cache_klines = read_kline_raw_from_csv(cache_filename)
        
        klines_length = len(cache_klines)
        klines_offset_days = MAX_KLINE_LOOKBACK_DAYS
        
        if klines_length > 0:
            if klines_offset_days > klines_length:
                klines_offset_days = klines_length
            
            kline = cache_klines[klines_length - klines_offset_days]
            current_start_date = Timestamp.parse(kline.date)
        
        # 2. Determine end date
        current_end_date = Timestamp.now().get_pre_market_time()
        logger.debug(f"[dataset::KLineRaw] [{code}]: from {current_start_date.only_date()} to {current_end_date.only_date()}")
        
        step = SECURITY_BARS_MAX
        start = 0
        hs: List[List[SecurityBar]] = []
        element_count = 0
        
        while True:
            count = step
            reply = fetch_kline(code, start, count)
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
        
        incremental_klines: List[KLineRaw] = []
        
        for vec in hs:
            for row in vec:
                date_time = Timestamp.parse(f"{row.Year}-{row.Month:02d}-{row.Day:02d}").get_pre_market_time()
                
                if date_time < current_start_date or date_time > current_end_date:
                    continue
                    
                kx = KLineRaw(
                    date=date_time.only_date(),
                    open=row.Open,
                    close=row.Close,
                    high=row.High,
                    low=row.Low,
                    volume=row.Vol * 100,  # Convert to shares
                    amount=row.Amount,
                    up=row.UpCount,
                    down=row.DownCount,
                    datetime=row.DateTime
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
from data.adapter import register

# 创建并注册DataKLineRaw插件
_data_kline_raw_plugin = adapter.register(DataKLineRaw)

if __name__ == "__main__":
    # Example usage
    code = "sh600000"
    klines = checkout_kline_raw(code)
    print(f"Loaded {len(klines)} kline records for {code}")
    if klines:
        # 显示最近5条记录
        print("Recent 5 records:")
        for kline in klines[-5:]:
            print(f"  {kline.date}: O:{kline.open:.2f} H:{kline.high:.2f} L:{kline.low:.2f} C:{kline.close:.2f} V:{kline.volume:.0f}")