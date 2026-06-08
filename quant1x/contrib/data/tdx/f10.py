# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
import csv
import traceback
import requests
from dataclasses import dataclass
from typing import Optional, List, Dict

import pandas as pd

from quant1x.config import config
from quant1x.data import status
from quant1x.data.base import BASEDATA_F10
from quant1x.data.meta.timestamp import Timestamp
from quant1x.data.market import Instrument, Exchange, detect_symbol
from quant1x.data.schema import XdxrInfo
from quant1x.log import logger
from quant1x.std import filesystem as fs
from quant1x.std.time import get_quarter_by_date

from .client import get_std_conn
from . import protocol
from .level1 import FinanceInfoRequest
from .xdxr import get_xdxr_list
from .kline import ipo_date_from_xdxrs
from . import share_holder
from . import financial_report
from . import notice


@dataclass
class F10:
    """F10因子数据结构"""
    date: str = ''
    code: str = ''
    security_name: str = ''
    sub_new: bool = False
    margin_trading_target: bool = False
    vol_unit: int = 100
    decimal_point: int = 2
    ipo_date: str = ''
    update_date: str = ''
    total_capital: float = 0.0
    capital: float = 0.0
    free_capital: float = 0.0
    top10_capital: float = 0.0
    top10_change: float = 0.0
    change_capital: float = 0.0
    increase_ratio: float = 0.0
    reduction_ratio: float = 0.0
    quarterly_year_quarter: str = ''
    q_date: str = ''
    annual_report_date: str = ''
    quarterly_report_date: str = ''
    total_operate_income: float = 0.0
    bps: float = 0.0
    basic_eps: float = 0.0
    deduct_basic_eps: float = 0.0
    safety_score: int = 0
    increases: int = 0
    reduces: int = 0
    risk: int = 0
    risk_keywords: str = ''
    update_time: int = 0
    state: int = 0


# ---- 文件路径 ----

def _get_f10_filename(inst: Instrument) -> str:
    """根据股票代码生成对应的F10因子数据文件路径"""
    base = config.data_path
    sub = f'f10/{inst.cache_dir()}'
    symbol = inst.symbol()
    return f'{base}/{sub}/{symbol}.csv'


def _f10_headers() -> List[str]:
    """返回F10 CSV文件头"""
    return [
        "date", "code", "security_name", "sub_new", "margin_trading_target",
        "vol_unit", "decimal_point", "ipo_date", "update_date",
        "total_capital", "capital", "free_capital", "top10_capital",
        "top10_change", "change_capital", "increase_ratio", "reduction_ratio",
        "quarterly_year_quarter", "q_date", "annual_report_date",
        "quarterly_report_date", "total_operate_income", "bps",
        "basic_eps", "deduct_basic_eps", "safety_score",
        "increases", "reduces", "risk", "risk_keywords",
        "update_time", "state",
    ]


# ---- CSV 序列化/反序列化 ----

def _f10_to_row(f: F10) -> List:
    """将F10对象转换为CSV行"""
    return [
        f.date, f.code, f.security_name, f.sub_new, f.margin_trading_target,
        f.vol_unit, f.decimal_point, f.ipo_date, f.update_date,
        f.total_capital, f.capital, f.free_capital, f.top10_capital,
        f.top10_change, f.change_capital, f.increase_ratio, f.reduction_ratio,
        f.quarterly_year_quarter, f.q_date, f.annual_report_date,
        f.quarterly_report_date, f.total_operate_income, f.bps,
        f.basic_eps, f.deduct_basic_eps, f.safety_score,
        f.increases, f.reduces, f.risk, f.risk_keywords,
        f.update_time, f.state,
    ]


def _row_to_f10(row: Dict[str, str]) -> F10:
    """将CSV行转换为F10对象"""
    f = F10()
    f.date = row.get("date", "")
    f.code = row.get("code", "")
    f.security_name = row.get("security_name", "")
    f.sub_new = row.get("sub_new", "False") == "True"
    f.margin_trading_target = row.get("margin_trading_target", "False") == "True"
    f.vol_unit = int(float(row.get("vol_unit", 100)))
    f.decimal_point = int(float(row.get("decimal_point", 2)))
    f.ipo_date = row.get("ipo_date", "")
    f.update_date = row.get("update_date", "")
    f.total_capital = float(row.get("total_capital", 0))
    f.capital = float(row.get("capital", 0))
    f.free_capital = float(row.get("free_capital", 0))
    f.top10_capital = float(row.get("top10_capital", 0))
    f.top10_change = float(row.get("top10_change", 0))
    f.change_capital = float(row.get("change_capital", 0))
    f.increase_ratio = float(row.get("increase_ratio", 0))
    f.reduction_ratio = float(row.get("reduction_ratio", 0))
    f.quarterly_year_quarter = row.get("quarterly_year_quarter", "")
    f.q_date = row.get("q_date", "")
    f.annual_report_date = row.get("annual_report_date", "")
    f.quarterly_report_date = row.get("quarterly_report_date", "")
    f.total_operate_income = float(row.get("total_operate_income", 0))
    f.bps = float(row.get("bps", 0))
    f.basic_eps = float(row.get("basic_eps", 0))
    f.deduct_basic_eps = float(row.get("deduct_basic_eps", 0))
    f.safety_score = int(float(row.get("safety_score", 0)))
    f.increases = int(float(row.get("increases", 0)))
    f.reduces = int(float(row.get("reduces", 0)))
    f.risk = int(float(row.get("risk", 0)))
    f.risk_keywords = row.get("risk_keywords", "")
    f.update_time = int(float(row.get("update_time", 0)))
    f.state = int(float(row.get("state", 0)))
    return f


# ---- CSV 读写 ----

def load_f10(inst: Instrument) -> List[F10]:
    """从CSV文件加载F10因子数据"""
    result: List[F10] = []
    try:
        filename = _get_f10_filename(inst)
        logger.debug(f"Loading F10 data from {filename}")
        if os.path.exists(filename):
            with open(filename, 'r', encoding='utf-8') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    result.append(_row_to_f10(row))
    except Exception:
        logger.exception(f"[tdx::f10] load failed for {inst}")
    return result


def load_f10_as_dataframe(inst: Instrument) -> pd.DataFrame:
    """从CSV文件加载F10因子数据，返回DataFrame"""
    try:
        filename = _get_f10_filename(inst)
        if os.path.exists(filename):
            return pd.read_csv(filename)
    except Exception:
        logger.exception(f"[tdx::f10] load DataFrame failed for {inst}")
    return pd.DataFrame(columns=_f10_headers())


def save_f10(inst: Instrument, values: List[F10]):
    """保存F10因子数据到CSV文件"""
    filename = _get_f10_filename(inst)
    try:
        fs.mkdirs(os.path.dirname(filename))
        with open(filename, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(_f10_headers())
            for v in values:
                writer.writerow(_f10_to_row(v))
    except Exception:
        logger.exception(f"[tdx::f10] save failed for {inst}")


# ---- TDX 数据获取 ----

# 安全分 HTTP API
URL_SAFETY_SCORE = "http://page3.tdx.com.cn:7615/site/pcwebcall_static/bxb/json/"
DEFAULT_SAFETY_SCORE = 100

def _fetch_finance_info_from_tdx(exchange: Exchange, ticker: str):
    """从TDX标准行情TCP获取财务信息"""
    try:
        conn = get_std_conn()
        msg = FinanceInfoRequest(exchange=exchange, ticker=ticker)
        protocol.process_level1_new(conn, msg)
        return msg.info
    except Exception:
        logger.exception(f"[tdx::f10] fetch finance_info failed for {ticker}")
        return None


def _fetch_safety_score(security_code: str) -> int:
    """从通达信安全分HTTP接口获取个股风险评分

    参考 factors/safety_score.py 的实现，纯HTTP调用，不依赖factors层。
    """
    try:
        inst = detect_symbol(security_code)
        pure_code = inst.ticker

        if len(pure_code) != 6:
            return DEFAULT_SAFETY_SCORE

        url = f"{URL_SAFETY_SCORE}{pure_code}.json"
        response = requests.get(url, timeout=5)
        if response.status_code != 200:
            return DEFAULT_SAFETY_SCORE

        data = response.json()
        report_data = data.get("data", [])
        score = 100

        for category in report_data:
            rows = category.get("rows", [])
            for v in rows:
                if v.get("trig", 0) == 1:
                    score -= v.get("fs", 0)

        return score
    except Exception:
        logger.debug(f"[tdx::f10] fetch safety_score failed for {security_code}")
        return DEFAULT_SAFETY_SCORE


def _checkout_capital(xdxr_list: List[XdxrInfo], date: str) -> Optional[XdxrInfo]:
    """从除权除息列表中查找指定日期的股本变更记录"""
    sorted_list = sorted(xdxr_list, key=lambda x: x.Date, reverse=True)
    for v in sorted_list:
        if v.is_capital_change() and date >= v.Date:
            return v
    return None


def _compute_free_capital(holder_df, capital: float):
    """计算前十大流通股东相关指标 (参考 C++ ComputeFreeCapital)"""
    top10_capital = 0.0
    free_capital = capital
    capital_changed = 0.0
    increase_ratio = 0.0
    reduction_ratio = 0.0

    increase = 0
    reduce = 0

    if holder_df.empty:
        return 0.0, capital, 0.0, 0.0, 0.0

    if 'HoldNum' not in holder_df.columns:
        return 0.0, capital, 0.0, 0.0, 0.0

    for i, row in holder_df.iterrows():
        hold_num = row['HoldNum']
        hold_change = row['HoldNumChange']

        top10_capital += hold_num
        capital_changed += hold_change

        if hold_change >= 0:
            increase += hold_change
        else:
            reduce += hold_change

        if i >= 10:
            continue

        free_ratio = row.get('FreeHoldNumRatio', 0.0)
        is_org = str(row.get('IsHoldOrg', '0'))

        if free_ratio >= 1.00 and is_org == '1':
            free_capital -= hold_num

    if top10_capital > 0:
        increase_ratio = 100.0 * (increase / top10_capital)
        reduction_ratio = 100.0 * (reduce / top10_capital)

    return top10_capital, free_capital, capital_changed, increase_ratio, reduction_ratio


def _checkout_share_holder(security_code: str, feature_date: str, xdxr_list: List[XdxrInfo]):
    """获取前十大流通股东数据 (参考 C++ checkoutShareHolder)"""
    v = _checkout_capital(xdxr_list, feature_date)
    if not v:
        return None

    # 获取股东列表
    df = share_holder.get_cache_share_holder(security_code, feature_date)

    capital = v.HouLiuTong * 10000
    total_capital = v.HouZongGuBen * 10000

    top10_capital, free_capital, capital_changed, increase_ratio, reduction_ratio = \
        _compute_free_capital(df, capital)

    if free_capital < 0:
        top10_capital, free_capital, capital_changed, increase_ratio, reduction_ratio = \
            _compute_free_capital(df, total_capital)

    # 前一期数据
    front_df = share_holder.get_cache_share_holder(security_code, feature_date, 2)
    front_top10_capital, _, _, _, _ = _compute_free_capital(front_df, total_capital)

    return {
        "FreeCapital": free_capital,
        "Top10Capital": top10_capital,
        "Top10Change": top10_capital - front_top10_capital,
        "ChangeCapital": capital_changed,
        "IncreaseRatio": increase_ratio,
        "ReductionRatio": reduction_ratio,
    }


def _compute_f10_from_tdx(inst: Instrument, date: str) -> Optional[F10]:
    """
    从 contrib 层数据源计算F10因子 (对齐 C++/Rust F10Feature::Update)。

    数据来源:
    - 财务信息 (TCP STD_FINANCE_INFO): 股本、IPO日期、更新日期
    - 除权除息 (get_xdxr_list): 历史股本变化, IPO日期fallback
    - 前十大流通股东 (东方财富API): 自由流通股本、增减持
    - 季报 (东方财富API): bps, eps, 营业收入
    - 公告 (东方财富API): 增减持、风险
    - 安全分 (TDX HTTP API): 个股风险评分
    - 季度信息 (std.time): 当前所属季报期

    暂不依赖的字段:
    - margin_trading_target: 依赖 exchange 层东方财富API
    """
    security_code = inst.code()

    f10 = F10()
    f10.date = date
    f10.code = security_code
    f10.security_name = inst.name
    f10.vol_unit = inst.lot_size
    f10.decimal_point = inst.price_precision
    f10.update_date = date

    # 1. 基本信息: 除权除息股本 (参考 C++ checkoutSecurityBasicInfo)
    xdxr_list = get_xdxr_list(inst)
    v = _checkout_capital(xdxr_list, date)
    if v:
        f10.total_capital = v.HouZongGuBen * 10000
        f10.capital = v.HouLiuTong * 10000
    else:
        finance = _fetch_finance_info_from_tdx(inst.exchange, inst.ticker)
        if finance and not finance.is_delisting():
            f10.capital = finance.liu_tong_gu_ben
            f10.total_capital = finance.zong_gu_ben

    # 2. IPO日期: 先从财务信息获取，否则从除权除息提取
    finance = _fetch_finance_info_from_tdx(inst.exchange, inst.ticker)
    if finance and not finance.is_delisting():
        if finance.ipo_date >= 19900101:
            ipo_str = str(finance.ipo_date)
            f10.ipo_date = f"{ipo_str[:4]}-{ipo_str[4:6]}-{ipo_str[6:]}"
        if finance.updated_date >= 19900101:
            upd_str = str(finance.updated_date)
            f10.update_date = f"{upd_str[:4]}-{upd_str[4:6]}-{upd_str[6:]}"

    if not f10.ipo_date:
        ipo_from_xdxr = ipo_date_from_xdxrs(xdxr_list)
        if ipo_from_xdxr:
            f10.ipo_date = ipo_from_xdxr

    if not f10.update_date or f10.update_date > date:
        f10.update_date = date

    # 3. 次新股: IPO 1年内
    if f10.ipo_date:
        try:
            ipo_dt = pd.to_datetime(f10.ipo_date)
            feature_dt = pd.to_datetime(date)
            one_year_later = ipo_dt + pd.DateOffset(years=1)
            if feature_dt < one_year_later:
                f10.sub_new = True
        except Exception:
            pass

    # 4. 两融标的: 依赖 exchange 层, 保留默认值
    #    margin_trading_target 由上游补充

    # 5. 前十大流通股股东 (东方财富API)
    try:
        holder_info = _checkout_share_holder(security_code, date, xdxr_list)
        if holder_info:
            f10.free_capital = holder_info["FreeCapital"]
            f10.top10_capital = holder_info["Top10Capital"]
            f10.top10_change = holder_info["Top10Change"]
            f10.change_capital = holder_info["ChangeCapital"]
            f10.increase_ratio = holder_info["IncreaseRatio"]
            f10.reduction_ratio = holder_info["ReductionRatio"]
    except Exception as e:
        logger.warning(f"[tdx::f10] share_holder failed for {security_code}: {e}")
        logger.warning(traceback.format_exc())

    # 如果自由流通股本为0，使用流通股本
    if f10.free_capital == 0:
        f10.free_capital = f10.capital

    # 6. 上市公司公告
    try:
        n = notice.get_one_notice(security_code, date)
        if n:
            f10.increases = n.increase
            f10.reduces = n.reduce
            f10.risk = n.risk
            f10.risk_keywords = n.risk_keywords
    except Exception as e:
        logger.warning(f"[tdx::f10] notice failed for {security_code}: {e}")
        logger.warning(traceback.format_exc())

    # 7. 季报
    try:
        report = financial_report.get_quarterly_report_summary(security_code, date)
        if report:
            f10.q_date = report.QDate
            f10.bps = report.BPS
            f10.basic_eps = report.BasicEPS
            f10.total_operate_income = report.TotalOperateIncome
            f10.deduct_basic_eps = report.DeductBasicEPS
    except Exception as e:
        logger.warning(f"[tdx::f10] financial_report failed for {security_code}: {e}")
        logger.warning(traceback.format_exc())

    # 8. 季度信息
    try:
        quarter_str, _, _ = get_quarter_by_date(date, 1)
        f10.quarterly_year_quarter = quarter_str
    except Exception:
        logger.debug(f"[tdx::f10] get_quarter_by_date failed for date={date}")

    # 9. 安全分 (TDX HTTP API)
    f10.safety_score = _fetch_safety_score(security_code)

    # 10. 年报/季报披露日期
    try:
        annual_date, quarterly_date = notice.notice_date_for_report(security_code, date)
        f10.annual_report_date = annual_date
        f10.quarterly_report_date = quarterly_date
    except Exception:
        logger.debug(f"[tdx::f10] notice_date_for_report failed for {security_code}")

    # 时间戳
    f10.update_time = int(pd.Timestamp.now().timestamp())

    return f10


# ---- 更新和获取 ----

def update_f10(inst: Instrument, date: Optional[str] = None):
    """
    更新指定合约的F10因子数据。

    从TDX标准行情TCP获取财务信息和除权除息数据，
    计算基本F10因子并缓存到本地CSV。

    Args:
        inst: 合约信息
        date: 目标日期，格式为'YYYY-MM-DD'，默认为None表示使用当前日期
    """
    try:
        if date is None:
            from quant1x.data.meta.calendar import last_trading_day
            ts = last_trading_day() if inst.exchange in (Exchange.SSE, Exchange.SZSE, Exchange.BSE) else Timestamp.now().offset(hour=-24)
            date = ts.only_date()

        f10_obj = _compute_f10_from_tdx(inst, date)
        if f10_obj is None:
            logger.warning(f"[tdx::f10] failed to compute F10 for {inst}")
            return

        # 加载已有数据，按日期去重后追加新数据
        existing = load_f10(inst)
        existing_dates = {e.date for e in existing}

        if f10_obj.date not in existing_dates:
            existing.append(f10_obj)
            # 按日期排序
            existing.sort(key=lambda x: x.date)
            save_f10(inst, existing)
            logger.debug(f"[tdx::f10] updated F10 for {inst} on {date}")
        else:
            logger.debug(f"[tdx::f10] F10 for {inst} on {date} already exists, skip")
    except Exception:
        logger.exception(f"[tdx::f10] update failed for {inst}")


def get_f10(inst: Instrument, date: Optional[str] = None) -> Optional[F10]:
    """
    获取指定合约和日期的F10因子数据。

    优先从本地缓存加载，若不存在则触发更新。

    Args:
        inst: 合约信息
        date: 目标日期，格式为'YYYY-MM-DD'

    Returns:
        F10对象，若未找到返回None
    """
    filename = _get_f10_filename(inst)

    # 检查是否需要初始化或更新
    create_or_update = status.should_initialize_file(fname=filename, exchange=inst.exchange)
    if create_or_update:
        logger.debug(f"[tdx::f10] update F10 data for {inst}")
        update_f10(inst, date)
    else:
        logger.debug(f"[tdx::f10] load cached F10 data for {inst}")

    records = load_f10(inst)
    if not records:
        return None

    if date is None:
        # 返回最新一条
        return records[-1]

    # 查找指定日期的记录
    for r in records:
        if r.date == date:
            return r

    # 如果未找到精确匹配，尝试触发当日更新
    logger.debug(f"[tdx::f10] no cached F10 for {inst} on {date}, updating...")
    update_f10(inst, date)
    records = load_f10(inst)
    for r in records:
        if r.date == date:
            return r

    return None


# ---- DataAdapter 注册 ----

from quant1x.data import adapter
from quant1x.data.adapter import DataAdapter, DEFAULT_DATA_PROVIDER


class DataF10(DataAdapter):
    """F10因子数据适配器"""

    def kind(self):
        return BASEDATA_F10

    def owner(self):
        return DEFAULT_DATA_PROVIDER

    def key(self):
        return "f10"

    def name(self):
        return "F10因子"

    def usage(self):
        return ""

    def print(self, inst: Instrument, date: Optional[Timestamp] = None):
        f10_obj = get_f10(inst, date.only_date() if date else None)
        if f10_obj:
            logger.info(f"F10: {f10_obj.code} {f10_obj.date} name={f10_obj.security_name} "
                        f"total_capital={f10_obj.total_capital} capital={f10_obj.capital} "
                        f"bps={f10_obj.bps} basic_eps={f10_obj.basic_eps} "
                        f"safety_score={f10_obj.safety_score}")

    def update(self, inst: Instrument, date: Optional[Timestamp] = None):
        date_str = date.only_date() if date else None
        update_f10(inst, date_str)


# 注册插件
_data_f10_plugin = adapter.register(DataF10)


if __name__ == "__main__":
    from .instruments import get_instrument_info

    # 测试A股
    code = "sh600000"
    inst = get_instrument_info(code)
    if inst is None:
        print(f"Instrument not found: {code}")
        exit(1)
    print(f"Instrument: {inst}")

    # 更新F10数据
    update_f10(inst)

    # 加载F10数据
    f10_obj = get_f10(inst)
    print(f10_obj)
    if f10_obj:
        print(f"F10: date={f10_obj.date}, code={f10_obj.code}, name={f10_obj.security_name}")
        print(f"  total_capital={f10_obj.total_capital}, capital={f10_obj.capital}")
        print(f"  bps={f10_obj.bps}, basic_eps={f10_obj.basic_eps}")
        print(f"  safety_score={f10_obj.safety_score}")
