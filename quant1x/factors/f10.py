# -*- coding: UTF-8 -*-

from __future__ import annotations

import logging
import struct
from dataclasses import dataclass
from typing import Optional, Tuple, List, Dict

import pandas as pd
from quant1x.datasets import xdxr
from quant1x.exchange import detect_market, correct_security_code, is_margin_trading_target
from quant1x.exchange.security import get_security_info
from quant1x.factors import share_holder, financial_report, safety_score, notice
from quant1x.level1.client import client
from quant1x.level1.finance_info import FinanceRequest, FinanceResponse
from quant1x.level1.protocol import process
from quant1x import std
from quant1x.level1.xdxr_info import XdxrInfo

logger = logging.getLogger(__name__)

@dataclass
class F10:
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

def get_finance_info(security_code: str, feature_date: str) -> Tuple[float, float, str, str]:
    capital = 0.0
    total_capital = 0.0
    ipo_date = ""
    update_date = ""
    base_date = 19900101

    try:
        with client() as conn:
            req = FinanceRequest(security_code)
            resp = FinanceResponse()
            process(conn.socket, req, resp)
            
            if resp.count > 0:
                info = resp.info
                if info.liu_tong_gu_ben > 0 and info.zong_gu_ben > 0:
                    capital = info.liu_tong_gu_ben
                    total_capital = info.zong_gu_ben
                
                if info.ipo_date >= base_date:
                    ipo_date = str(info.ipo_date)
                    ipo_date = f"{ipo_date[:4]}-{ipo_date[4:6]}-{ipo_date[6:]}"
                
                if info.updated_date >= base_date:
                    update_date = str(info.updated_date)
                    update_date = f"{update_date[:4]}-{update_date[4:6]}-{update_date[6:]}"
                    
    except Exception as e:
        logger.debug(f"get_finance_info failed: {e}")
        
    return capital, total_capital, ipo_date, update_date

def checkout_capital(xdxr_list: List[XdxrInfo], date: str) -> Optional[XdxrInfo]:
    # List is sorted by Date descending
    for v in xdxr_list:
        if v.is_capital_change() and date >= v.Date:
            return v
    return None

def checkout_security_basic_info(security_code: str, feature_date: str) -> dict:
    info = {
        "TotalCapital": 0.0,
        "Capital": 0.0,
        "VolUnit": 100,
        "DecimalPoint": 2,
        "Name": "Unknown",
        "IpoDate": "",
        "SubNew": False,
        "UpdateDate": ""
    }
    
    xdxr_list = xdxr.load_xdxr(security_code)
    # Sort descending by Date
    xdxr_list.sort(key=lambda x: x.Date, reverse=True)
    
    v = checkout_capital(xdxr_list, feature_date)
    if v:
        info["TotalCapital"] = v.HouZongGuBen * 10000
        info["Capital"] = v.HouLiuTong * 10000
        # Try to get IPO date from finance info if missing
        _, _, ipo_date, _ = get_finance_info(security_code, feature_date)
        info["IpoDate"] = ipo_date
    else:
        capital, total_capital, ipo_date, update_date = get_finance_info(security_code, feature_date)
        info["Capital"] = capital
        info["TotalCapital"] = total_capital
        info["IpoDate"] = ipo_date
        info["UpdateDate"] = update_date
        
    if not info["UpdateDate"] or info["UpdateDate"] > feature_date:
        info["UpdateDate"] = feature_date
        
    if info["IpoDate"]:
        # Check if sub-new (within 1 year)
        try:
            ipo_dt = pd.to_datetime(info["IpoDate"])
            feature_dt = pd.to_datetime(feature_date)
            one_year_later = ipo_dt + pd.DateOffset(years=1)
            if feature_dt < one_year_later:
                info["SubNew"] = True
        except Exception:
            pass
            
    sec_info = get_security_info(security_code)
    if sec_info:
        info["VolUnit"] = sec_info.lot_size
        info["DecimalPoint"] = sec_info.price_precision
        info["Name"] = sec_info.name
        
    return info

def compute_free_capital(holder_list: pd.DataFrame, capital: float) -> Tuple[float, float, float, float, float]:
    top10_capital = 0.0
    free_capital = capital
    capital_changed = 0.0
    increase_ratio = 0.0
    reduction_ratio = 0.0
    
    increase = 0
    reduce = 0
    
    if holder_list.empty:
        return 0.0, capital, 0.0, 0.0, 0.0
        
    if 'HoldNum' not in holder_list.columns:
        return 0.0, capital, 0.0, 0.0, 0.0

    # Iterate top 10
    for i, row in holder_list.iterrows():
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

def checkout_share_holder(security_code: str, feature_date: str) -> Optional[dict]:
    xdxr_list = xdxr.load_xdxr(security_code)
    xdxr_list.sort(key=lambda x: x.Date, reverse=True)
    
    xdxr_info_obj = checkout_capital(xdxr_list, feature_date)
    
    if xdxr_info_obj:
        # Get shareholder list
        df = share_holder.get_cache_share_holder(security_code, feature_date)
        
        capital = xdxr_info_obj.HouLiuTong * 10000
        total_capital = xdxr_info_obj.HouZongGuBen * 10000
        
        top10_capital, free_capital, capital_changed, increase_ratio, reduction_ratio = \
            compute_free_capital(df, capital)
            
        if free_capital < 0:
             top10_capital, free_capital, capital_changed, increase_ratio, reduction_ratio = \
                compute_free_capital(df, total_capital)
                
        # Front list (previous quarter?)
        front_df = share_holder.get_cache_share_holder(security_code, feature_date, 2)
        front_top10_capital, _, _, _, _ = compute_free_capital(front_df, total_capital)
        
        return {
            "Code": security_code,
            "FreeCapital": free_capital,
            "Top10Capital": top10_capital,
            "Top10Change": top10_capital - front_top10_capital,
            "ChangeCapital": capital_changed,
            "IncreaseRatio": increase_ratio,
            "ReductionRatio": reduction_ratio
        }
        
    return None

def get_f10(code: str, date: str) -> F10:
    f10 = F10()
    feature_date = date # YYYY-MM-DD
    
    security_code = correct_security_code(code)
    
    # 1. Basic Info
    sec_info = checkout_security_basic_info(security_code, feature_date)
    f10.code = security_code
    f10.total_capital = sec_info["TotalCapital"]
    f10.capital = sec_info["Capital"]
    f10.vol_unit = sec_info["VolUnit"]
    f10.decimal_point = sec_info["DecimalPoint"]
    f10.security_name = sec_info["Name"]
    f10.ipo_date = sec_info["IpoDate"]
    f10.sub_new = sec_info["SubNew"]
    f10.update_date = sec_info["UpdateDate"]
    f10.margin_trading_target = is_margin_trading_target(security_code)
    
    # 2. Share Holder
    holder_info = checkout_share_holder(security_code, feature_date)
    if holder_info:
        f10.free_capital = holder_info["FreeCapital"]
        f10.top10_capital = holder_info["Top10Capital"]
        f10.top10_change = holder_info["Top10Change"]
        f10.change_capital = holder_info["ChangeCapital"]
        f10.increase_ratio = holder_info["IncreaseRatio"]
        f10.reduction_ratio = holder_info["ReductionRatio"]
        
    if f10.free_capital == 0:
        f10.free_capital = f10.capital
        
    # 3. Notice
    try:
        n = notice.get_one_notice(security_code, feature_date)
        if n:
            f10.increases = n.increase
            f10.reduces = n.reduce
            f10.risk = n.risk
            f10.risk_keywords = n.risk_keywords
    except Exception:
        pass
        
    # 4. Quarterly Report
    try:
        report = financial_report.get_quarterly_report_summary(security_code, feature_date)
        if report:
            f10.q_date = report.q_date
            f10.bps = report.bps
            f10.basic_eps = report.basic_eps
            f10.total_operate_income = report.total_operate_income
            f10.deduct_basic_eps = report.deduct_basic_eps
    except Exception:
        pass
        
    # 5. Safety Score
    try:
        score, reason = safety_score.get_safety_score(security_code)
        f10.safety_score = score
    except Exception:
        pass
        
    # 6. Report Dates
    try:
        annual_date, quarterly_date = notice.notice_date_for_report(security_code, feature_date)
        f10.annual_report_date = annual_date
        f10.quarterly_report_date = quarterly_date
    except Exception:
        pass
        
    f10.update_time = int(pd.Timestamp.now().timestamp())
    
    return f10
