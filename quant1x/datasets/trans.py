# -*- coding: UTF-8 -*-
import logging
import os
import threading
import csv
from dataclasses import dataclass
from typing import List, Tuple

from quant1x.level1 import client, protocol
from quant1x.level1 import transaction
from quant1x.level1.transaction import TickTransaction
from quant1x import exchange
from quant1x.exchange import Timestamp
from quant1x.config import config
from quant1x.factors import f10
from quant1x.cache import trains_begin_date
from quant1x import numerics

log = logging.getLogger(__name__)

# Constants
HistoricalTransactionDataFirstTime = "09:25"
HistoricalTransactionDataStartTime = "09:30"
HistoricalTransactionDataFinalBiddingTime = "14:57"
HistoricalTransactionDataLastTime = "15:00"

@dataclass
class TurnoverDataSummary:
    OuterVolume: int = 0
    OuterAmount: float = 0.0
    InnerVolume: int = 0
    InnerAmount: float = 0.0
    OpenVolume: int = 0
    OpenTurnZ: float = 0.0
    CloseVolume: int = 0
    CloseTurnZ: float = 0.0

    def __str__(self):
        return (f"OuterVolume: {self.OuterVolume} OuterAmount: {self.OuterAmount} "
                f"InnerVolume: {self.InnerVolume} InnerAmount: {self.InnerAmount} "
                f"OpenVolume: {self.OpenVolume} OpenTurnZ: {self.OpenTurnZ} "
                f"CloseVolume: {self.CloseVolume} CloseTurnZ: {self.CloseTurnZ}")

# Global state for historical trading data begin date
_historical_trading_data_once = False
_historical_trading_data_mutex = threading.Lock()
_historical_trading_data_begin_date = Timestamp.parse(trains_begin_date)

def lazy_init_historical_trading_data():
    """
    初始化历史交易数据的开始日期
    
    该函数将全局变量 _historical_trading_data_begin_date 设置为从 trains_begin_date 解析得到的时间戳
    
    Note:
        这是一个惰性初始化函数，只在首次调用时执行初始化
    """
    global _historical_trading_data_begin_date
    _historical_trading_data_begin_date = Timestamp.parse(trains_begin_date)

def get_begin_date_of_historical_trading_data() -> Timestamp:
    """
    获取历史交易数据的起始日期
    
    该函数使用双重检查锁模式确保历史交易数据只被初始化一次，
    并返回初始化后的起始日期。
    
    Returns:
        Timestamp: 历史交易数据的起始日期
    """
    global _historical_trading_data_once
    if not _historical_trading_data_once:
        with _historical_trading_data_mutex:
            if not _historical_trading_data_once:
                lazy_init_historical_trading_data()
                _historical_trading_data_once = True
    return _historical_trading_data_begin_date

def update_begin_date_of_historical_trading_data(date: str):
    """
    更新历史交易数据的起始日期
    
    Args:
        date (str): 新的起始日期字符串，格式应符合Timestamp.parse的要求
    
    Note:
        - 该函数会确保历史交易数据的延迟初始化已完成
        - 如果日期解析失败，函数会静默处理异常而不抛出
        - 操作是线程安全的，使用互斥锁保护
    """
    get_begin_date_of_historical_trading_data()
    
    with _historical_trading_data_mutex:
        try:
            dt = Timestamp.parse(date)
            global _historical_trading_data_begin_date
            _historical_trading_data_begin_date = dt
        except Exception:
            pass

def restore_begin_date_of_historical_trading_data():
    """
    将历史交易数据的开始日期恢复为初始训练开始日期
    
    该函数通过调用update_begin_date_of_historical_trading_data函数，
    将历史交易数据的开始日期重置为预设的训练开始日期(trains_begin_date)
    
    Note:
        函数内部使用了模块级变量trains_begin_date作为恢复的目标日期
    """
    update_begin_date_of_historical_trading_data(trains_begin_date)

def load_transaction_data_from_cache(corrected_code: str, feature_date: Timestamp, ignore_previous_data: bool) -> Tuple[List[transaction.TickTransaction], str]:
    """
    从缓存文件加载指定证券代码在特定日期的逐笔交易数据
    
    Args:
        corrected_code (str): 证券代码(已校正)
        feature_date (Timestamp): 查询日期
        ignore_previous_data (bool): 是否忽略历史数据
        
    Returns:
        Tuple[List[transaction.TickTransaction], str]: 
            返回两个值：
            1. 交易数据列表，每个元素为TickTransaction对象
            2. 数据起始时间字符串(用于增量更新)
            
    Raises:
        不直接抛出异常，但会在日志中记录错误信息
    """
    trade_date = feature_date.cache_date()
    
    if ignore_previous_data:
        start_date = get_begin_date_of_historical_trading_data()
        if feature_date.cache_date() < start_date.cache_date():
            log.error(f"[dataset::trans] code={corrected_code}, trade-date={trade_date}, start-date={start_date.to_string()}, 没有数据")
            return [], HistoricalTransactionDataFirstTime

    start_time = HistoricalTransactionDataFirstTime
    filename = config.get_historical_trade_filename(corrected_code, feature_date.only_date())
    
    data_list = []
    
    if os.path.exists(filename):
        try:
            with open(filename, 'r', encoding='utf-8') as f:
                reader = csv.DictReader(f)
                # Expected headers: time, price, vol, num, amount, buyOrSell
                for row in reader:
                    t = transaction.TickTransaction(
                        time=row['time'],
                        price=float(row['price']),
                        vol=int(row['vol']),
                        num=int(row['num']),
                        amount=float(row['amount']),
                        buyOrSell=int(row['buyOrSell'])
                    )
                    data_list.append(t)
            
            if data_list:
                last_time = data_list[-1].time
                if last_time == HistoricalTransactionDataLastTime:
                    return data_list, start_time
                
                first_time = ""
                skip_count = 0
                cache_length = len(data_list)
                for i in range(1, cache_length + 1):
                    tm = data_list[cache_length - i].time
                    if not first_time:
                        first_time = tm
                        start_time = first_time
                        skip_count += 1
                        continue
                    
                    if tm < first_time:
                        start_time = first_time
                        break
                    else:
                        skip_count += 1
                
                if skip_count > 0:
                    data_list = data_list[:-skip_count]
                    
        except Exception as e:
            log.error(f"[dataset::trans] Read cache failed: {e}")
            
    return data_list, start_time

def update_transaction_data(corrected_code: str, feature_date: Timestamp, start_time: str):
    """
    更新指定证券代码在特定日期的交易数据，并将其保存到CSV文件中
    
    Args:
        corrected_code (str): 证券代码
        feature_date (Timestamp): 交易日期
        start_time (str): 开始时间(格式为HH:MM:SS)
    
    Raises:
        Exception: 当网络请求或文件操作失败时记录错误日志
    """
    trade_date_str = feature_date.cache_date()
    trade_date_int = int(trade_date_str)
    
    # Check if today is last trading day
    today_is_last_trading_date = feature_date.is_same_date(Timestamp.parse(exchange.last_trade_date()))
    
    offset = transaction.TICK_TRANSACTION_MAX
    start = 0
    history: List[transaction.TickTransaction] = []
    hs: List[List[transaction.TickTransaction]] = []
    
    market_id, market_code, pure_code = exchange.detect_market(corrected_code)
    market_val = market_id.value if hasattr(market_id, 'value') else market_id
    
    if today_is_last_trading_date:
        while True:
            try:
                req = transaction.TransactionRequest(corrected_code, start, offset)
                
                with client.client() as conn:
                    sock = conn.socket
                    resp = transaction.TransactionResponse(market_val, pure_code)
                    protocol.process(sock, req, resp)
                
                if resp.count == 0 or not resp.list:
                    break
                    
                tmp = []
                tmp_list = list(resp.list)
                tmp_list.reverse()
                
                for td in tmp_list:
                    if td.time >= start_time:
                        tmp.append(td)
                
                tmp.reverse()
                hs.append(tmp)
                
                if len(tmp) < offset:
                    break
                
                start += offset
            except Exception as e:
                log.error(f"[dataset::trans] code={corrected_code}, tradeDate={trade_date_str}, error={e}")
                break
    else:
        while True:
            try:
                req = transaction.HistoryTransactionRequest(corrected_code, trade_date_int, start, offset)
                
                with client.client() as conn:
                    sock = conn.socket
                    resp = transaction.HistoryTransactionResponse(market_val, pure_code)
                    protocol.process(sock, req, resp)
                
                if resp.count == 0 or not resp.list:
                    break
                    
                tmp = []
                tmp_list = list(resp.list)
                tmp_list.reverse()
                
                for td in tmp_list:
                    if td.time >= start_time:
                        tmp.append(td)
                
                tmp.reverse()
                hs.append(tmp)
                
                if len(tmp) < offset:
                    break
                
                start += offset
            except Exception as e:
                log.error(f"[dataset::trans] code={corrected_code}, tradeDate={trade_date_str}, error={e}")
                break

    # Reverse hs
    hs.reverse()
    for v in hs:
        history.extend(v)
        
    if not history:
        return

    # Load existing and merge
    existing_list, _ = load_transaction_data_from_cache(corrected_code, feature_date, False)
    existing_list.extend(history)
    
    # Write to CSV
    filename = config.get_historical_trade_filename(corrected_code, feature_date.only_date())
    tmp_filename = filename + ".tmp"
    
    try:
        os.makedirs(os.path.dirname(filename), exist_ok=True)
        with open(tmp_filename, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(["time", "price", "vol", "num", "amount", "buyOrSell"])
            for rec in existing_list:
                writer.writerow([rec.time, rec.price, rec.vol, rec.num, rec.amount, rec.buyOrSell])
        
        os.replace(tmp_filename, filename)
    except Exception as e:
        log.error(f"[dataset::trans] Rename failed: {tmp_filename} -> {filename}: {e}")
        if os.path.exists(tmp_filename):
            os.remove(tmp_filename)

def ensure_transaction_data_updated(corrected_code: str, feature_date: Timestamp, ignore_previous_data: bool):
    """
    确保指定证券代码在特定日期的交易数据是最新的
    
    Args:
        corrected_code (str): 证券代码(已修正格式)
        feature_date (Timestamp): 需要检查的日期
        ignore_previous_data (bool): 是否忽略之前缓存的数据
    
    Raises:
        DataUpdateError: 当数据更新失败时抛出
    """
    data_list, start_time = load_transaction_data_from_cache(corrected_code, feature_date, ignore_previous_data)
    needs_update = not data_list or (data_list[-1].time != HistoricalTransactionDataLastTime)
    if needs_update:
        update_transaction_data(corrected_code, feature_date, start_time)

def checkout_transaction_data(security_code: str, feature_date: Timestamp, ignore_previous_data: bool) -> List[transaction.TickTransaction]:
    """
    获取指定证券在特定日期的逐笔交易数据
    
    Args:
        security_code (str): 证券代码
        feature_date (Timestamp): 目标日期
        ignore_previous_data (bool): 是否忽略已有缓存数据，强制重新获取
    
    Returns:
        List[transaction.TickTransaction]: 返回指定证券在目标日期的逐笔交易数据列表
    
    Raises:
        ValueError: 如果证券代码无效或日期格式不正确
        DataNotAvailableError: 如果请求的数据不可用
    """
    corrected_code = exchange.correct_security_code(security_code)
    ensure_transaction_data_updated(corrected_code, feature_date, ignore_previous_data)
    data_list, _ = load_transaction_data_from_cache(corrected_code, feature_date, ignore_previous_data)
    return data_list

def count_inflow(data_list: List[transaction.TickTransaction], security_code: str, feature_date: Timestamp) -> TurnoverDataSummary:
    """
    计算指定证券在特定日期的资金流入流出情况
    
    Args:
        data_list (List[transaction.TickTransaction]): 该证券的逐笔交易数据列表
        security_code (str): 证券代码
        feature_date (Timestamp): 计算日期
    
    Returns:
        TurnoverDataSummary: 包含以下统计数据的对象:
            - OuterVolume: 外盘成交量
            - OuterAmount: 外盘成交金额  
            - InnerVolume: 内盘成交量
            - InnerAmount: 内盘成交金额
            - OpenVolume: 开盘成交量
            - CloseVolume: 收盘成交量
            - OpenTurnZ: 开盘换手率(万分之)
            - CloseTurnZ: 收盘换手率(万分之)
    
    Notes:
        - 根据买卖方向(TICK_BUY/TICK_SELL)区分内外盘
        - 对于中性方向交易，按成交量均分到内外盘
        - 开盘/收盘成交量根据时间范围统计
        - 换手率基于F10中的流通股本计算(万分之)
    """
    summary = TurnoverDataSummary()
    if not data_list:
        return summary
        
    corrected_code = exchange.correct_security_code(security_code)
    last_price = 0.0
    
    for v in data_list:
        tm = v.time
        direction = v.buyOrSell
        price = v.price
        
        if last_price == 0:
            last_price = price
            
        vol = v.vol
        
        if direction != transaction.TICK_BUY and direction != transaction.TICK_SELL:
            if price > last_price:
                direction = transaction.TICK_BUY
            elif price < last_price:
                direction = transaction.TICK_SELL
                
        if direction == transaction.TICK_BUY:
            summary.OuterVolume += vol
            summary.OuterAmount += float(vol) * price
        elif direction == transaction.TICK_SELL:
            summary.InnerVolume += vol
            summary.InnerAmount += float(vol) * price
        else:
            vn = vol
            buy_offset = vn // 2
            sell_offset = vn - buy_offset
            
            summary.OuterVolume += buy_offset
            summary.OuterAmount += float(buy_offset) * price
            summary.InnerVolume += sell_offset
            summary.InnerAmount += float(sell_offset) * price
            
        if HistoricalTransactionDataFirstTime <= tm < HistoricalTransactionDataStartTime:
            summary.OpenVolume += vol
            
        if HistoricalTransactionDataFinalBiddingTime < tm <= HistoricalTransactionDataLastTime:
            summary.CloseVolume += vol
            
        last_price = price
        
    # F10 TurnZ
    f10_data = f10.get_f10(corrected_code, feature_date.only_date())
    if f10_data:
        free_capital = f10_data.free_capital
        capital = f10_data.capital
        
        if free_capital == 0:
            free_capital = capital
            
        if abs(free_capital) > 1e-6:
            def calculate_turn_z(v):
                turnover_rate_z = numerics.change_rate(free_capital, v)
                turnover_rate_z *= 10000
                return numerics.decimal(turnover_rate_z)
                
            summary.OpenTurnZ = calculate_turn_z(float(summary.OpenVolume))
            summary.CloseTurnZ = calculate_turn_z(float(summary.CloseVolume))
        
    return summary

class DataTrans:
    def kind(self):
        return "BaseTransaction"
        
    def owner(self):
        return "default"
        
    def key(self):
        return "trans"
        
    def name(self):
        return "历史成交"
        
    def usage(self):
        return "历史成交"
        
    def print(self, code: str, dates: List[str]):
        pass
        
    def update(self, code: str, date: str):
        corrected_code = exchange.correct_security_code(code)
        ensure_transaction_data_updated(corrected_code, Timestamp.parse(date), False)

