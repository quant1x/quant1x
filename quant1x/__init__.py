#!/usr/bin/python
# -*- coding: UTF-8 -*-

"""
89K 努力打造一个适度的量化交易框架
"""

__version__ = "0.3.0"
__author__ = "WangFeng"

import sys

if sys.version_info < (3, 8):
    print(f"Quant1X {__version__} requires Python 3.8+ and 64 bit OS")
    sys.exit(1)
del sys


def init(**kwargs):
    pass
