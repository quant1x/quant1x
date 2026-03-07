# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from enum import Enum, unique

@unique
class Region(Enum):
    """市场区域, 用于收敛货币和时区"""
    CN = "CN"
    HK = "HK"
    US = "US"
    UK = "UK"
    EU = "EU"
    SG = "SG"
    JP = "JP"
    OFFSHORE = "OS" # 离岸市场
    ONSHORE = "ON" # 内地市场
    GLB = "GLB" # 全球市场
    UNKNOWN = "UNKNOWN"
    
    @property
    def currency(self) -> str:
        """主要货币"""
        currencies = {
            Region.CN: "CNY",
            Region.HK: "HKD",
            Region.US: "USD",
            Region.UK: "GBP",
            Region.EU: "EUR", # 欧元区
            Region.SG: "SGD",
            Region.JP: "JPY",
            Region.OFFSHORE: "USD",
            Region.ONSHORE: "CNY",
        }
        return currencies.get(self, "USD")
    
    @property
    def timezone(self) -> str:
        """主要时区"""
        timezones = {
            Region.CN: "Asia/Shanghai",
            Region.HK: "Asia/Hong_Kong",
            Region.US: "America/New_York",
            Region.UK: "Europe/London",
            Region.EU: "Europe/Berlin",
            Region.SG: "Asia/Singapore",
            Region.JP: "Asia/Tokyo",
            Region.OFFSHORE: "America/New_York",
            Region.ONSHORE: "Asia/Shanghai",
        }
        return timezones.get(self, "UTC")
    
    