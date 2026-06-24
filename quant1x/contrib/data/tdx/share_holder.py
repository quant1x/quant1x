# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

"""
前十大流通股股东数据获取 (东方财富API)

从 factors/share_holder.py 迁移, 去掉对 quant1x.exchange 的依赖,
改用 quant1x.data.market.detect_symbol 和 data.meta.timestamp. 
"""

import os
import pandas as pd
import requests
from typing import List, Dict, Tuple, Optional

from quant1x.config import top10_holders_filename
from quant1x.data.market import detect_symbol
from quant1x.log import logger

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
    """Fetch top 10 circulating shareholders from Eastmoney."""
    from quant1x.std.time import get_quarter_by_date

    inst = detect_symbol(security_code)
    code = inst.ticker

    # Get quarter end date
    _, _, q_end = get_quarter_by_date(date_str, diff)

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
        response = requests.get(URL_TOP10_SHARE_HOLDER, params=params, timeout=10)
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
            raw_code = v.get("SECUCODE", "")
            from quant1x.data.market import correct_security_code
            normalized_code = correct_security_code(raw_code)

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
        logger.debug(f"[share-holder] Error fetching data: {e}")
        return pd.DataFrame()


def cache_share_holder(security_code: str, date_str: str, diff: int = 1) -> pd.DataFrame:
    """Get share holder data from cache or fetch if missing."""
    from quant1x.std.time import get_quarter_by_date

    _, _, last = get_quarter_by_date(date_str, diff)
    filename = top10_holders_filename(security_code, last)

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
    """Get share holder data, retrying with previous quarters if empty."""
    for d in range(diff, 4):
        df = cache_share_holder(security_code, date_str, d)
        if not df.empty:
            return df

    return pd.DataFrame()
