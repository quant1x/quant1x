#!/usr/bin/python
# -*- coding: UTF-8 -*-

"""
基础工具集
"""

__author__ = 'WangFeng'

from .app import application
from .device import cpu_num, max_procs
from .devp import project_path, redirect
from .file import mkdirs, touch, homedir
from .logger import logger
from .network import ip_is_private, ip_is_secure, lan_address
from .numerics import is_nan, float_round, fix_float
from .pattern import Singleton
from .timestamp import TimeRange, FORMAT_DATETIME
from .timestamp import TradingSession, seconds_to_timestamp
