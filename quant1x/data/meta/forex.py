import requests
import csv
import os
import bisect
from typing import Dict, List
from dataclasses import dataclass
from datetime import datetime

from quant1x.data import cache
from quant1x.log import logger
from quant1x.data.meta.timestamp import Timestamp
from ...config.config import base_config as config
from ..storage.storage import MetaFileStorage
from ..meta.region import Region

_currencies = ["CNY", "HKD", "USD", "EUR", "GBP", "SGD", "JPY"]

class ExchangeRateCache(MetaFileStorage):
    """
    汇率缓存, Foreign Exchange(FX) Rate Cache
    """
    EARLIEST_DATE = "1999-01-04"
    """最早日期"""
    
    def __init__(self, currency: str = Region.CN.currency):
        self.currency = currency.upper()
        """货币名称"""
        self.fields = ['date']
        """字段名称列表"""
        for c in _currencies:
            if c != self.currency:
                self.fields.append(c.upper())
        self.data : Dict[str, Dict[str, float]] = {}
        """汇率数据"""
        self.sorted_dates: List[str] = []
        """已排序的日期列表"""
        self._load_cache()
    
    def file_name(self) -> str:
        return f'{config.meta_path}/forex_{self.currency.lower()}.csv'
    
    def should_initialize(self, timestamp: Timestamp = Timestamp.now()) -> bool:
        """
        检查当前是否需要初始化文件
        
        Args:
            timestamp (Timestamp): 可选的时间戳参数，默认为当前时间
        
        Returns:
            bool: 如果满足以下条件则返回True:
                1. 当前时间大于等于今日初始化时间戳
                2. 文件修改时间早于今日初始化时间戳
        """
        fname = self.file_name()
        if not os.path.exists(fname):
            return True
        now_time = Timestamp.now()
        today_threshold = cache.get_today_initialized_time()
        file_modtime = cache.get_filename_modified_time(fname)
        if file_modtime.is_empty():
            return True
        #logger.debug(f"should_initialize: now_time={now_time}, today_init_timestamp={today_init_timestamp}, file_modtime={file_modtime}")
        is_stale = now_time >= today_threshold and file_modtime < today_threshold
        return is_stale
    
    def should_update(self, timestamp: Timestamp =Timestamp.now()) -> bool:
        return True
    
    def _load_cache(self):
        """
        加载 CSV 缓存
        """
        fname = self.file_name()
        create_or_update = self.should_initialize()
        if create_or_update:
            logger.info(f"汇率缓存: {fname}, 缓存需要更新, 开始更新")
            self.update()
            logger.info(f"汇率缓存: {fname}, 缓存需要更新, 更新完成")
        if not os.path.exists(fname):
            return
        with open(fname, 'r', encoding='utf-8', newline='') as f:
            reader = csv.DictReader(f)
            header = reader.fieldnames or []
            self.fields = [h if h.lower() == 'date' else h.upper() for h in header]
            for row in reader:
                # normalize each row key to uppercase except date
                normalized = {'date': row.get('date', '')}
                for k, v in row.items():
                    if k.lower() == 'date':
                        continue
                    normalized[k.upper()] = float('NaN') if v.strip() == '' else float(v)
                self.data[normalized['date']] = normalized
        self.sorted_dates = sorted(self.data.keys())
    
    def _save_cache(self):
        """保存 CSV 缓存（按日期排序）"""
        with open(self.file_name(), 'w', encoding='utf-8', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(self.fields)
            for date in self.sorted_dates:
                entry = self.data.get(date, {})
                row = [date]
                for field in self.fields[1:]:
                    # use get() to avoid KeyError if a rate is missing
                    row.append(entry.get(field, "NaN"))
                writer.writerow(row)
    
    def get_rate(self, date: str, offset: int = 0) -> Dict[str, float]:
        """
        获取汇率: 二分查找 ≤ target_date 的最大日期
        时间复杂度: O(log n)
        """
        if not self.sorted_dates:
            raise RuntimeError("缓存为空，请先调用 fetch_all_history()")
        
        date = date.strip()
        
        # 二分查找: 找到插入位置
        idx = bisect.bisect_right(self.sorted_dates, date) - 1
        
        # 边界: 早于最早数据 → 返回最早日期汇率
        if idx < 0:
            idx = 0
        # 边界: 晚于最新数据 → 返回最新日期汇率
        elif idx >= len(self.sorted_dates) - 1:
            idx = len(self.sorted_dates) - 1
        # 确定日期索引
        date_idx = self.sorted_dates[idx]
        
        # 返回找到的日期汇率
        return self.data[date_idx]
    
    def get_rates_batch(self, dates: list) -> dict:
        """批量获取汇率"""
        return {date: self.get_rate(date) for date in dates}
    
    def fetch_all_history(self, start_date: str = "1999-01-04", end_date: str = None):
        """获取全部历史数据"""
        if end_date is None:
            end_date = datetime.now().strftime('%Y-%m-%d')
        
        logger.debug(f"🔄 获取 {start_date} 至 {end_date}...")
        url = f"https://api.frankfurter.app/{start_date}..{end_date}"
        #params = {"from": "USD", "to": "HKD"}
        params = {"base": self.currency, "symbols": ",".join(self.fields[1:])}
        resp = requests.get(url, params=params, timeout=120)
        resp.raise_for_status()
        data = resp.json()
        
        rates = data.get('rates', {})
        # force currency codes to upper-case so keys are consistent
        self.data = {
            date: {k.upper(): float(v) for k, v in rates[date].items()}
            for date in rates
        }
        self.sorted_dates = sorted(self.data.keys())
        self._save_cache()
        
        logger.debug(f"✅ {len(self.data)} 条记录 | {self.sorted_dates[0]} ~ {self.sorted_dates[-1]}")
    
    def update(self, start_date: str = "1999-01-04", end_date: str = None):
        """增量更新"""
        if self.sorted_dates:
            latest = self.sorted_dates[-1]
            if latest > start_date:
                start_date = latest
        
        if end_date is None:
            end_date = datetime.now().strftime('%Y-%m-%d')
        
        if start_date >= end_date:
            return
        
        logger.debug(f"🔄 更新 {start_date} 至 {end_date}")
        url = f"https://api.frankfurter.app/{start_date}..{end_date}"
        params = {"base": self.currency, "symbols": ",".join(self.fields[1:])}
        resp = requests.get(url, params, timeout=60)
        resp.raise_for_status()
        data = resp.json()
        
        rates = data.get('rates', {})
        new_data = {
            date: {k.upper(): float(v) for k, v in rates[date].items()}
            for date in rates
        }
        self.data.update(new_data)
        self.sorted_dates = sorted(self.data.keys())
        self._save_cache()
        logger.debug(f"✅ 共 {len(self.data)} 条记录")


# ============================================
# 使用示例
# ============================================
if __name__ == "__main__":
    rate_cache = ExchangeRateCache("HKD")
    
    # # 首次运行获取全量数据
    # if not rate_cache.sorted_dates:
    #     rate_cache.fetch_all_history()
    # else:
    #     rate_cache.update()
    
    # 查询测试
    test_dates = [
        "2026-01-25",  # 周日 → 用周五
        "2026-01-20",  # 节假日 → 用前一交易日
        "2026-02-17",  # 交易日 → 精确匹配
        "1999-01-01",  # 早于最早 → 用最早
        "2099-12-31",  # 晚于最新 → 用最新
    ]
    
    print("\n汇率查询测试:")
    for date in test_dates:
        rate = rate_cache.get_rate(date)
        print(rate)
        print(f"  {date} → {rate['CNY']}")
    
    # # 港股分红复权
    # dividends = [
    #     {"symbol": "00700.HK", "ex_date": "2026-01-25", "dividend_usd": 0.50},
    #     {"symbol": "00941.HK", "ex_date": "2026-02-14", "dividend_usd": 1.20},
    # ]
    
    # print("\n分红复权计算:")
    # for item in dividends:
    #     rate = cache.get_rate(item["ex_date"])
    #     hkd = item["dividend_usd"] * rate
    #     print(f"{item['symbol']} | {item['ex_date']} | {item['dividend_usd']:.2f} USD × {rate:.4f} = {hkd:.4f} HKD")