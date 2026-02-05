# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

COMMAND_HEARTBEAT                = 0x0004 # 心跳维持
COMMAND_LOGIN1                   = 0x000d # 第一次登录
COMMAND_LOGIN2                   = 0x0fdb # 第二次登录
COMMAND_XDXR_INFO                = 0x000f # 除权除息信息
COMMAND_FINANCE_INFO             = 0x0010 # 财务信息
COMMAND_PING                     = 0x0015 # 测试连接
COMMAND_COMPANY_CATEGORY         = 0x02cf # 公司信息分类
COMMAND_COMPANY_CONTENT          = 0x02d0 # 公司信息描述
COMMAND_SECURITY_COUNT           = 0x044e # 证券数量
COMMAND_SECURITY_LIST            = 0x044d # 证券列表
COMMAND_OLD_SECURITY_LIST        = 0x0450 # 证券列表, 已废弃, 缺少北交所证券代码列表
COMMAND_INDEX_BARS               = 0x052d # 指数K线
COMMAND_SECURITY_BARS            = 0x052d # 股票K线
COMMAND_SECURITY_QUOTES_OLD      = 0x053e # 旧版行情信息
COMMAND_SECURITY_QUOTES_NEW      = 0x054c # 新版行情信息
COMMAND_MINUTE_TIME_DATA         = 0x051d # 分时数据
COMMAND_BLOCK_META               = 0x02c5 # 板块文件信息
COMMAND_BLOCK_DATA               = 0x06b9 # 板块文件数据
COMMAND_TRANSACTION_DATA         = 0x0fc5 # 分笔成交信息
COMMAND_HISTORY_MINUTE_DATA      = 0x0fb4 # 历史分时信息
COMMAND_HISTORY_TRANSACTION_DATA = 0x0fb5 # 历史分笔成交信息

FLAG_ZIP          = 0x10                         # 压缩标志
FLAG_UNCOMPRESSED = 0x0C                         # 未压缩标志
FLAG_ZIPPED       = FLAG_ZIP | FLAG_UNCOMPRESSED # 压缩标志
