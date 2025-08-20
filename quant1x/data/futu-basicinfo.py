#!/usr/bin/python
# -*- coding: UTF-8 -*-
"""
证券基本信息
"""
from futu import *

quote_ctx = OpenQuoteContext(host='127.0.0.1', port=11111)
print('******************************************')
ret, data = quote_ctx.get_stock_basicinfo(Market.SH, SecurityType.STOCK)
if ret == RET_OK:
    data.to_csv('sh.csv', index=False)
else:
    print('error:', data)
print('******************************************')
ret, data = quote_ctx.get_stock_basicinfo(Market.SZ, SecurityType.STOCK)
if ret == RET_OK:
    data.to_csv('sz.csv', index=False)
else:
    print('error:', data)
ret, data = quote_ctx.get_stock_basicinfo(Market.HK, SecurityType.STOCK)
if ret == RET_OK:
    data.to_csv('hk.csv', index=False)
else:
    print('error:', data)

quote_ctx.close()  # 结束后记得关闭当条连接，防止连接条数用尽
