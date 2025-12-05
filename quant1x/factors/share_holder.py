
import os
import pandas as pd
import requests
from typing import List, Dict, Tuple, Optional
from datetime import datetime
from quant1x import exchange
from quant1x.exchange import markets
from quant1x.config import config
from quant1x import std

# Constants
ERROR_CAPITAL_BASE = 90000
URL_EASTMONEY_GDFX_HOLDING_ANALYSE = "https://datacenter-web.eastmoney.com/api/data/v1/get"
EASTMONEY_GDFX_HOLDING_ANALYSE_PAGE_SIZE = 500
URL_TOP10_SHARE_HOLDER = URL_EASTMONEY_GDFX_HOLDING_ANALYSE

# HoldNumChangeState
HOLD_NUM_DAMPENED = -1       # 减少
HOLD_NUM_UNCHANGED = 0       # 不变
HOLD_NUM_NEWLY_ADDED = 1     # 新进/新增
HOLD_NUM_INCREASE = 2        # 增加
HOLD_NUM_UNKNOWN_CHANGES = -9 # 未知变化

def fetch_share_holder(security_code: str, date_str: str, diff: int = 0) -> pd.DataFrame:
    """
    Fetch top 10 circulating shareholders from Eastmoney.
    """
    _, _, code = exchange.detect_market(security_code)
    
    # Get quarter end date
    _, _, q_end = std.get_quarter_by_date(date_str, diff)
    # q_end is YYYY-MM-DD 23:59:59 in C++, here we just need YYYY-MM-DD
    # But the API expects YYYY-MM-DD
    
    params = {
        "sortColumns": "HOLDER_RANK",
        "sortTypes": "1",
        "pageSize": "10",
        "pageNumber": "1",
        "reportName": "RPT_F10_EH_FREEHOLDERS",
        "columns": "ALL",
        "source": "WEB",
        "client": "WEB",
        "filter": f'(SECURITY_CODE="{code}")(END_DATE=\'{q_end}\')'
    }
    
    try:
        response = requests.get(URL_TOP10_SHARE_HOLDER, params=params)
        if response.status_code != 200:
            return pd.DataFrame()
            
        data = response.json()
        if not data.get("success") or not data.get("result") or not data["result"].get("data"):
            return pd.DataFrame()
            
        records = []
        for v in data["result"]["data"]:
            # Determine HoldChangeState
            change_name = v.get("HOLDNUM_CHANGE_NAME", "")
            if change_name == "新进":
                change_state = HOLD_NUM_NEWLY_ADDED
            elif change_name == "增加":
                change_state = HOLD_NUM_INCREASE
            elif change_name == "减少":
                change_state = HOLD_NUM_DAMPENED
            elif change_name == "不变":
                change_state = HOLD_NUM_UNCHANGED
            else:
                change_state = HOLD_NUM_UNKNOWN_CHANGES
            
            # Normalize SecurityCode
            # In C++: auto [_, mflag, mcode] = exchange::DetectMarket(shareholder.SecurityCode);
            # shareholder.SecurityCode = mflag + mcode;
            raw_code = v.get("SECUCODE", "")
            normalized_code = exchange.correct_security_code(raw_code)

            record = {
                "SecurityCode": normalized_code,
                "SecurityName": v.get("SECURITY_NAME_ABBR", ""),
                "EndDate": pd.to_datetime(v.get("END_DATE")).strftime("%Y-%m-%d"),
                "UpdateDate": pd.to_datetime(v.get("UPDATE_DATE")).strftime("%Y-%m-%d"),
                "HolderType": v.get("HOLDER_NEWTYPE", ""),
                "HolderName": v.get("HOLDER_NAME", ""),
                "IsHoldOrg": v.get("IS_HOLDORG", ""),
                "HolderRank": v.get("HOLDER_RANK", 0),
                "HoldNum": v.get("HOLD_NUM", 0),
                "FreeHoldNumRatio": v.get("FREE_HOLDNUM_RATIO", 0.0),
                "HoldNumChange": v.get("XZCHANGE", 0),
                "HoldChangeName": v.get("HOLDNUM_CHANGE_NAME", ""),
                "HoldChangeState": change_state,
                "HoldChangeRatio": v.get("CHANGE_RATIO", 0.0),
                "HoldRatio": v.get("HOLD_RATIO", 0.0),
                "HoldRatioChange": v.get("HOLD_RATIO_CHANGE", 0.0)
            }
            records.append(record)
            
        df = pd.DataFrame(records)
        if not df.empty:
            df = df.sort_values(by="HolderRank")
            
        return df
        
    except Exception as e:
        print(f"[share-holder] Error fetching data: {e}")
        return pd.DataFrame()

def cache_share_holder(security_code: str, date_str: str, diff: int = 1) -> pd.DataFrame:
    """
    Get share holder data from cache or fetch if missing.
    """
    _, _, last = std.get_quarter_by_date(date_str, diff)
    filename = config.top10_holders_filename(security_code, last)
    
    if os.path.exists(filename):
        try:
            df = pd.read_csv(filename)
            if not df.empty:
                return df
        except Exception:
            pass
            
    # Fetch from API
    df = fetch_share_holder(security_code, last)
    if not df.empty:
        # Save to cache
        os.makedirs(os.path.dirname(filename), exist_ok=True)
        df.to_csv(filename, index=False)
        
    return df

def get_cache_share_holder(security_code: str, date_str: str, diff: int = 1) -> pd.DataFrame:
    """
    Get share holder data, retrying with previous quarters if empty.
    """
    for d in range(diff, 4):
        df = cache_share_holder(security_code, date_str, d)
        if not df.empty:
            return df
            
    return pd.DataFrame()
