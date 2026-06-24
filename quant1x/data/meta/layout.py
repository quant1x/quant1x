# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

"""
时间相关的常量定义
"""
# 时间格式

# 仅日期格式: 2022-11-28
FORMAT_ONLY_DATE = '%Y-%m-%d'
# 仅时间格式: 09:15:59
FORMAT_ONLY_TIME = '%H:%M:%S'
# 文件名中包含日期的日期格式: 20221128
FORMAT_FILE_DATE = '%Y%m%d'
# 时间戳: 2022-11-28 09:15:59
FORMAT_DATETIME = '%Y-%m-%d %H:%M:%S'
# 时间戳带毫秒数, 如果毫秒数保留前3位, 需要自己截取: 2022-11-28 09:15:59.123456
FORMAT_TIMESTAMP = '%Y-%m-%d %H:%M:%S.%f'
