# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import enum

class QuoteType(enum.Enum):
    """行情类型"""
    STANDARD  = ("L1", "std",  "standard", "标准")
    EXTENSION = ("L1", "ext",  "extension", "扩展")
    LEVEL2    = ("L2", "level2","level2", "二级")
    
    def __new__(cls, level, value, identifier, desc):
        obj = object.__new__(cls)
        obj.__setattr__("level", level)
        obj.__setattr__("_value_", value)
        obj.__setattr__("identifier", identifier)
        obj.__setattr__("desc", desc)
        return obj

# COMMAND_HEARTBEAT                = 0x0004 # 心跳维持
# COMMAND_LOGIN1                   = 0x000d # 第一次登录
# COMMAND_LOGIN2                   = 0x0fdb # 第二次登录
# COMMAND_XDXR_INFO                = 0x000f # 除权除息信息
# COMMAND_FINANCE_INFO             = 0x0010 # 财务信息
# COMMAND_PING                     = 0x0015 # 测试连接
# COMMAND_COMPANY_CATEGORY         = 0x02cf # 公司信息分类
# COMMAND_COMPANY_CONTENT          = 0x02d0 # 公司信息描述
# COMMAND_SECURITY_COUNT           = 0x044e # 证券数量
# COMMAND_SECURITY_LIST            = 0x044d # 证券列表
# COMMAND_OLD_SECURITY_LIST        = 0x0450 # 证券列表, 已废弃, 缺少北交所证券代码列表
# COMMAND_INDEX_BARS               = 0x052d # 指数K线, 废弃, 只是不同的证券类型返回不同的数据
# COMMAND_SECURITY_BARS            = 0x052d # 股票K线
# COMMAND_SECURITY_QUOTES_OLD      = 0x053e # 旧版行情信息
# COMMAND_SECURITY_QUOTES_NEW      = 0x054c # 新版行情信息
# COMMAND_MINUTE_TIME_DATA         = 0x051d # 分时数据
# COMMAND_BLOCK_META               = 0x02c5 # 板块文件信息
# COMMAND_BLOCK_DATA               = 0x06b9 # 板块文件数据
# COMMAND_TRANSACTION_DATA         = 0x0fc5 # 分笔成交信息
# COMMAND_HISTORY_MINUTE_DATA      = 0x0fb4 # 历史分时信息
# COMMAND_HISTORY_TRANSACTION_DATA = 0x0fb5 # 历史分笔成交信息

# COMMAND_EXT_HELLO            = 0x2454
# COMMAND_EXT_MARKET_LIST      = 0x23f4
# COMMAND_EXT_INSTRUMENT_INFO  = 0x23f5
# COMMAND_EXT_INSTRUMENT_COUNT = 0x23f0
# COMMAND_EXT_INSTRUMENT_BARS  = 0x23ff

class Command(enum.Enum):
    """行情指令"""
    UNKNOWN                      = (QuoteType.STANDARD, 0x0000, "未知")
    STD_SYNCHRONIZE1             = (QuoteType.STANDARD, 0x000d, "标准行情协议握手1")
    STD_SYNCHRONIZE2             = (QuoteType.STANDARD, 0x0fdb, "标准行情协议握手2")
    STD_HEARTBEAT                = (QuoteType.STANDARD, 0x0004, "心跳维持")
    STD_XDXR_INFO                = (QuoteType.STANDARD, 0x000f, "除权除息信息")
    STD_FINANCE_INFO             = (QuoteType.STANDARD, 0x0010, "财务信息")
    STD_PING                     = (QuoteType.STANDARD, 0x0015, "测试连接")
    STD_COMPANY_CATEGORY         = (QuoteType.STANDARD, 0x02cf, "公司信息分类")
    STD_COMPANY_CONTENT          = (QuoteType.STANDARD, 0x02d0, "公司信息描述")
    STD_SECURITY_COUNT           = (QuoteType.STANDARD, 0x044e, "证券数量")
    STD_SECURITY_LIST            = (QuoteType.STANDARD, 0x044d, "证券列表")
    STD_OLD_SECURITY_LIST        = (QuoteType.STANDARD, 0x0450, "证券列表(已废弃)")
    STD_SECURITY_BARS            = (QuoteType.STANDARD, 0x052d, "K线")
    STD_SECURITY_QUOTES_OLD      = (QuoteType.STANDARD, 0x053e, "旧版行情信息")
    STD_SECURITY_QUOTES_NEW      = (QuoteType.STANDARD, 0x054c, "新版行情信息")
    STD_MINUTE_TIME_DATA         = (QuoteType.STANDARD, 0x051d, "分时数据")
    STD_BLOCK_META               = (QuoteType.STANDARD, 0x02c5, "板块文件信息")
    STD_BLOCK_DATA               = (QuoteType.STANDARD, 0x06b9, "板块文件数据")
    STD_TRANSACTION_DATA         = (QuoteType.STANDARD, 0x0fc5, "分笔成交信息")
    STD_HISTORY_MINUTE_DATA      = (QuoteType.STANDARD, 0x0fb4, "历史分时信息")
    STD_HISTORY_TRANSACTION_DATA = (QuoteType.STANDARD, 0x0fb5, "历史分笔成交信息")
    
    EXT_SYNCHRONIZE              = (QuoteType.EXTENSION, 0x2454, "扩展行情协议握手")
    EXT_MARKET_LIST              = (QuoteType.EXTENSION, 0x23f4, "市场列表")
    EXT_INSTRUMENT_INFO          = (QuoteType.EXTENSION, 0x23f5, "证券列表")
    EXT_INSTRUMENT_COUNT         = (QuoteType.EXTENSION, 0x23f0, "证券数量")
    EXT_INSTRUMENT_X1            = (QuoteType.EXTENSION, 0x23fb, "即时行情")
    EXT_INSTRUMENT_BARS          = (QuoteType.EXTENSION, 0x23ff, "K线")
    
    def __new__(cls, type: QuoteType, value, desc):
        obj = object.__new__(cls)
        obj.__setattr__("type", type)
        obj.__setattr__("_value_", value & 0xffff)
        obj.__setattr__("desc", desc)
        return obj
        
FLAG_ZIP          = 0x10                         # 压缩标志
FLAG_UNCOMPRESSED = 0x0C                         # 未压缩标志
FLAG_ZIPPED       = FLAG_ZIP | FLAG_UNCOMPRESSED # 压缩标志
FLAG_GENERIC      = 0x01                         # 一般性标志
