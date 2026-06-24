# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from .storage import FileStorage
from .csv import csv_to_slice, slice_to_csv
__all__ = [
    "FileStorage", # 文件存储接口
    "csv_to_slice", # 从 CSV 加载为 dataclass 实例列表
    "slice_to_csv", # 将 dataclass 实例列表保存为 CSV
]