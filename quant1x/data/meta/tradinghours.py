# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

PRE_MARKET_HOUR = 9
PRE_MARKET_MINUTE = 0
PRE_MARKET_SECOND = 0
cn_cron_expr_daily_init = f"0 {PRE_MARKET_HOUR} {PRE_MARKET_MINUTE} * * *"

__all__ = [
    "PRE_MARKET_HOUR",
    "PRE_MARKET_MINUTE",
    "PRE_MARKET_SECOND",
    "cn_cron_expr_daily_init",
]