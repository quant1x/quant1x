# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import re
import time
from dataclasses import dataclass
from enum import IntFlag
from typing import List, Optional, Union
from datetime import datetime

from quant1x.runtime.once import RollingOnce
from quant1x.std.time import get_timezone_offset_standard
from quant1x.log import logger

from .. import market
from .timestamp import Timestamp
from .exchange import Exchange
from .region import Region
from . import calendar, layout, tradinghours

# TODO: https://www.tradinghours.com/markets

def seconds_to_timestamp(x: int):
    """
    秒数转时间戳字符串
    :param x:
    :return:
    """
    return time.strftime(layout.FORMAT_DATETIME, time.localtime(x))

# ==========================================
# 1. 权限位掩码 (全属性统一)
# ==========================================
class Permission(IntFlag):
    """
    全球统一交易状态位掩码
    所有状态信息用一个整数表示
    
    位分配:
    - Bit 0-5: 订单操作权限
    - Bit 6-7: 状态性质 (临时/异常)
    - Bit 8-15: 预留扩展
    """
    # ========== 订单操作权限 (Bit 0-3) ==========
    NONE                = 0         # 0b00000000
    CANCEL              = 1 << 0    # 0b00000001 - 允许撤单
    """允许撤单"""
    MODIFY              = 1 << 1    # 0b00000010 - 允许改单
    """允许改单"""
    MARKET              = 1 << 2    # 0b00000100 - 允许市价单
    """允许市价单"""
    LIMIT               = 1 << 3    # 0b00001000 - 允许限价单
    """允许限价单"""
    
    # ========== 撮合机制 (Bit 4) ==========
    MATCHING            = 1 << 4    # 0b00010000 - 匹配中
    """匹配中"""
    
    # ========== 成交机制 (Bit 5) ==========
    FILL                = 1 << 5    # 0b00100000 - 会产生成交记录
    """成交"""
    
    # ========== 统计标志 (Bit 6) ==========
    OPEN                = 1 << 6    # 0b01000000 - 计入交易分钟数
    
    # ========== 状态性质 (Bit 7) ==========
    IS_TEMPORARY        = 1 << 7    # 0b10000000 - 临时状态 (可自动恢复)
    
    # ========== 常用组合 ==========
    MATCHING_TRANSACTION= MATCHING | FILL
    """撮合成交, 正在撮合中, 会产生成交记录"""
    
    # 连续交易：市价 + 限价 + 可成交 + 撤单 + 改单 + 计入分钟数
    CONTINUOUS_TRADING  = MARKET | LIMIT |  CANCEL | MODIFY | OPEN | MATCHING_TRANSACTION
    
    INITIALIZING        = IS_TEMPORARY
    """初始化阶段"""
    
    # 盘前
    PRE_MARKET          = IS_TEMPORARY | CANCEL | LIMIT
    """盘前, 允许下单、撤单, 但不允许市价单"""
    AFTER_HOURS         = IS_TEMPORARY | CANCEL | LIMIT
    """盘后, 允许下单、撤单, 但不允许市价单"""
    
    # 早盘集合竞价 = POS (Pre-Opening Session)
    # 收盘竞价时段 = CAS Closing Auction Session)
    
    # 集合竞价：限价 + 撤单 + 撮合
    CALL_AUCTION        = LIMIT | MATCHING | IS_TEMPORARY
    """集合竞价, 仅限价单, 临时状态 (可自动恢复)"""
    
    CALL_AUCTION_PRE    = CALL_AUCTION | CANCEL
    """集合竞价, 可撤单阶段"""
    CALL_AUCTION_ORDER  = CALL_AUCTION
    """集合竞价, 不可撤单阶段"""
    CALL_AUCTION_FILL   = CALL_AUCTION | FILL
    """集合竞价, 随机对盘阶段"""
    
    # 只挂单不成交 (午间休市) - 无 MATCHING
    ACCEPT_ORDER_ONLY   = LIMIT
    
    # 只读状态 (停牌)
    READ_ONLY           = CANCEL
    
    # 完全关闭
    CLOSED              = NONE
    
    # 紧急停牌 (只有 OPEN 位)
    EMERGENCY_HALT      = OPEN
    """紧急停牌 (市场活跃但不能撮合, 只有 OPEN 位)"""
    
    LUNCH_BREAK         = ACCEPT_ORDER_ONLY | IS_TEMPORARY
    """交易日休息时段 (允许下单、撤单, 但不允许市价单)"""
    
    def can_match(self) -> bool:
        """是否允许成交 (连续或集合竞价)"""
        return bool(self & Permission.MATCHING)
    
    def can_cancel(self) -> bool:
        return bool(self & Permission.CANCEL)
    
    def can_modify(self) -> bool:
        return bool(self & Permission.MODIFY)
    
    def can_market_order(self) -> bool:
        return bool(self & Permission.MARKET)
    
    def can_limit_order(self) -> bool:
        return bool(self & Permission.LIMIT)
    
    def is_suspended(self) -> bool:
        """是否暂停交易 (不允许撮合)"""
        return not self.can_match()
    
    def is_continuous_trading(self) -> bool:
        """是否计入交易分钟数"""
        return bool(self & Permission.OPEN)

# ======================================================================
# 时间状态枚举（使用掩码组合）
# ======================================================================
class TimeStatus(IntFlag):
    """全球统一交易时间状态枚举, 使用掩码组合表示不同状态"""
    OPEN                           = Permission.OPEN
    """开盘"""
    CLOSED                         = Permission.CLOSED              # 当日收盘（默认状态, 不可交易）
    """当日收盘（默认状态, 不可交易）"""
    PRE_MARKET                     = Permission.PRE_MARKET          # 盘前（活跃但未开始交易）
    AFTER_HOURS                    = Permission.AFTER_HOURS         # 盘后（活跃但已结束交易）
    SUSPEND                        = Permission.LUNCH_BREAK         # 休市中(非活跃, 不可交易)
    CONTINUOUS_TRADING             = Permission.CONTINUOUS_TRADING  # 连续竞价(上午/下午, 可撤单)
    TRADING                        = CONTINUOUS_TRADING             # 连续竞价, 盘中交易别名
    CALL_AUCTION                   = Permission.CALL_AUCTION        # 集合竞价(开盘/收盘)
    """集合竞价"""
    # 早盘集合竞价 = POS (Pre-Opening Session)
    # 收盘竞价时段 = CAS Closing Auction Session)
    AUCTION_ORDER_INPUT_PERIOD     = CALL_AUCTION | Permission.CANCEL
    """集合竞价, 订单输入 阶段, 可撤单"""
    AUCTION_NO_CANCELLATION_PERIOD = CALL_AUCTION
    """集合竞价, 不可撤销 阶段"""
    AUCTION_MATCHING_FILL_PERIOD   = CALL_AUCTION | Permission.FILL
    """集合竞价, 竞价撮合/随机对盘 阶段"""
    
    AUCTION_MATCHING_TO_OPENING    = CALL_AUCTION | Permission.FILL
    """集合竞价开盘 阶段"""
    AUCTION_MATCHING_TO_CLOSING    = CALL_AUCTION | Permission.FILL # CLOSING_AUCTION_MATCHING
    """集合竞价收盘 阶段"""
    
    ExchangeHaltTrading            = OPEN                         # 市场活跃但暂停交易(如临时停牌、熔断等)
    """市场活跃但暂停交易(如午间休市、临时停牌、熔断等)"""
    
    
    def is_market_active(self) -> bool:
        """市场是否活跃 (允许下单或撤单)"""
        return self.has_realtime_data()
    
    def is_open(self) -> bool:
        """市场是否开盘"""
        return (self & Permission.OPEN) == Permission.OPEN
    
    def is_continuous_trading(self) -> bool:
        """是否在连续竞价阶段 (计入交易分钟数)"""
        return (self & Permission.CONTINUOUS_TRADING) == Permission.CONTINUOUS_TRADING
    
    def is_trading_disabled(self) -> bool:
        """是否禁止交易 (不允许下单或成交)"""
        return (self & Permission.MATCHING) == 0
    
    # 有实时数据
    def has_realtime_data(self) -> bool:
        """是否有实时数据"""
        return bool(self & Permission.MATCHING)


@dataclass
class TimeRange(object):
    """
    时间范围, 用~或-间隔HH-MM-SS
    """
    begin: Timestamp
    end: Timestamp
    status: TimeStatus

    def __init__(self, time_range: str, status: TimeStatus = TimeStatus.TRADING, reg: Region = Region.CN):
        """
        构造
        :param time_range:
        :return:
        """
        self.begin = Timestamp.zero()
        self.end = Timestamp.zero()
        self.status = status
        self.reg = reg
        zone_offset_hours = get_timezone_offset_standard(reg.timezone)*-1
        
        time_range = time_range.strip()
        # 支持直接传入 begin, end 格式 (e.g. "09:30:00", "11:30:00")
        # 这里为了兼容旧代码, 仍然解析字符串
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
        begin_str = list_[0].strip()
        end_str = list_[1].strip()
        self.begin = Timestamp.parse_time(begin_str).offset(zone_offset_hours)
        self.end = Timestamp.parse_time(end_str).offset(zone_offset_hours)
        if self.begin > self.end:
            self.begin, self.end = self.end, self.begin

    def in_range(self, timestamp: Union[Timestamp, str] = "") -> Optional[TimeStatus]:
        """
        是否在本交易时段
        """
        # 将字符串转换为 Timestamp
        if isinstance(timestamp, str):
            timestamp = timestamp.strip()
            if len(timestamp) == 0:
                timestamp = Timestamp.parse_time(time.strftime(layout.FORMAT_ONLY_TIME))
            else:
                timestamp = Timestamp.parse_time(timestamp)
        elif not isinstance(timestamp, Timestamp):
            timestamp = Timestamp.now()

        # 比较 Timestamp
        if self.begin <= timestamp < self.end: # 左闭右开
             return self.status
        return None

    def is_trading(self, timestamp: Union[Timestamp, str] = "") -> bool:
        """
        是否连续竞价交易中
        :param timestamp: %H:%M:%S 或 Timestamp
        :return:
        """
        status = self.in_range(timestamp)
        if status is not None:
            return (status & TimeStatus.TRADING) == TimeStatus.TRADING
        return False

    def is_valid(self) -> bool:
        """
        时段是否有效
        :return:
        """
        return not self.begin.is_empty() and not self.end.is_empty()

    def is_session_pre(self, timestamp: Union[Timestamp, str] = "") -> bool:
        """
        是否盘前
        :param timestamp: %H:%M:%S 或 Timestamp
        """
        # 将字符串转换为 Timestamp
        if isinstance(timestamp, str):
            timestamp = timestamp.strip()
            if len(timestamp) == 0:
                timestamp = Timestamp.parse_time(time.strftime(layout.FORMAT_ONLY_TIME))
            else:
                timestamp = Timestamp.parse_time(timestamp)
        elif not isinstance(timestamp, Timestamp):
            timestamp = Timestamp.now()

        return timestamp < self.begin

    def is_session_reg(self, timestamp: Union[Timestamp, str] = "") -> bool:
        """
        是否盘中
        :param timestamp: %H:%M:%S 或 Timestamp
        """
        return self.is_trading(timestamp)

    def is_session_post(self, timestamp: Union[Timestamp, str] = "") -> bool:
        """
        是否盘后
        :param timestamp: %H:%M:%S 或 Timestamp
        """
        # 将字符串转换为 Timestamp
        if isinstance(timestamp, str):
            timestamp = timestamp.strip()
            if len(timestamp) == 0:
                timestamp = Timestamp.parse_time(time.strftime(layout.FORMAT_ONLY_TIME))
            else:
                timestamp = Timestamp.parse_time(timestamp)
        elif not isinstance(timestamp, Timestamp):
            timestamp = Timestamp.now()

        return timestamp >= self.end # 右开区间, 所以 >= end 就是盘后
    
    def get_duration_minutes(self) -> int:
        """计算时段总时长 (分钟)"""
        start_minutes = self.begin.value() // 60000 # 转换为分钟
        end_minutes = self.end.value() // 60000

        if end_minutes > start_minutes:
            return end_minutes - start_minutes
        else:
            return (24 * 60 - start_minutes) + end_minutes
    
    def get_elapsed_minutes(self, current_time: Timestamp) -> int:
        """时段已经开始多少分钟"""
        # if self.begin > current_time or current_time >= self.end:
        #     return 0
        current = min(current_time, self.end) # 不超过结束时间
        start = min(self.begin, current) # 不早于开始时间
        current_minutes = current.value() // 60000 # 转换为分钟
        start_minutes = start.value() // 60000

        if current_minutes >= start_minutes:
            return int(current_minutes - start_minutes)
        else:
            return 0


@dataclass
class TradingSession:
    """
    交易时段
    """
    sessions: List[TimeRange]
    earliest_start: Optional[Timestamp] = None
    """最早开始时间"""
    latest_end: Optional[Timestamp] = None
    """最晚结束时间"""
    # 收盘时间点 (例如 15:00:00), 用于判断是否已收盘 (timestamp >= closing_time 就认为已收盘)
    closing_time: Optional[Timestamp] = None
    """收盘时间点"""

    def __post_init__(self):
        """初始化时设置默认值"""
        if self.earliest_start is None:
            self.earliest_start = Timestamp.parse_time("23:59:59")
        if self.latest_end is None:
            self.latest_end = Timestamp.parse_time("00:00:00")
        if self.closing_time is None:
            self.closing_time = Timestamp.parse_time("00:00:00")

    def __init__(self, *args):
        """
        构造
        支持传入多个 TimeRange 对象, 或者一个包含多个时间段的字符串
        """
        self.sessions = []
        
        if len(args) == 1 and isinstance(args[0], str):
            # 兼容旧的字符串构造方式: "09:30:00 ~ 11:30:00, 13:00:00 ~ 15:00:00"
            time_range_str = args[0].strip()
            list_ = re.split(r",\s*", time_range_str)
            for v in list_:
                v = v.strip()
                r = TimeRange(v) # 默认为 TRADING
                self.sessions.append(r)
        else:
            # 传入 TimeRange 对象列表
            for arg in args:
                if isinstance(arg, TimeRange):
                    self.sessions.append(arg)
        
        self.update_time_bounds()

    def update_time_bounds(self):
        """
        更新交易时段的时间边界, 计算所有交易时段中的最早开始时间和最晚结束时间。
        
        如果没有交易时段(sessions为空), 则设置默认时间边界为23:59:59和00:00:00。
        否则遍历所有交易时段, 找到最早的开始时间(begin)和最晚的结束时间(end)。
        
        Attributes Updated:
            earliest_start (Timestamp): 所有交易时段中最小的开始时间
            latest_end (Timestamp): 所有交易时段中最大的结束时间
        """
        if not self.sessions:
            self.earliest_start = Timestamp.parse_time("23:59:59")
            self.latest_end = Timestamp.parse_time("00:00:00")
            self.closing_time = Timestamp.parse_time("00:00:00")
            return

        self.earliest_start = Timestamp.parse_time("23:59:59")
        self.latest_end = Timestamp.parse_time("00:00:00")
        self.closing_time = Timestamp.parse_time("00:00:00")
        for session in self.sessions:
            if session.begin < self.earliest_start:
                self.earliest_start = session.begin
            if session.end > self.latest_end:
                self.latest_end = session.end
                if session.status.is_open():
                    self.closing_time = session.end

    def add_session(self, range: TimeRange):
        self.sessions.append(range)
        self.update_time_bounds()

    def check_status(self, timestamp: Union[Timestamp, str] = "") -> TimeStatus:
        """
        判断当前时间的状态
        """
        # 将字符串转换为 Timestamp
        if isinstance(timestamp, str):
            timestamp = timestamp.strip()
            if len(timestamp) == 0:
                timestamp = Timestamp.parse_time(time.strftime(layout.FORMAT_ONLY_TIME))
            else:
                timestamp = Timestamp.parse_time(timestamp)
        elif not isinstance(timestamp, Timestamp):
            timestamp = Timestamp.now()

        for session in self.sessions:
            status = session.in_range(timestamp)
            if status is not None:
                return status

        # 不在任何交易时段内, 进一步判断是盘前、盘后还是休市

        # 全天交易开始前
        if timestamp < self.earliest_start:
            return TimeStatus.PRE_MARKET

        # 全天交易结束前, 则会休市 (例如中午休市)
        if timestamp < self.latest_end:
            return TimeStatus.ExchangeHaltTrading

        # 不在任何交易时段内, 返回已收盘
        return TimeStatus.CLOSED

    def is_trading(self, timestamp: Union[Timestamp, str] = "") -> bool:
        """
        是否交易中
        :param timestamp: %H:%M:%S 或 Timestamp
        :return:
        """
        status = self.check_status(timestamp)
        return (status & TimeStatus.TRADING) == TimeStatus.TRADING

    def is_valid(self) -> bool:
        """
        时段是否有效
        :return:
        """
        for item in self.sessions:
            if not item.is_valid():
                return False
        return True
    
    def is_trading_not_started(self, timestamp: Union[Timestamp, str] = "") -> bool:
        # 将字符串转换为 Timestamp
        if isinstance(timestamp, str):
            timestamp = timestamp.strip()
            if len(timestamp) == 0:
                timestamp = Timestamp.parse_time(time.strftime(layout.FORMAT_ONLY_TIME))
            else:
                timestamp = Timestamp.parse_time(timestamp)
        elif not isinstance(timestamp, Timestamp):
            timestamp = Timestamp.now()

        return timestamp < self.earliest_start

    def is_trading_ended(self, timestamp: Union[Timestamp, str] = "") -> bool:
        # 将字符串转换为 Timestamp
        if isinstance(timestamp, str):
            timestamp = timestamp.strip()
            if len(timestamp) == 0:
                timestamp = Timestamp.parse_time(time.strftime(layout.FORMAT_ONLY_TIME))
            else:
                timestamp = Timestamp.parse_time(timestamp)
        elif not isinstance(timestamp, Timestamp):
            timestamp = Timestamp.now()

        return timestamp > self.latest_end
    
    def minutes(self, timestamp: Union[Timestamp, str] = "") -> int:
        """
        计算当前时间距离最近的交易时间的分钟数
        :param timestamp: %H:%M:%S 或 Timestamp
        :return:
        """
        # 将字符串转换为 Timestamp
        if isinstance(timestamp, str):
            timestamp = timestamp.strip()
            if len(timestamp) == 0:
                timestamp = Timestamp.parse_time(time.strftime(layout.FORMAT_ONLY_TIME))
            else:
                timestamp = Timestamp.parse_time(timestamp)
        elif not isinstance(timestamp, Timestamp):
            timestamp = Timestamp.now()
        return sum(
            tr.get_elapsed_minutes(timestamp) 
            for tr in self.sessions 
            if tr.status.is_open()
        )
    
    def get_trading_minutes(self) -> int:
        """当日可交易时段总时长 (分钟)"""
        return sum(
            tr.get_duration_minutes() 
            for tr in self.sessions 
            if tr.status.is_open()
        )

def init_cn_session() -> TradingSession:
    """
    初始化当日的交易会话时段 (A股)
    """
    # 9:15~9:20, 开盘集合竞价, 可撤单
    tr1 = TimeRange("09:15:00 ~ 09:20:00", TimeStatus.AUCTION_ORDER_INPUT_PERIOD)
    # 9:20~9:25, 开盘集合竞价, 不可撤单
    tr2 = TimeRange("09:20:00 ~ 09:25:00", TimeStatus.AUCTION_MATCHING_TO_OPENING)
    # 9:25~9:30, 休市 (实际上是撮合时间, 但对外部来说是不可交易的)
    tr3 = TimeRange("09:25:00 ~ 09:30:00", TimeStatus.SUSPEND)
    # 9:30~11:30, 连续竞价
    tr4 = TimeRange("09:30:00 ~ 11:30:00", TimeStatus.TRADING)
    # 13:00~14:57, 连续竞价
    tr5 = TimeRange("13:00:00 ~ 14:57:00", TimeStatus.TRADING)
    # 14:57~15:00, 收盘集合竞价
    tr6 = TimeRange("14:57:00 ~ 15:00:00", TimeStatus.AUCTION_MATCHING_TO_CLOSING | Permission.OPEN)
    
    return TradingSession(tr1, tr2, tr3, tr4, tr5, tr6)

def init_hk_session() -> TradingSession:
    """
    初始化当日的交易会话时段 (港股)
    https://www.futunn.com/learn/detail-before-entering-the-market-understand-the-trading-rules-of-the-hong-kong-stock-market-83831-230556033
    """
    # 1. 输入买卖盘时段：上午9:00-9:15,这段时间可以随时输入下单(竞价市价单及竞价限价单), 且期间随时可以撤单。
    tr1 = TimeRange("09:00:00 ~ 09:15:00", TimeStatus.AUCTION_ORDER_INPUT_PERIOD)
    # 2. 不可取消时段：上午9:15-9:20, 这段时间随时可以下单, 但不可撤单。
    tr2 = TimeRange("09:15:00 ~ 09:20:00", TimeStatus.AUCTION_NO_CANCELLATION_PERIOD)
    # 3. 随机对盘时段：上午9:20-9:22, 在这段时间如果对盘成功, 会产生集合竞价的价格, 也就是开盘价。开盘价涨跌幅限制在15%以内。
    tr3 = TimeRange("09:20:00 ~ 09:22:00", TimeStatus.AUCTION_MATCHING_TO_OPENING)
    # 4. 暂停时段：完成对盘后-上午9:30, 这段时间的竞价限价单, 将自动转为限价单, 并于持续交易时继续等待成交。系统在9:28公布开盘价。
    tr4 = TimeRange("09:22:00 ~ 09:30:00", TimeStatus.SUSPEND)
    tr5 = TimeRange("09:30:00 ~ 12:00:00", TimeStatus.CONTINUOUS_TRADING)
    tr6 = TimeRange("12:00:00 ~ 13:00:00", TimeStatus.SUSPEND)
    tr7 = TimeRange("13:00:00 ~ 16:00:00", TimeStatus.CONTINUOUS_TRADING)
    #tr8 = TimeRange("16:00:00 ~ 16:10:00", TimeStatus.CALL_AUCTION_CLOSING)
    
    # 收盘竞价 - 参考价定价阶段(Reference Price) (16:00-16:01)
    tr8 = TimeRange("16:00:00 ~ 16:01:00", TimeStatus.AUCTION_ORDER_INPUT_PERIOD)
    
    # 收盘竞价 - 输入订单阶段 (16:01-16:06)
    tr9 = TimeRange("16:01:00 ~ 16:06:00", TimeStatus.AUCTION_ORDER_INPUT_PERIOD)
    
    # 收盘竞价 - 不可撤销阶段 (16:06-16:10)
    tr10 = TimeRange("16:06:00 ~ 16:08:00", TimeStatus.AUCTION_NO_CANCELLATION_PERIOD)
    # 收盘竞价 - 随机收盘 (16:06-16:10)
    tr11 = TimeRange("16:06:00 ~ 16:10:00", TimeStatus.AUCTION_MATCHING_TO_CLOSING)
    
    return TradingSession(tr1, tr2, tr3, tr4, tr5, tr6, tr7, tr8, tr9, tr10, tr11)

def init_us_session() -> TradingSession:
    """
    初始化当日的交易会话时段 (美股)
    """
    tr1 = TimeRange("04:00:00 ~ 09:30:00", TimeStatus.PRE_MARKET, Region.US)  # 盘前
    tr2 = TimeRange("09:30:00 ~ 16:00:00", TimeStatus.TRADING, Region.US)     # 盘中
    tr3 = TimeRange("16:00:00 ~ 20:00:00", TimeStatus.AFTER_HOURS, Region.US) # 盘后
    
    return TradingSession(tr1, tr2, tr3)


# 全局单例（由 RollingOnce 每日重建）
_trading_hours_map = {}
_trading_hours_default = init_cn_session() # 默认中国市场时段
_ts_today_session_once = RollingOnce(name='sessions_init', cron=tradinghours.cn_cron_expr_daily_init)


def _ts_today_session_init():
    """
    初始化今日各市场交易时段信息
    
    该函数负责初始化中国(CN)、香港(HK)和美国(US)市场的当日交易时段, 
    并将这些信息存储到全局变量 `_trading_hours_map` 中。
    
    注意:
        该函数会修改全局变量 `_trading_hours_map`, 
        键名为市场代码('cn', 'hk', 'us'), 
        值为对应市场的交易时段对象
    """
    global _trading_hours_map
    _ts_today_cn_session = init_cn_session()
    _ts_today_hk_session = init_hk_session()
    _ts_today_us_session = init_us_session()
    _trading_hours_map[Region.CN.value.lower()] = _ts_today_cn_session
    _trading_hours_map[Region.HK.value.lower()] = _ts_today_hk_session
    _trading_hours_map[Region.US.value.lower()] = _ts_today_us_session

def latest_session_by_exchange(exchange: Exchange = Exchange.SSE) -> TradingSession:
    """
    获取指定交易所当天的交易时段信息
    
    Args:
        exchange (Exchange): 交易所枚举, 默认为SSE（上海证券交易所）
    
    Returns:
        TradingSession: 返回对应交易所的交易时段对象
    
    Note:
        如果找不到指定交易所的配置, 会使用默认的中国市场交易时段并记录警告
    """
    global _trading_hours_map

    _ts_today_session_once.do(_ts_today_session_init)
    key = exchange.region.value.lower() if exchange.region else 'cn'
    session_ = _trading_hours_map.get(key)
    if session_ is None:
        #raise ValueError(f"Unsupported exchange: {exchange}")
        logger.warning(f"Unsupported exchange: {exchange}")
        session_ = _trading_hours_default  # default to CN if not found
    return session_


class RuntimeStatus:
    before_last_trade_day: bool = False # 最后交易日前
    is_holiday: bool = False          # 是否节假日休市
    before_init_time: bool = False     # 初始化时间前
    cache_after_init_time: bool = False # 缓存在初始化时间之后
    update_in_real_time: bool = False   # 是否可以实时更新
    status: TimeStatus = TimeStatus.CLOSED


def check_trading_timestamp(exchange: Exchange = Exchange.SSE, last_modified: Optional[Timestamp] = None) -> RuntimeStatus:
    logger.debug(f"check_trading_timestamp called with exchange={exchange}, last_modified={last_modified}")
    rs = RuntimeStatus()
    rs.status = TimeStatus.CLOSED

    now = Timestamp.now()
    if last_modified is not None:
        ts = last_modified
    else:
        ts = now
    
    logger.debug(f"check_trading_timestamp: {ts}")
    
    last_day = calendar.last_trading_day(now)

    # 1. timestamp before last trading day
    if ts < last_day:
        rs.before_last_trade_day = True
        return rs

    # 2. if today != last_day => holiday
    today = now
    if not today.is_same_date(last_day):
        rs.is_holiday = True
        return rs

    # 3. before init
    if ts < get_today():
        rs.before_init_time = True
        return rs

    rs.status = TimeStatus.PRE_MARKET
    rs.cache_after_init_time = True

    # 5. trading not started
    session = latest_session_by_exchange(exchange)
    # convert timestamp to time-only string for existing string-based session
    tstr = ts.to_string(layout.FORMAT_ONLY_TIME)
    if session.is_trading_not_started(tstr):
        return rs

    rs.update_in_real_time = True

    rs.status = session.check_status(tstr)
    if rs.status.is_trading_disabled():
        rs.update_in_real_time = False
    return rs


_ts_today_init: Timestamp = Timestamp.zero()
_ts_today_init_once = RollingOnce(name='today_init', cron=tradinghours.cn_cron_expr_daily_init)

def get_today() -> Timestamp:
    global _ts_today_init

    # Use rolling once to set _ts_today_init exactly once per day at pre-market reset
    def do_init():
        global _ts_today_init
        now = Timestamp.now()
        _ts_today_init = now.get_pre_market_time()

    _ts_today_init_once.do(do_init)
    return _ts_today_init


def can_initialize(exchange: Exchange = Exchange.SSE, last_modified: Optional[Timestamp] = None) -> bool:
    rs = check_trading_timestamp(exchange, last_modified)
    if rs.before_last_trade_day:
        return True
    if rs.is_holiday:
        return False
    if rs.before_init_time:
        return False
    return not rs.cache_after_init_time


if __name__ == '__main__':
    dt = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    print(dt)

    session = latest_session_by_exchange(Exchange.USA)
    print(f"Earliest: {session.earliest_start}, Latest: {session.latest_end}, Closing: {session.closing_time}")
    print(f"Trading minutes: {session.get_trading_minutes()}")
    test_times = ["09:00:00", "09:16:00", "09:22:00", "09:28:00", "09:35:00", "12:00:00", "13:30:00", "14:58:00", "15:01:00"]
    for t in test_times:
        status = session.check_status(t)
        ts = Timestamp.parse_time(t)
        print(f'{t} -> {ts} -> {ts.to_string()}')
        print(f'elapsed: {t} -> {session.minutes(ts)}, trading: {session.is_trading(ts)}')
        print(f"Time: {t}, Status: {status.name} ({status.value}), Active: {status.is_market_active()}, Trading: {status.is_continuous_trading()}")

