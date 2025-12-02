# -*- coding: UTF-8 -*-
import sys
import os
import pandas as pd

# Ensure the project root is in sys.path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from quant1x import config
from quant1x.cache import (
    klines, stock_name, securities, block_list, sector_filename,
    get_sector_constituents, get_minutes_data, get_tick_transaction, get_f10
)

def test_cache_functions():
    print(config.get_quant1x_config_filename())
    print('data_path', config.data_path)
    print('kline_path', config.kline_path)
    code = '600600'
    df = klines(code)
    print(df)
    stock_name_val = stock_name(code)
    print(stock_name_val)
    security_list = securities()
    print(security_list)
    index_list = block_list()
    print(index_list)
    sfn = sector_filename()
    df = pd.read_csv(sfn)
    print(df)
    print(df['code'].dtype)
    df['code'] = 'sh' + df['code'].astype(str)
    s1 = df[df['code'] == 'sh881478']
    print(s1)

    l1 = get_sector_constituents('880675')
    print(l1)
    print(type(l1))

    df2 = get_minutes_data(code, date='2025-06-20')
    print(df2)
    df3 = get_tick_transaction(code, date='2025-06-20')
    print(df3)
    df4 = get_f10(code)
    print(df4)

if __name__ == '__main__':
    test_cache_functions()
