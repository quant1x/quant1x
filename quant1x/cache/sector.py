# -*- coding: UTF-8 -*-
import os
import json
from functools import lru_cache
import pandas as pd
from .. import exchange, config
from .security import securities

# SectorFilename 板块缓存文件名
def sector_filename(date: str = '') -> str:
    """
    板块缓存文件名
    """
    name = 'blocks'
    cache_date = date.strip()
    if len(cache_date) == 0:
        cache_date = exchange.last_trade_date()
    filename = os.path.join(config.meta_path, f'{name}.{cache_date}')
    if not os.path.isfile(filename):
        # fallback to blocks.csv
        filename = os.path.join(config.meta_path, 'blocks.csv')
    return filename

@lru_cache(maxsize=None)
def get_sector_list() -> pd.DataFrame:
    """
    获取板块列表
    """
    sfn = sector_filename()
    try:
        df = pd.read_csv(sfn)
    except FileNotFoundError:
        # fallback: find latest blocks.* file under meta_path
        meta_dir = os.path.dirname(sfn)
        prefix = os.path.join(meta_dir, 'blocks.')
        candidates = [f for f in os.listdir(meta_dir) if f.startswith('blocks.')]
        if not candidates:
            raise
        # pick the latest by modification time
        candidates_full = [os.path.join(meta_dir, f) for f in candidates]
        candidates_full.sort(key=lambda p: os.path.getmtime(p), reverse=True)
        sfn2 = candidates_full[0]
        df = pd.read_csv(sfn2)
    # 补全sh前缀
    s = df['code'].astype(str)
    df['code'] = s.where(s.str.startswith('sh'), 'sh' + s)
    return df

@lru_cache(maxsize=None)
def block_list():
    """
    板块列表
    """
    df = securities()
    if df.empty or 'code' not in df.columns:
        return pd.DataFrame(columns=df.columns if not df.empty else ['code', 'name'])
    return df[df['code'].astype(str).str.startswith(('sh880', 'sh881'))]

def get_sector_constituents(code: str) -> list[str]:
    """
    获取板块成分股列表
    """
    code = code.strip()
    security_code = exchange.correct_security_code(code)
    df = get_sector_list().copy()
    cs = df[df['code'] == security_code]['ConstituentStocks']
    list = []
    if cs.empty:
        return list
    cs1 = cs.iloc[0]
    ConstituentStocks = json.loads(cs1)
    list = []
    for sc in ConstituentStocks:
        sc = sc.strip()
        sc = exchange.correct_security_code(sc)
        list.append(sc)
    return list
