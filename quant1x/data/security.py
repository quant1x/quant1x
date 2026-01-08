# -*- coding: UTF-8 -*-
import os
from functools import lru_cache
import pandas as pd
from .. import exchange, config

@lru_cache(maxsize=None)
def securities() -> pd.DataFrame:
    """
    证券列表
    """
    full_path = os.path.join(config.meta_path, 'securities.csv')
    if not os.path.isfile(full_path):
        return pd.DataFrame(columns=['code', 'name'])
    # with open(full_path, 'rb') as f:
    #     f.seek(87287 - 13)
    #     print('Bytes around 85710:', f.read(19).hex(' '))
    df = pd.read_csv(full_path)
    # 转换为小写
    df.columns = df.columns.str.lower()
    # 兼容多种列名：优先匹配 'code' 和 'name'，否则尝试常见替代列名，最终回退到前两列
    cols = list(df.columns)
    code_candidates = ['code', 'symbol', 'securitycode', 'security_code', 'sec_code', 'sid']
    name_candidates = ['name', 'sec_name', 'security_name', 'secname', 'stock_name']

    def find_first(candidates, cols):
        for c in candidates:
            if c in cols:
                return c
        return None

    code_col = find_first(code_candidates, cols) or (cols[0] if cols else None)
    name_col = find_first(name_candidates, cols) or (cols[1] if len(cols) > 1 else (cols[0] if cols else None))

    if code_col is None or name_col is None:
        return pd.DataFrame(columns=['code', 'name'])

    # 返回时保持列名为标准名称 'code' 和 'name'，以便后续代码不变
    out = df[[code_col, name_col]].copy()
    out.columns = ['code', 'name']
    return out

def stock_name(code: str) -> str:
    corrected_symbol = exchange.correct_security_code(code)
    df = securities()
    if df.empty:
        return ""
    # ensure types align
    try:
        tmp = df[df['code'].astype(str) == str(corrected_symbol)]
    except Exception:
        tmp = df[df['code'] == corrected_symbol]
    if tmp.empty:
        return ""
    name = tmp['name'].iloc[0]
    return name
