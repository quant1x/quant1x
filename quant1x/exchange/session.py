# -*- coding: UTF-8 -*-
import re
import time
from dataclasses import dataclass
from enum import IntFlag, auto
from typing import List, Optional
from datetime import datetime

# 仅日期格式: 2022-11-28
FORMAT_ONLY_DATE = '%Y-%m-%d'
# 仅时间格式: 09:15:59
FORMAT_ONLY_TIME = '%H:%M:%S'
# 文件名中包含日期的日期格式: 20221128
FORMAT_FILE_DATE = '%Y%m%d'
# 时间戳: 2022-11-28 09:15:59
FORMAT_DATETIME = '%Y-%m-%d %H:%M:%S'
# 时间戳带毫秒数, 如果毫秒数保留前3位, 需要自己截取: 2022-11-28 09:15:59.123456
FORMAT_TIMESTAMP = '%Y-%m-%d %H:%M:%S.%f'


def seconds_to_timestamp(x: int):
    """
    秒数转时间戳字符串
    :param x:
    :return:
    """
    return time.strftime(FORMAT_DATETIME, time.localtime(x))

# ======================================================================
# 状态掩码标志（bitmask flags）
# ======================================================================
MaskClosed      = 0x00  # 无任何状态, 收盘, 休市
MaskActive      = 0x01  # 是否活跃（可用于处理订单）
MaskTrading     = 0x02  # 正常连续竞价阶段
MaskCallAuction = 0x04  # 集合竞价阶段
MaskOrder       = 0x08  # 是否可委托
MaskCancelable  = 0x10  # 是否允许撤单
MaskOpening     = 0x20  # 开盘, 集合竞价, 09:15~09:25
MaskClosing     = 0x40  # 收盘, 集合竞价, 14:57~15:00
MaskHalt        = 0x80  # 暂停交易（市场活跃但不能撮合, 熔断或临时停牌）

# ======================================================================
# 时间状态枚举（使用掩码组合）
# ======================================================================
class TimeStatus(IntFlag):
    ExchangeClosing               = MaskClosed                                  # 当日收盘（默认状态，不可交易）
    ExchangePreMarket             = MaskActive                                  # 盘前（活跃但未开始交易）
    ExchangeSuspend               = MaskHalt                                    # 休市中（非活跃，不可交易）
    ExchangeContinuousTrading     = MaskActive | MaskOrder | MaskTrading        # 连续竞价（上午/下午，可撤单）
    ExchangeTrading               = ExchangeContinuousTrading                   # 连续竞价, 盘中交易别名
    ExchangeCallAuction           = MaskActive | MaskOrder | MaskCallAuction    # 集合竞价
    ExchangeCallAuctionOpening    = ExchangeCallAuction | MaskOpening           # 早盘集合竞价
    ExchangeCallAuctionOpenPhase1 = ExchangeCallAuctionOpening | MaskCancelable # 9:15~9:20，开盘集合竞价，可撤单
    ExchangeCallAuctionOpenPhase2 = ExchangeCallAuctionOpening                  # 9:20~9:25，开盘集合竞价，不可撤单
    ExchangeCallAuctionClosePhase = ExchangeCallAuction | MaskClosing           # 14:57~15:00，收盘集合竞价，不可撤单
    ExchangeHaltTrading           = MaskActive | MaskHalt                       # 市场活跃但暂停交易（如临时停牌、熔断等）

# ======================================================================
# 辅助判断函数
# ======================================================================

def is_market_closed(status: int) -> bool:
    return status == TimeStatus.ExchangeClosing

def is_market_suspended(status: int) -> bool:
    return status == TimeStatus.ExchangeSuspend

def is_trading_halted(status: int) -> bool:
    return (status & MaskHalt) != 0

def is_market_active(status: int) -> bool:
    return (status & MaskActive) != 0

def is_in_continuous_trading(status: int) -> bool:
    return (status & MaskTrading) != 0

def is_in_call_auction(status: int) -> bool:
    return (status & MaskCallAuction) != 0

def is_call_auction_open_phase(status: int) -> bool:
    return (status & (TimeStatus.ExchangeCallAuction | MaskOpening)) == (TimeStatus.ExchangeCallAuction | MaskOpening)

def is_call_auction_close_phase(status: int) -> bool:
    return (status & (TimeStatus.ExchangeCallAuction | MaskClosing)) == (TimeStatus.ExchangeCallAuction | MaskClosing)

def is_order_cancelable(status: int) -> bool:
    return (status & MaskCancelable) != 0

def is_trading_disabled(status: int) -> bool:
    return status == TimeStatus.ExchangeClosing or status == TimeStatus.ExchangeSuspend or (status & MaskHalt)


@dataclass
class TimeRange(object):
    """
    时间范围, 用~或-间隔HH-MM-SS
    """
    begin: str
    end: str
    status: TimeStatus

    def __init__(self, time_range: str, status: TimeStatus = TimeStatus.ExchangeTrading):
        """
        构造
        :param time_range:
        :return:
        """
        self.begin = ''
        self.end = ''
        self.status = status

        time_range = time_range.strip()
        # 支持直接传入 begin, end 格式 (e.g. "09:30:00", "11:30:00")
        # 这里为了兼容旧代码，仍然解析字符串
        if ',' in time_range:
             # Handle case where multiple ranges might be passed by mistake, or just take the first one?
             # The original code split by ~ or -
             pass

        list_ = re.split(r"[~-]\s*", time_range)
        if len(list_) != 2:
            # Try to handle if it's just two times passed as args? No, __init__ takes one string.
            # If the user passes "09:30:00", "11:30:00" to constructor, it would be 2 args.
            # But here we take one string.
            # Let's assume the input is "09:30:00 ~ 11:30:00"
            raise RuntimeError(f"非法的时间格式: {time_range}")
        
        # 时间排序
        self.begin = list_[0].strip()
        self.end = list_[1].strip()
        if self.begin > self.end:
            self.begin, self.end = self.end, self.begin

    def in_range(self, timestamp: str = "") -> Optional[TimeStatus]:
        """
        是否在本交易时段
        """
        timestamp = timestamp.strip()
        if len(timestamp) == 0:
            timestamp = time.strftime(FORMAT_ONLY_TIME)
        
        # 简单的字符串比较，假设格式一致 (HH:MM:SS)
        if self.begin <= timestamp < self.end: # 左闭右开
             return self.status
        return None

    def is_trading(self, timestamp: str = "") -> bool:
        """
        是否交易中 (兼容旧接口)
        :param timestamp: %H:%M:%S
        :return:
        """
        status = self.in_range(timestamp)
        if status is not None:
            return is_in_continuous_trading(status) or is_in_call_auction(status)
        return False

    def is_valid(self) -> bool:
        """
        时段是否有效
        :return:
        """
        return self.begin != '' and self.end != ''

    def is_session_pre(self, timestamp: str = "") -> bool:
        """
        是否盘前
        :param timestamp: %H:%M:%S
        """
        timestamp = timestamp.strip()
        if len(timestamp) == 0:
            timestamp = time.strftime(FORMAT_ONLY_TIME)
        return timestamp < self.begin

    def is_session_reg(self, timestamp: str = "") -> bool:
        """
        是否盘中
        :param timestamp: %H:%M:%S
        """
        return self.is_trading(timestamp)

    def is_session_post(self, timestamp: str = "") -> bool:
        """
        是否盘后
        :param timestamp: %H:%M:%S
        """
        timestamp = timestamp.strip()
        if len(timestamp) == 0:
            timestamp = time.strftime(FORMAT_ONLY_TIME)
        return timestamp >= self.end # 右开区间，所以 >= end 就是盘后


@dataclass
class TradingSession:
    """
    交易时段
    """
    sessions: List[TimeRange]
    earliest_start: str = "23:59:59"
    latest_end: str = "00:00:00"

    def __init__(self, *args):
        """
        构造
        支持传入多个 TimeRange 对象，或者一个包含多个时间段的字符串
        """
        self.sessions = []
        
        if len(args) == 1 and isinstance(args[0], str):
            # 兼容旧的字符串构造方式: "09:30:00 ~ 11:30:00, 13:00:00 ~ 15:00:00"
            time_range_str = args[0].strip()
            list_ = re.split(r",\s*", time_range_str)
            for v in list_:
                v = v.strip()
                r = TimeRange(v) # 默认为 ExchangeTrading
                self.sessions.append(r)
        else:
            # 传入 TimeRange 对象列表
            for arg in args:
                if isinstance(arg, TimeRange):
                    self.sessions.append(arg)
        
        self.update_time_bounds()

    def update_time_bounds(self):
        if not self.sessions:
            self.earliest_start = "23:59:59"
            self.latest_end = "00:00:00"
            return
        
        self.earliest_start = "23:59:59"
        self.latest_end = "00:00:00"
        for session in self.sessions:
            if session.begin < self.earliest_start:
                self.earliest_start = session.begin
            if session.end > self.latest_end:
                self.latest_end = session.end

    def add_session(self, range: TimeRange):
        self.sessions.append(range)
        self.update_time_bounds()

    def check_status(self, timestamp: str = "") -> TimeStatus:
        """
        判断当前时间的状态
        """
        timestamp = timestamp.strip()
        if len(timestamp) == 0:
            timestamp = time.strftime(FORMAT_ONLY_TIME)
            
        for session in self.sessions:
            status = session.in_range(timestamp)
            if status is not None:
                return status
        
        # 全天交易开始前
        if timestamp < self.earliest_start:
            return TimeStatus.ExchangePreMarket
        
        # 全天交易结束前, 则会休市 (例如中午休市)
        if timestamp < self.latest_end:
            return TimeStatus.ExchangeHaltTrading
            
        # 不在任何交易时段内, 返回已收盘
        return TimeStatus.ExchangeClosing

    def is_trading(self, timestamp: str = "") -> bool:
        """
        是否交易中
        :param timestamp:
        :return:
        """
        status = self.check_status(timestamp)
        return is_in_continuous_trading(status) or is_in_call_auction(status)

    def is_valid(self) -> bool:
        """
        时段是否有效
        :return:
        """
        for item in self.sessions:
            if not item.is_valid():
                return False
        return True
    
    def is_trading_not_started(self, timestamp: str = "") -> bool:
        timestamp = timestamp.strip() or time.strftime(FORMAT_ONLY_TIME)
        return timestamp < self.earliest_start

    def is_trading_ended(self, timestamp: str = "") -> bool:
        timestamp = timestamp.strip() or time.strftime(FORMAT_ONLY_TIME)
        return timestamp > self.latest_end


def init_session() -> TradingSession:
    """
    初始化当日的交易会话时段 (A股)
    """
    # 9:15~9:20，开盘集合竞价，可撤单
    tr1 = TimeRange("09:15:00 ~ 09:20:00", TimeStatus.ExchangeCallAuctionOpenPhase1)
    # 9:20~9:25，开盘集合竞价，不可撤单
    tr2 = TimeRange("09:20:00 ~ 09:25:00", TimeStatus.ExchangeCallAuctionOpenPhase2)
    # 9:25~9:30，休市 (实际上是撮合时间，但对外部来说是不可交易的)
    tr3 = TimeRange("09:25:00 ~ 09:30:00", TimeStatus.ExchangeSuspend)
    # 9:30~11:30，连续竞价
    tr4 = TimeRange("09:30:00 ~ 11:30:00", TimeStatus.ExchangeTrading)
    # 13:00~14:57，连续竞价
    tr5 = TimeRange("13:00:00 ~ 14:57:00", TimeStatus.ExchangeTrading)
    # 14:57~15:00，收盘集合竞价
    tr6 = TimeRange("14:57:00 ~ 15:00:00", TimeStatus.ExchangeCallAuctionClosePhase)
    
    return TradingSession(tr1, tr2, tr3, tr4, tr5, tr6)


# 全局单例
ts_today_session = init_session()


@dataclass
class RuntimeStatus:
    before_last_trade_day: bool = False # 最后交易日前
    is_holiday: bool = False          # 是否节假日休市
    before_init_time: bool = False     # 初始化时间前
    cache_after_init_time: bool = False # 缓存在初始化时间之后
    update_in_real_time: bool = False   # 是否可以实时更新
    status: TimeStatus = TimeStatus.ExchangeClosing


def check_trading_timestamp(last_modified: Optional[str] = None) -> RuntimeStatus:
    """
    检查运行时交易状态 (Stub)
    """
    # TODO: 实现完整的检查逻辑
    return RuntimeStatus()


if __name__ == '__main__':
    dt = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    print(dt)

    session = init_session()
    print(f"Earliest: {session.earliest_start}, Latest: {session.latest_end}")
    
    test_times = ["09:00:00", "09:16:00", "09:22:00", "09:28:00", "09:35:00", "12:00:00", "13:30:00", "14:58:00", "15:01:00"]
    for t in test_times:
        status = session.check_status(t)
        print(f"Time: {t}, Status: {status.name} ({status.value}), Active: {is_market_active(status)}, Trading: {is_in_continuous_trading(status)}")

