#!/usr/bin/python
# -*- coding: UTF-8 -*-

from quant1x.data import D

# 1. 更新分笔数据
# D.update_tick()
# 2. 更新估值分析数据
D.update_forecast()
# 3. 更新 除权信息
D.update_xdxr()
# 4. 更新 基本信息
D.update_info()
# 5. 更新 K线数据
D.update_history()
