# -*- coding: UTF-8 -*-
import os
import time
import threading
import requests
import pandas as pd
from typing import List, Dict, Tuple, Optional

from quant1x import config, exchange
from quant1x.exchange import Timestamp

# Constants
MARGIN_TRADING_FILENAME = "margin-trading.csv"
URL_EASTMONEY_API_RZRQ = "https://datacenter-web.eastmoney.com/api/data/v1/get"
RZRQ_PAGE_SIZE = 500

# Cache
_margin_trading_cache_list: List[str] = []
_margin_trading_cache_map: Dict[str, bool] = {}
_margin_trading_cache_mutex = threading.Lock()
_margin_trading_loaded = False

def raw_margin_trading_list(date_str: str, page_number: int) -> Tuple[List[Dict], int, str]:
    """
    Fetch raw margin trading data from EastMoney API.
    """
    params = {
        "reportName": "RPTA_WEB_RZRQ_GGMX",
        "columns": "ALL",
        "source": "WEB",
        "sortColumns": "scode",
        "sortTypes": "1",
        "pageSize": str(RZRQ_PAGE_SIZE),
        "pageNumber": str(page_number),
        "client": "WEB",
        "filter": f"(DATE='{date_str}')"
    }

    try:
        response = requests.get(URL_EASTMONEY_API_RZRQ, params=params, timeout=10)
        if response.status_code != 200:
            return [], 0, f"HTTP request failed with code: {response.status_code}"
        
        data = response.json()
        
        if not data.get("success", False):
            return [], 0, data.get("message", "Unknown error")
            
        result = data.get("result", {})
        if not result:
             return [], 0, "No result data"
             
        pages = result.get("pages", 0)
        data_list = result.get("data", [])
        
        return data_list, pages, ""
        
    except Exception as e:
        return [], 0, str(e)

def get_margin_trading_date() -> str:
    """
    Get the date for margin trading data (previous trading day).
    """
    ts = exchange.prev_trading_day()
    return ts.only_date()

def get_margin_trading_list() -> List[Dict]:
    """
    Fetch all margin trading data for the target date.
    """
    date_str = get_margin_trading_date()
    full_list = []
    pages = 1
    
    i = 0
    while i < pages:
        tmp_list, tmp_pages, err = raw_margin_trading_list(date_str, i + 1)
        if err:
            # Log error?
            break
            
        full_list.extend(tmp_list)
        
        if len(tmp_list) < RZRQ_PAGE_SIZE:
            break
            
        if pages == 1:
            pages = tmp_pages
            
        i += 1
        
    return full_list

def lazy_load_margin_trading():
    global _margin_trading_loaded, _margin_trading_cache_list, _margin_trading_cache_map
    
    with _margin_trading_cache_mutex:
        if _margin_trading_loaded:
            # Check if we need to reload? 
            # For now, assume loaded once per session is enough or rely on file mtime check if we want to be strict.
            # But C++ uses RollingOnce with daily 9am reset.
            # Here we just load once.
            return

        cache_filename = os.path.join(config.meta_path, MARGIN_TRADING_FILENAME)
        
        # Check file mtime
        last_modified = 0
        if os.path.exists(cache_filename):
            last_modified = os.path.getmtime(cache_filename)
            
        cache_timestamp = Timestamp(int(last_modified))
        last_trading_day_ts = exchange.last_trading_day()
        
        temp_list = []
        
        # If cache is old, download new data
        if cache_timestamp.only_date() < last_trading_day_ts.only_date():
            data_list = get_margin_trading_list()
            
            if data_list:
                # Save to CSV
                df = pd.DataFrame(data_list)
                # Ensure columns order if needed, or just save all
                # C++ writes specific columns. Let's try to match if possible, but pandas handles it.
                # C++ columns: DATE, MARKET, SCODE, SECNAME, ...
                
                # Normalize SECUCODE
                df['SECUCODE'] = df['SECUCODE'].apply(exchange.correct_security_code)
                
                try:
                    os.makedirs(os.path.dirname(cache_filename), exist_ok=True)
                    df.to_csv(cache_filename, index=False)
                    temp_list = df.to_dict('records')
                except Exception as e:
                    print(f"[margin-trading] Failed to write cache: {e}")
        
        # If temp_list is empty (download failed or not needed), try to load from cache
        if not temp_list and os.path.exists(cache_filename):
            try:
                df = pd.read_csv(cache_filename)
                # Normalize SECUCODE just in case
                if 'SECUCODE' in df.columns:
                    df['SECUCODE'] = df['SECUCODE'].apply(exchange.correct_security_code)
                    temp_list = df.to_dict('records')
            except Exception as e:
                print(f"[margin-trading] Failed to read cache: {e}")
                
        # Update cache list and map
        codes = []
        for item in temp_list:
            code = item.get('SECUCODE', '')
            if code:
                codes.append(code)
                
        # Unique and sort
        codes = sorted(list(set(codes)))
        
        _margin_trading_cache_list = codes
        _margin_trading_cache_map = {code: True for code in codes}
        _margin_trading_loaded = True

def margin_trading_list() -> List[str]:
    """
    Get list of margin trading target codes.
    """
    lazy_load_margin_trading()
    return _margin_trading_cache_list

def is_margin_trading_target(code: str) -> bool:
    """
    Check if a security code is a margin trading target.
    """
    lazy_load_margin_trading()
    security_code = exchange.correct_security_code(code)
    return _margin_trading_cache_map.get(security_code, False)
