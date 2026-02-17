# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
from __future__ import annotations
from typing import List
from dataclasses import dataclass, field
from enum import Enum

from quant1x.std.numeric import NumberRange

from .meta.exchange import Exchange
from .meta.instrument import Instrument, InstrumentType
# 证券代码规则定义

@dataclass
class CodeRule:
    """证券代码规则"""
    exchange: Exchange       # 交易所
    prefix: str |NumberRange # 代码前缀
    type: InstrumentType     # 证券类型
    name: str                # 证券类型名称
    desc: str                # 规则描述

# 全局规则
global_rules = [
    CodeRule(Exchange.SSE, "880", InstrumentType.SECTOR, "板块指数", "通达信"),
    CodeRule(Exchange.SSE, "881", InstrumentType.SECTOR, "板块指数", "通达信"),
]

# SSE 上海证券交易所规则
sse_rules = [
    CodeRule(Exchange.SSE, "000", InstrumentType.INDEX, "上证指数", "上证指数系列; 000680-000689 用于科创板相关指数"),
    CodeRule(Exchange.SSE, "009", InstrumentType.BOND, "国债", "国债(2000年前发行)"),
    CodeRule(Exchange.SSE, "010", InstrumentType.BOND, "国债", "国债(2000-2009年发行)"),
    CodeRule(Exchange.SSE, "018", InstrumentType.BOND, "政策性银行债", "政策性银行金融债"),
    CodeRule(Exchange.SSE, "019", InstrumentType.BOND, "国债", "国债(2010年及以后发行)"),
    CodeRule(Exchange.SSE, "020", InstrumentType.BOND, "记账式贴现国债", "记账式贴现国债"),
    CodeRule(Exchange.SSE, "090", InstrumentType.BOND, "国债质押回购出入库", "国债质押式回购质押券出入库"),
    CodeRule(Exchange.SSE, "091", InstrumentType.BOND, "国债质押回购出入库", "对应019***"),
    CodeRule(Exchange.SSE, "099", InstrumentType.BOND, "国债质押回购出入库", "对应009***"),
    
    CodeRule(Exchange.SSE, "0", InstrumentType.INDEX, "指数/国债", "首位 0: 指数、国债"),

    CodeRule(Exchange.SSE, "100", InstrumentType.BOND, "债券回售/可转债", "100000-100899 用于可转换公司债券(对应600***); 100900-100999 用于债券回售(不再增用部分)"),
    CodeRule(Exchange.SSE, "101", InstrumentType.BOND, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "102", InstrumentType.BOND, "企业债质押出入库", "对应127000-127999"),
    CodeRule(Exchange.SSE, "103", InstrumentType.BOND, "企业债质押出入库", "对应124000-124999"),
    CodeRule(Exchange.SSE, "104", InstrumentType.BOND, "公司/企业债质押出入库", "104000-104499 用于公司债质押(对应122000-122499); 104500-104999 用于企业债质押(对应122500-122999)"),
    CodeRule(Exchange.SSE, "105", InstrumentType.BOND, "债券质押出入库", "105000-105699 分离交易的可转债质押(对应126***); 105700-105799 债券ETF质押; 105800-105899 可转债质押(对应110***、113***); 105900-105999 企业债质押(对应120***、129***)"),
    CodeRule(Exchange.SSE, "106", InstrumentType.BOND, "地方政府债质押出入库", "对应130***"),
    CodeRule(Exchange.SSE, "107", InstrumentType.BOND, "记账式贴现国债质押出入库", "对应020***"),
    CodeRule(Exchange.SSE, "108", InstrumentType.BOND, "政策性银行债质押出入库", "对应018***"),
    CodeRule(Exchange.SSE, "109", InstrumentType.BOND, "地方政府债", "地方政府债券"),

    CodeRule(Exchange.SSE, "110", InstrumentType.BOND, "可转换公司债", "110000-110799 上市公司公开发行可转债(对应600***); 110800-110999 非公开发行"),
    CodeRule(Exchange.SSE, "111", InstrumentType.BOND, "可转换公司债", "111000-111499 对应605***"),
    CodeRule(Exchange.SSE, "112", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "113", InstrumentType.BOND, "可转换公司债", "113000-113499 对应601***; 113500-113999 对应603***"),
    CodeRule(Exchange.SSE, "114", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "115", InstrumentType.BOND, "公开公司债", "公开发行公司债券"),
    CodeRule(Exchange.SSE, "118", InstrumentType.BOND, "科创板可转债", "118000-118499 用于科创板上市公司公开发行可转债"),

    CodeRule(Exchange.SSE, "120", InstrumentType.BOND, "企业/公司债", "122000-122499 用于公司债券; 122500-122999 用于企业债券(见122)"),
    CodeRule(Exchange.SSE, "121", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "122", InstrumentType.BOND, "公司债/企业债", "122000-122499 用于公司债券; 122500-122999 用于企业债券"),
    CodeRule(Exchange.SSE, "123", InstrumentType.BOND, "公司/企业债/ABS", "123000-123499 用于企业/公司债; 123500-123999 用于资产支持证券"),
    CodeRule(Exchange.SSE, "124", InstrumentType.BOND, "企业债质押出入库", "对应124000-124999"),
    CodeRule(Exchange.SSE, "125", InstrumentType.BOND, "中小企业私募债/非公开公司债", "中小企业私募债券、非公开发行公司债券"),
    CodeRule(Exchange.SSE, "126", InstrumentType.BOND, "分离交易可转债", "分离交易的可转换公司债券"),
    CodeRule(Exchange.SSE, "127", InstrumentType.BOND, "企业债", "127000-127899 用于企业债券; 127900-127999 用于政府支持债(中国铁路建设债专用)"),
    CodeRule(Exchange.SSE, "128", InstrumentType.BOND, "信贷资产支持证券", "信贷资产支持证券"),
    CodeRule(Exchange.SSE, "129", InstrumentType.BOND, "企业债", "企业债券"),

    CodeRule(Exchange.SSE, "130", InstrumentType.BOND, "地方政府债", "地方政府债券(对应130***)"),
    CodeRule(Exchange.SSE, "131", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "132", InstrumentType.BOND, "可交换公司债", "可交换公司债券"),
    CodeRule(Exchange.SSE, "133", InstrumentType.BOND, "可交换债质押出入库", "对应132***"),
    CodeRule(Exchange.SSE, "134", InstrumentType.BOND, "公开公司债质押出入库", "对应136***"),
    CodeRule(Exchange.SSE, "135", InstrumentType.BOND, "证券公司短期债/并购私募债", "证券公司短期债、并购重组私募债券、非公开发行公司债券"),
    CodeRule(Exchange.SSE, "136", InstrumentType.BOND, "公开公司债质押出入库", "对应136***"),
    CodeRule(Exchange.SSE, "137", InstrumentType.BOND, "可交换/公开公司债", "137000-137499 非公开可交换; 137500-137999 公开公司债"),
    CodeRule(Exchange.SSE, "138", InstrumentType.BOND, "可交换换股/公开公司债", "138000-138499 非公开可交换换股(对应137000-137499); 138500-138999 公开公司债"),
    CodeRule(Exchange.SSE, "139", InstrumentType.BOND, "企业债", "企业债券"),

    CodeRule(Exchange.SSE, "140", InstrumentType.BOND, "地方政府债质押出入库", "对应140***"),
    CodeRule(Exchange.SSE, "141", InstrumentType.BOND, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "142", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "143", InstrumentType.BOND, "公开公司债质押出入库", "对应143***"),
    CodeRule(Exchange.SSE, "144", InstrumentType.BOND, "公开公司债", "公开发行公司债券"),
    CodeRule(Exchange.SSE, "145", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "146", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "147", InstrumentType.BOND, "地方政府债质押出入库", "对应147***"),
    CodeRule(Exchange.SSE, "148", InstrumentType.BOND, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "149", InstrumentType.BOND, "资产支持证券", "资产支持证券"),

    CodeRule(Exchange.SSE, "150", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "151", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "152", InstrumentType.BOND, "企业债质押出入库", "对应152***"),
    CodeRule(Exchange.SSE, "153", InstrumentType.BOND, "企业债", "企业债券"),
    CodeRule(Exchange.SSE, "154", InstrumentType.BOND, "公司债质押出入库", "对应155***"),
    CodeRule(Exchange.SSE, "155", InstrumentType.BOND, "公司债质押出入库", "对应155***"),
    CodeRule(Exchange.SSE, "156", InstrumentType.BOND, "公司债", "公司债券"),
    CodeRule(Exchange.SSE, "157", InstrumentType.BOND, "地方政府债质押出入库", "对应157***"),
    CodeRule(Exchange.SSE, "158", InstrumentType.BOND, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "159", InstrumentType.BOND, "资产支持证券", "资产支持证券"),

    CodeRule(Exchange.SSE, "160", InstrumentType.BOND, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "161", InstrumentType.BOND, "地方政府债质押出入库", "对应160***"),
    CodeRule(Exchange.SSE, "162", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "163", InstrumentType.BOND, "公开公司债质押出入库", "对应163***"),
    CodeRule(Exchange.SSE, "164", InstrumentType.BOND, "公开公司债", "公开发行公司债券"),
    CodeRule(Exchange.SSE, "165", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "166", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "167", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "168", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "169", InstrumentType.BOND, "资产支持证券", "资产支持证券"),

    CodeRule(Exchange.SSE, "170", InstrumentType.BOND, "信用保护工具", "170000-170499 用于信用保护凭证; 170900-170999 用于组合型信用保护合约"),
    CodeRule(Exchange.SSE, "171", InstrumentType.BOND, "地方政府债质押出入库", "对应171***"),
    CodeRule(Exchange.SSE, "172", InstrumentType.BOND, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "173", InstrumentType.BOND, "地方政府债质押出入库", "对应173***"),
    CodeRule(Exchange.SSE, "174", InstrumentType.BOND, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "175", InstrumentType.BOND, "公开公司债质押出入库", "对应175***"),
    CodeRule(Exchange.SSE, "176", InstrumentType.BOND, "公开公司债", "公开发行公司债券"),
    CodeRule(Exchange.SSE, "177", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "178", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "179", InstrumentType.BOND, "资产支持证券", "资产支持证券"),

    CodeRule(Exchange.SSE, "180", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "181", InstrumentType.BOND, "可转债转股/非公开公司债", "对应600*** 的转股等/182000 系列为回售或非公开"),
    CodeRule(Exchange.SSE, "182", InstrumentType.BOND, "债券回售/非公开公司债", "182000-182299 用于债券回售; 182300-182999 用于非公开发行公司债券"),
    CodeRule(Exchange.SSE, "183", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "184", InstrumentType.BOND, "企业债/政府支持债", "184000-184799 企业债券; 184800-184999 政府支持债(中国铁路建设债专用)"),
    CodeRule(Exchange.SSE, "185", InstrumentType.BOND, "公开公司债", "公开发行公司债券"),
    CodeRule(Exchange.SSE, "186", InstrumentType.BOND, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "187", InstrumentType.BOND, "公开公司债质押出入库", "对应188***"),
    CodeRule(Exchange.SSE, "188", InstrumentType.BOND, "公开公司债质押出入库", "对应188***"),
    CodeRule(Exchange.SSE, "189", InstrumentType.BOND, "资产支持证券", "资产支持证券"),

    CodeRule(Exchange.SSE, "190", InstrumentType.BOND, "可转债转股", "对应600***"),
    CodeRule(Exchange.SSE, "191", InstrumentType.BOND, "可转债转股", "191000-191499 对应601***; 191500-191999 对应603***"),
    CodeRule(Exchange.SSE, "192", InstrumentType.BOND, "可交换债换股", "对应132***"),
    CodeRule(Exchange.SSE, "193", InstrumentType.BOND, "创新创业转股/ABS", "193000-193099 创新创业公司非公开可转债转股(对应145900-145999); 193100-193999 用于资产支持证券"),
    CodeRule(Exchange.SSE, "194", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "195", InstrumentType.BOND, "可转债转股", "195000-195499 用于可转债转股，对应605***"),
    CodeRule(Exchange.SSE, "196", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "197", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "198", InstrumentType.BOND, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "199", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    
    CodeRule(Exchange.SSE, "1", InstrumentType.BOND, "债券现券", "首位 1: 债券现券"),

    CodeRule(Exchange.SSE, "201", InstrumentType.BOND, "国债回购", "国债回购(席位托管方式)"),
    CodeRule(Exchange.SSE, "202", InstrumentType.BOND, "企业债回购", "企业债回购(席位托管方式)"),
    CodeRule(Exchange.SSE, "203", InstrumentType.BOND, "国债买断式回购", "国债买断式回购"),
    CodeRule(Exchange.SSE, "204", InstrumentType.BOND, "债券质押式回购(账户托管)", "债券质押式回购(账户托管方式)"),
    CodeRule(Exchange.SSE, "205", InstrumentType.BOND, "质押式报价回购", "质押式报价回购"),
    CodeRule(Exchange.SSE, "206", InstrumentType.BOND, "质押式协议回购", "债券质押式协议回购"),
    CodeRule(Exchange.SSE, "207", InstrumentType.BOND, "质押式三方回购", "债券质押式三方回购"),
    CodeRule(Exchange.SSE, "208", InstrumentType.BOND, "债券借贷", "208000-208009 用于债券借贷业务"),

    CodeRule(Exchange.SSE, "230", InstrumentType.BOND, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "231", InstrumentType.BOND, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "232", InstrumentType.BOND, "地方政府债", "地方政府债券"),
    CodeRule(Exchange.SSE, "233", InstrumentType.BOND, "地方政府债", "地方政府债券"),

    CodeRule(Exchange.SSE, "240", InstrumentType.BOND, "公开公司债", "公开发行公司债券"),
    CodeRule(Exchange.SSE, "241", InstrumentType.BOND, "公开公司债", "公开发行公司债券"),

    CodeRule(Exchange.SSE, "250", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "251", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "252", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "253", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "254", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "255", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "256", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),
    CodeRule(Exchange.SSE, "257", InstrumentType.BOND, "非公开公司债", "非公开发行公司债券"),

    CodeRule(Exchange.SSE, "260", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "261", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "262", InstrumentType.BOND, "资产支持证券", "资产支持证券"),
    CodeRule(Exchange.SSE, "263", InstrumentType.BOND, "资产支持证券", "资产支持证券"),

    CodeRule(Exchange.SSE, "270", InstrumentType.BOND, "企业债", "企业债券"),
    CodeRule(Exchange.SSE, "271", InstrumentType.BOND, "企业债", "企业债券"),
    CodeRule(Exchange.SSE, "272", InstrumentType.BOND, "企业债", "企业债券"),
    
    CodeRule(Exchange.SSE, "2", InstrumentType.BOND, "债券回购/借贷", "首位 2: 债券回购、债券借贷等"),

    CodeRule(Exchange.SSE, "310", InstrumentType.BOND, "国债期货", "国债期货(已暂停)"),
    CodeRule(Exchange.SSE, "330", InstrumentType.IPO, "优先股(公开)", "公开发行优先股"),
    CodeRule(Exchange.SSE, "360", InstrumentType.OTHER, "非公开优先股", "非公开发行优先股"),
    
    CodeRule(Exchange.SSE, "3", InstrumentType.OTHER, "优先股/国债期货", "首位 3: 优先股、国债期货(已暂停)"),
    
    CodeRule(Exchange.SSE, "4", InstrumentType.OTHER, "备用", "首位 4: 备用"),
    
    CodeRule(Exchange.SSE, "500", InstrumentType.FUND, "封闭式基金", "契约型封闭式基金"),
    CodeRule(Exchange.SSE, "501", InstrumentType.FUND, "上市开放式基金", "上市开放式基金"),
    CodeRule(Exchange.SSE, "502", InstrumentType.FUND, "上市开放式基金", "上市开放式基金"),
    CodeRule(Exchange.SSE, "505", InstrumentType.FUND, "创新封闭式基金", "505800-505899 用于创新型封闭式证券投资基金"),
    CodeRule(Exchange.SSE, "506", InstrumentType.FUND, "科创板LOF", "506000-506099 用于科创板相关 LOF"),
    CodeRule(Exchange.SSE, "508", InstrumentType.FUND, "公募REITs", "508000-508099 用于公募 REITs"),
    CodeRule(Exchange.SSE, "510", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为沪市指数、跨市场指数或跨境指数"),
    CodeRule(Exchange.SSE, "511", InstrumentType.ETF, "债券ETF/货基", "511000-511299 单市场债券(沪)ETF; 511300-511599 现金申赎类债券ETF; 511600-511999 交易型货币基金"),
    CodeRule(Exchange.SSE, "512", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "513", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨境指数"),
    CodeRule(Exchange.SSE, "515", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "516", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "517", InstrumentType.ETF, "跨市场股票ETF", "517000-517999 用于跨市场股票(沪港深京)ETF"),
    CodeRule(Exchange.SSE, "518", InstrumentType.ETF, "商品交易型开放式证券投资基金", "商品类 ETF"),
    CodeRule(Exchange.SSE, "519", InstrumentType.FUND, "开放式基金申赎/认购", "519*** 系列用于开放式基金的申赎/认购/跨市场转托管/分红/转换等; 5198** 用于实时申赎货币基金(实时申赎)"),
    CodeRule(Exchange.SSE, "520", InstrumentType.ETF, "跨境ETF", "520500-520999 用于跨境 ETF"),
    CodeRule(Exchange.SSE, "521", InstrumentType.FUND, "开放式基金认购", "对应519*** 系列的认购业务"),
    CodeRule(Exchange.SSE, "522", InstrumentType.FUND, "开放式基金跨市场转托管", "对应519*** 系列的跨市场转托管业务"),
    CodeRule(Exchange.SSE, "523", InstrumentType.FUND, "开放式基金分红", "对应519*** 系列的分红业务"),
    CodeRule(Exchange.SSE, "524", InstrumentType.FUND, "开放式基金基金转换", "对应519*** 系列的基金转换业务"),
    CodeRule(Exchange.SSE, "530", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为沪市指数"),
    CodeRule(Exchange.SSE, "550", InstrumentType.FUND, "基金", ""),
    CodeRule(Exchange.SSE, "560", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "561", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "562", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "563", InstrumentType.ETF, "交易型开放式指数证券投资基金", "标的指数为跨市场指数"),
    CodeRule(Exchange.SSE, "580", InstrumentType.WARRANT, "权证", "含股改权证、公司权证"),
    CodeRule(Exchange.SSE, "582", InstrumentType.WARRANT, "权证行权", "用于权证行权/行权相关代码"),
    CodeRule(Exchange.SSE, "588", InstrumentType.ETF, "科创板ETF", "588000-588299 单市场(科创板)ETF; 588300-588699 跨市场(含科创板)ETF; 588700-588999 单市场(科创板)ETF"),
    
    CodeRule(Exchange.SSE, "5", InstrumentType.FUND, "基金/REITs/权证", "首位 5: 基金、公募 REITs、权证"),
    
    CodeRule(Exchange.SSE, "600", InstrumentType.STOCK, "主板A股", "主板 A 股"),
    CodeRule(Exchange.SSE, "601", InstrumentType.STOCK, "主板A股", "主板 A 股"),
    CodeRule(Exchange.SSE, "603", InstrumentType.STOCK, "主板A股", "主板 A 股"),
    CodeRule(Exchange.SSE, "605", InstrumentType.STOCK, "主板A股", "主板 A 股(配套号段)"),
    CodeRule(Exchange.SSE, "688", InstrumentType.STOCK, "科创板", "科创板股票"),
    CodeRule(Exchange.SSE, "689", InstrumentType.STOCK, "科创板存托凭证", "科创板存托凭证"),
    
    CodeRule(Exchange.SSE, "6", InstrumentType.STOCK, "A股/存托凭证", "首位 6: A 股、存托凭证"),
    
    CodeRule(Exchange.SSE, "700", InstrumentType.OTHER, "配股", "配股(对应600***)"),
    CodeRule(Exchange.SSE, "701", InstrumentType.OTHER, "转配股", "转配股"),
    CodeRule(Exchange.SSE, "702", InstrumentType.OTHER, "职工股配股", "对应600***"),
    CodeRule(Exchange.SSE, "703", InstrumentType.OTHER, "配售", "配售"),
    CodeRule(Exchange.SSE, "704", InstrumentType.OTHER, "可转债配债", "可转换公司债券持股配债(对应600***)"),
    CodeRule(Exchange.SSE, "706", InstrumentType.OTHER, "要约收购/现金选择权", "706000-706599 主板; 706600-706999 科创板"),
    CodeRule(Exchange.SSE, "707", InstrumentType.OTHER, "网上按市值申购/增发", "对应605***"),
    CodeRule(Exchange.SSE, "708", InstrumentType.OTHER, "网上按市值申购配号", "对应605***"),
    CodeRule(Exchange.SSE, "713", InstrumentType.OTHER, "可转债申购", "对应605***"),
    CodeRule(Exchange.SSE, "714", InstrumentType.OTHER, "可转债申购配号", "对应605***"),
    CodeRule(Exchange.SSE, "715", InstrumentType.OTHER, "可转债持股配债", "对应605***"),
    CodeRule(Exchange.SSE, "718", InstrumentType.OTHER, "科创板可转债申购", "对应118000-118499"),
    CodeRule(Exchange.SSE, "726", InstrumentType.OTHER, "科创板可转债配债", "对应118000-118499"),
    CodeRule(Exchange.SSE, "730", InstrumentType.IPO, "新股申购", "新股申购/网上申购"),
    CodeRule(Exchange.SSE, "758", InstrumentType.OTHER, "可交换债配号", "758000-758099"),
    CodeRule(Exchange.SSE, "759", InstrumentType.OTHER, "可交换债申购", "759000-759099"),
    CodeRule(Exchange.SSE, "786", InstrumentType.OTHER, "科创板配售/存托配售", "786000-786899 科创板股票配售; 786900-786999 科创板存托凭证配售"),
    CodeRule(Exchange.SSE, "799", InstrumentType.OTHER, "特殊业务代码", "指定交易/融资融券/网络投票/资金前端控制/身份认证等(见799xxx 具体编码)"),
    
    CodeRule(Exchange.SSE, "7", InstrumentType.OTHER, "非交易业务", "首位 7: 非交易业务"),

    CodeRule(Exchange.SSE, "880", InstrumentType.SECTOR, "板块指数", "通达信"),
    CodeRule(Exchange.SSE, "881", InstrumentType.SECTOR, "板块指数", "通达信"),
    CodeRule(Exchange.SSE, "888", InstrumentType.BOND, "标准券", "888880 为新标准券，用于债券回购转换成标准券"),
    
    CodeRule(Exchange.SSE, "8", InstrumentType.BOND, "标准券/备用", "首位 8: 标准券、备用"),

    CodeRule(Exchange.SSE, "900", InstrumentType.BSTOCK, "B股", "B 股"),
    CodeRule(Exchange.SSE, "901", InstrumentType.BSTOCK, "B转H", "901000-901099 用于 B 转 H"),
    CodeRule(Exchange.SSE, "938", InstrumentType.OTHER, "网络投票", "对应 B 股(不再增用)"),
    CodeRule(Exchange.SSE, "939", InstrumentType.OTHER, "密码服务", "939988 用于 B 股网络投票密码服务"),
    
    CodeRule(Exchange.SSE, "9", InstrumentType.BSTOCK, "B股", "首位 9: B 股"),
]

# SZSE 深圳证券交易所规则
szse_rules = [
    CodeRule(Exchange.SZSE, "395", InstrumentType.INDEX, "成交量统计指数", ""),
    CodeRule(Exchange.SZSE, "399", InstrumentType.INDEX, "深证指数", ""),
    
    CodeRule(Exchange.SZSE, "000", InstrumentType.STOCK, "主板A股", ""),
    CodeRule(Exchange.SZSE, "001", InstrumentType.STOCK, "主板A股", ""),
    CodeRule(Exchange.SZSE, "002", InstrumentType.STOCK, "主板A股", ""),
    CodeRule(Exchange.SZSE, "003", InstrumentType.STOCK, "主板A股", ""),
    CodeRule(Exchange.SZSE, "030", InstrumentType.WARRANT, "权证", ""),
    CodeRule(Exchange.SZSE, "031", InstrumentType.WARRANT, "权证", ""),
    CodeRule(Exchange.SZSE, "032", InstrumentType.WARRANT, "权证", ""),
    CodeRule(Exchange.SZSE, "036", InstrumentType.WARRANT, "创业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0370", InstrumentType.WARRANT, "主板A股股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0371", InstrumentType.WARRANT, "主板A股股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0372", InstrumentType.WARRANT, "创业板股权激励计划审计的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0373", InstrumentType.WARRANT, "主板A股股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0374", InstrumentType.WARRANT, "主板A股股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0375", InstrumentType.WARRANT, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0376", InstrumentType.WARRANT, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0377", InstrumentType.WARRANT, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0378", InstrumentType.WARRANT, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "0379", InstrumentType.WARRANT, "中小企业板股权激励计划涉及的员工认股权", ""),
    CodeRule(Exchange.SZSE, "038", InstrumentType.WARRANT, "主板A股及中小企业股票认沽权证", ""),
    CodeRule(Exchange.SZSE, "039", InstrumentType.WARRANT, "主板A股及中小企业股票认沽权证", ""),
    CodeRule(Exchange.SZSE, "070", InstrumentType.WARRANT, "主板A股增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "071", InstrumentType.WARRANT, "主板A股增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "072", InstrumentType.WARRANT, "中小企业板增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "073", InstrumentType.WARRANT, "中小企业板增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "074", InstrumentType.WARRANT, "中小企业板增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "080", InstrumentType.WARRANT, "A股配股", ""),
    
    CodeRule(Exchange.SZSE, "0", InstrumentType.STOCK, "股票", ""),
    
    CodeRule(Exchange.SZSE, "10", InstrumentType.BOND, "国债", ""),
    CodeRule(Exchange.SZSE, "11", InstrumentType.BOND, "企业债", ""),
    CodeRule(Exchange.SZSE, "120", InstrumentType.BOND, "企业债券", ""),
    CodeRule(Exchange.SZSE, "123", InstrumentType.BOND, "可转债", ""),
    CodeRule(Exchange.SZSE, "127", InstrumentType.BOND, "可转债", ""),
    CodeRule(Exchange.SZSE, "128", InstrumentType.BOND, "可转债", ""),
    CodeRule(Exchange.SZSE, "13", InstrumentType.BOND, "债券回购", ""),
    CodeRule(Exchange.SZSE, "159", InstrumentType.ETF, "深交所ETF", ""),
    CodeRule(Exchange.SZSE, "15", InstrumentType.FUND, "ETF", ""),
    CodeRule(Exchange.SZSE, "16", InstrumentType.FUND, "LOF", ""),
    CodeRule(Exchange.SZSE, "17", InstrumentType.FUND, "传统投资基金", ""),
    CodeRule(Exchange.SZSE, "184", InstrumentType.FUND, "封闭式基金", ""),
    CodeRule(Exchange.SZSE, "18", InstrumentType.FUND, "封闭式基金", ""),
    
    CodeRule(Exchange.SZSE, "1", InstrumentType.BOND, "债券", ""),
    
    CodeRule(Exchange.SZSE, "200", InstrumentType.BSTOCK, "B股", ""),
    CodeRule(Exchange.SZSE, "238", InstrumentType.OTHER, "B股现金选择权", ""),
    CodeRule(Exchange.SZSE, "28", InstrumentType.OTHER, "B股配股优先权", ""),
    
    CodeRule(Exchange.SZSE, "2", InstrumentType.BSTOCK, "B股", ""),
    
    CodeRule(Exchange.SZSE, "300", InstrumentType.STOCK, "创业板", ""),
    CodeRule(Exchange.SZSE, "301", InstrumentType.STOCK, "创业板注册制", ""),
    CodeRule(Exchange.SZSE, "30", InstrumentType.STOCK, "创业板", ""),
    CodeRule(Exchange.SZSE, "36", InstrumentType.OTHER, "投票", ""),
    CodeRule(Exchange.SZSE, "37", InstrumentType.OTHER, "增发/可转债申购", ""),
    CodeRule(Exchange.SZSE, "38", InstrumentType.OTHER, "配股/可转债优先权", ""),
    
    CodeRule(Exchange.SZSE, "50", InstrumentType.BOND, "资产支持证券ABS", ""),
    CodeRule(Exchange.SZSE, "56", InstrumentType.BOND, "资产支持证券ABS", ""),
    
    CodeRule(Exchange.SZSE, "5", InstrumentType.BOND, "资产支持证券ABS", ""),
    
    CodeRule(Exchange.SZSE, "700", InstrumentType.WARRANT, "B股增发", ""),
    CodeRule(Exchange.SZSE, "730", InstrumentType.WARRANT, "跨市场申购", ""),
]

# BSE 北京证券交易所规则
bse_rules = [
    CodeRule(Exchange.BSE, "899", InstrumentType.INDEX, "指数", "证券指数首三位代码为899"),
    
    CodeRule(Exchange.BSE, "920", InstrumentType.STOCK, "北交所新上市", "2024-04-22 起新上市使用920号段; 已上市公司继续沿用原代码直到统一切换"),
    CodeRule(Exchange.BSE, "92", InstrumentType.STOCK, "上市公司普通股", "首两位92: 上市公司普通股票; 920号段自2024-04-22起用于新上市公司"),
    
    CodeRule(Exchange.BSE, "400", InstrumentType.STOCK, "两网/退市A股", "两网公司及退市公司A股首三位代码为400"),
    CodeRule(Exchange.BSE, "420", InstrumentType.BSTOCK, "退市B股", "退市公司B股首三位代码为420"),
    
    CodeRule(Exchange.BSE, "810", InstrumentType.BOND, "可转换公司债", "向特定对象发行的可转换公司债券首三位代码为810"),
    CodeRule(Exchange.BSE, "81", InstrumentType.BOND, "优先股(极少)", "其他极少数代码"),
    CodeRule(Exchange.BSE, "820", InstrumentType.BOND, "优先股", "优先股票首三位代码为820"),
    CodeRule(Exchange.BSE, "821", InstrumentType.BOND, "优先股", "优先股票首三位代码为820"),
    CodeRule(Exchange.BSE, "82", InstrumentType.BOND, "优先股(极少)", "其他极少数代码"),
    CodeRule(Exchange.BSE, "83", InstrumentType.STOCK, "挂牌公司普通股", "挂牌公司普通股票首两位为83"),
    CodeRule(Exchange.BSE, "840", InstrumentType.OTHER, "要约收购", "要约收购证券代码首三位代码为84"),
    CodeRule(Exchange.BSE, "841", InstrumentType.OTHER, "要约回购", "要约回购证券代码首三位代码为841"),
    CodeRule(Exchange.BSE, "87", InstrumentType.STOCK, "挂牌公司普通股", "挂牌公司普通股票首两位为87"),
    CodeRule(Exchange.BSE, "88", InstrumentType.STOCK, "挂牌公司普通股", "挂牌公司普通股票首两位为88"),
    CodeRule(Exchange.BSE, "850", InstrumentType.OPTION, "股权激励期权", "股权激励期权首三位代码为850"),
]

# HKEX 香港交易所规则
hkex_rules = [
    # 指数
    CodeRule(Exchange.HKEX, "HSI", InstrumentType.INDEX, "恒生指数", "香港交易所"),
    CodeRule(Exchange.HKEX, "HSCEI", InstrumentType.INDEX, "国企指数", "香港交易所"),
    CodeRule(Exchange.HKEX, "HSCCI", InstrumentType.INDEX, "红筹指数", "香港交易所"),
    CodeRule(Exchange.HKEX, "HSTECH", InstrumentType.INDEX, "恒生科技指数", "香港交易所"),
    
    # 00001-09999, 主板及GEM上市证券
    CodeRule(Exchange.HKEX, NumberRange("00001", "02799"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("02800", "02849"), InstrumentType.FUND, "交易所买卖基金", ""),
    CodeRule(Exchange.HKEX, NumberRange("02850", "02899"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("02900", "02999"), InstrumentType.TEMPORARY_STOCK, "主板临时柜台", ""),
    CodeRule(Exchange.HKEX, NumberRange("03000", "03199"), InstrumentType.FUND, "交易所买卖基金", ""),
    CodeRule(Exchange.HKEX, NumberRange("03200", "03399"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("03400", "03499"), InstrumentType.FUND, "交易所买卖基金", ""),
    CodeRule(Exchange.HKEX, NumberRange("03500", "03599"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("03600", "03999"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("04000", "04199"), InstrumentType.BOND, "外汇基金债券", "香港金融管理局"),
    CodeRule(Exchange.HKEX, NumberRange("04200", "04299"), InstrumentType.BOND, "政府债券", "香港特别行政区"),
    CodeRule(Exchange.HKEX, NumberRange("04300", "04329"), InstrumentType.BOND, "债券证券", "仅售予专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("04330", "04399"), InstrumentType.OTHER, "NASDQA-AMEX实验计划", ""),
    CodeRule(Exchange.HKEX, NumberRange("04400", "04599"), InstrumentType.BOND, "债券证券", "仅售予专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("04600", "04699"), InstrumentType.STOCK, "优先股", "仅售予专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("04700", "04799"), InstrumentType.BOND, "债务证券", "售予公众"),
    CodeRule(Exchange.HKEX, NumberRange("04800", "04999"), InstrumentType.WARRANT, "权证", "SPAC"),
    CodeRule(Exchange.HKEX, NumberRange("05000", "06029"), InstrumentType.BOND, "债券证券", "仅售予专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("06030", "06199"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("06200", "06299"), InstrumentType.OTHER, "香港预讬证券", "香港預託證券"),
    CodeRule(Exchange.HKEX, NumberRange("06300", "06399"), InstrumentType.OTHER, "证券/预讬证券", "被美国联邦证券法界定为受限制(RS)证券"),
    CodeRule(Exchange.HKEX, NumberRange("06400", "06599"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("06600", "06749"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("06750", "06799"), InstrumentType.BOND, "财政部债券", "中华人民共和国"),
    CodeRule(Exchange.HKEX, NumberRange("06800", "06999"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("07000", "07199"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("07200", "07399"), InstrumentType.OTHER, "杠杆及反向产品", ""),
    CodeRule(Exchange.HKEX, NumberRange("07400", "07499"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("07500", "07599"), InstrumentType.OTHER, "杠杆及反向产品", ""),
    CodeRule(Exchange.HKEX, NumberRange("07600", "07699"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("07700", "07799"), InstrumentType.OTHER, "杠杆及反向产品", ""),
    CodeRule(Exchange.HKEX, NumberRange("07800", "07999"), InstrumentType.OTHER, "股份", "SPAC"),
    CodeRule(Exchange.HKEX, NumberRange("08000", "08550"), InstrumentType.GEM_MARKET, "GEM证券", ""),
    CodeRule(Exchange.HKEX, NumberRange("08551", "08600"), InstrumentType.TEMPORARY_STOCK, "GEM临时柜台", ""),
    CodeRule(Exchange.HKEX, NumberRange("08601", "08999"), InstrumentType.GEM_MARKET, "GEM证券", ""),
    CodeRule(Exchange.HKEX, NumberRange("09000", "09199"), InstrumentType.FUND, "交易所买卖基金", "美元"),
    CodeRule(Exchange.HKEX, NumberRange("09200", "09399"), InstrumentType.OTHER, "杠杆及反向产品", "美元"),
    CodeRule(Exchange.HKEX, NumberRange("09400", "09499"), InstrumentType.FUND, "交易所买卖基金", "美元"),
    CodeRule(Exchange.HKEX, NumberRange("09500", "09599"), InstrumentType.OTHER, "杠杆及反向产品", "美元"),
    CodeRule(Exchange.HKEX, NumberRange("09600", "09699"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("09700", "09799"), InstrumentType.OTHER, "杠杆及反向产品", "美元"),
    CodeRule(Exchange.HKEX, NumberRange("09800", "09849"), InstrumentType.FUND, "交易所买卖基金", "美元"),
    CodeRule(Exchange.HKEX, NumberRange("09850", "09999"), InstrumentType.STOCK, "主板", ""),
    
    # 10000-29999, 衍生权证
    CodeRule(Exchange.HKEX, NumberRange("10000", "10899"), InstrumentType.WARRANT, "衍生权证", "相关资产在香港以外地区上市的衍生权证、一篮子权证及非标准型权证"),
    CodeRule(Exchange.HKEX, NumberRange("10900", "10999"), InstrumentType.WARRANT, "衍生权证", "相关资产在香港以外地区上市的衍生权证(以美元买卖)"),
    CodeRule(Exchange.HKEX, NumberRange("11000", "11999"), InstrumentType.WARRANT, "衍生权证", "相关资产在香港以外地区上市的衍生权证、一篮子权证及非标准型权证"),
    CodeRule(Exchange.HKEX, NumberRange("12000", "29999"), InstrumentType.WARRANT, "衍生权证", ""),
    
    # 30000-39999, 供沪深股通使用
    CodeRule(Exchange.HKEX, NumberRange("30000", "39999"), InstrumentType.OTHER, "沪深股通", ""),
    
    # 40000-40999, 仅售于专业投资者的债务证券
    CodeRule(Exchange.HKEX, NumberRange("40000", "40999"), InstrumentType.BOND, "债务证券", "仅售于专业投资者"),
    # 41000-46999, 供日后使用
    CodeRule(Exchange.HKEX, NumberRange("41000", "46999"), InstrumentType.OTHER, "供日后使用", "保留"),
    # 47000-48999, 供日后使用
    CodeRule(Exchange.HKEX, NumberRange("47000", "48999"), InstrumentType.OTHER, "界内证", "保留"),
    # 49000-49499, 供日后使用
    CodeRule(Exchange.HKEX, NumberRange("49000", "49499"), InstrumentType.OTHER, "供日后使用", "保留"),
    # 49500-69999, 牛熊证, callable bull and bear contract
    CodeRule(Exchange.HKEX, NumberRange("49500", "49999"), InstrumentType.OPTION, "牛熊证", "相关资产在香港以外地区上市"),
    CodeRule(Exchange.HKEX, NumberRange("50000", "69999"), InstrumentType.OPTION, "牛熊证", ""),  
    # 70000-79999, 供沪深股通使用
    CodeRule(Exchange.HKEX, NumberRange("70000", "79999"), InstrumentType.OTHER, "沪深股通", ""),
    # 80000-89999, 以人民币买卖的产品
    CodeRule(Exchange.HKEX, NumberRange("80000", "82799"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("82800", "82849"), InstrumentType.FUND, "交易所买卖基金", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("82850", "82899"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("82900", "82999"), InstrumentType.TEMPORARY_STOCK, "主板临时柜台", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("83000", "83199"), InstrumentType.FUND, "交易所买卖基金", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("83200", "83399"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("83400", "83499"), InstrumentType.FUND, "交易所买卖基金", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("83500", "83599"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("83600", "83999"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("84000", "84299"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("84300", "84329"), InstrumentType.BOND, "债券证券", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("84330", "84399"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("84400", "84599"), InstrumentType.BOND, "债务证券", "仅售于专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("84600", "84699"), InstrumentType.STOCK, "优先股", "仅售于专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("84700", "84999"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("85000", "85743"), InstrumentType.BOND, "债务证券", "仅售于专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("85744", "85900"), InstrumentType.BOND, "债务证券", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("85901", "86029"), InstrumentType.BOND, "债务证券", "仅售于专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("86030", "86199"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("86200", "86299"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("86600", "86799"), InstrumentType.OTHER, "中华人民共和国财政部债券/主板证券", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("86800", "86999"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87000", "87099"), InstrumentType.FUND, "房地产投资信托基金及交易所买卖基金以外的单位信托/互惠基金", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87100", "87199"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("87200", "87399"), InstrumentType.OTHER, "杠杆及反向产品", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87400", "87499"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87500", "87599"), InstrumentType.OTHER, "杠杆及反向产品", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87600", "87699"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87700", "87799"), InstrumentType.OTHER, "杠杆及反向产品", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87800", "88999"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("89000", "89099"), InstrumentType.BOND, "中华人民共和国财政部债券", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("89100", "89199"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("89200", "89599"), InstrumentType.WARRANT, "衍生权证", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("89600", "89699"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("89700", "89849"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("89850", "89999"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    
    # 90000-99999, 供沪深股通使用
    CodeRule(Exchange.HKEX, "9", InstrumentType.OTHER, "沪深股通", ""),
]


def match_rule(code: str, rules: List[CodeRule]) -> CodeRule:
    """
    根据代码前缀匹配最优规则
    
    Args:
        code (str): 需要匹配的代码字符串
        rules (List[CodeRule]): 可匹配的规则列表
    
    Returns:
        CodeRule: 匹配到的最优规则，若无匹配则返回默认的未知规则
    """
    code = code.upper().strip()
    best_match: CodeRule | None = None
    best_len = 0

    for entry in rules:
        prefix = entry.prefix
        print(prefix)
        # 跳过空前缀(可选，根据业务)
        if not prefix:
            continue
        if isinstance(prefix, str) and code.startswith(prefix) and len(prefix) > best_len:
            best_len = len(prefix)
            best_match = entry
        elif isinstance(prefix, NumberRange):
            prefix_len = prefix.max_value_length()
            if code in prefix and prefix_len > best_len:
                best_len = prefix_len
                best_match = entry
                break

    if best_match is not None:
        return best_match
    else:
        return CodeRule(
            exchange=Exchange.UNKNOWN,
            prefix="",
            type=InstrumentType.Unknown,
            name="",
            desc="未匹配到规则"
        )

def detect_instrument_type_by_rule(exchange: Exchange, code: str) -> InstrumentType:
    """
    根据交易所和代码，使用对应规则检测证券类型
    
    Args:
        exchange (Exchange): 交易所枚举
        code (str): 证券代码字符串
    
    Returns:
        InstrumentType: 检测到的证券类型，若无匹配则返回 Unknown
    """
    rules = None
    match exchange:
        case Exchange.SSE:
            rules = sse_rules
        case Exchange.SZSE:
            rules = szse_rules
        case Exchange.BSE:
            rules = bse_rules
        case Exchange.HKEX:
            rules = hkex_rules
        case _:
            return InstrumentType.Unknown

    cr = match_rule(code, rules)
    return cr.type

# 所有交易所标识
ALL_EXCHANGE_IDENTIFIERS = {
    Exchange.SSE.identifier,
    Exchange.SZSE.identifier,
    Exchange.BSE.identifier,
    Exchange.HKEX.identifier,
    Exchange.HKFE.identifier,
    Exchange.USA.identifier,
}

def detect_symbol(input_str: str) -> Instrument:
    """
    检测并解析证券代码的市场类型及证券类型
    
    Args:
        input_str (str): 输入的证券代码字符串，支持多种格式: 
            - 前缀形式: sh600000(上海交易所)
            - 后缀形式: 600000.sh 或 APPL.us
            - 纯数字形式: 600000(自动推断交易所)
            - 4字母全大写: AAPL(自动识别为美股)
    
    Returns:
        Instrument: 包含以下属性的对象: 
            - exchange_id: 交易所标识
            - symbol: 纯证券代码
            - instrument_type: 证券类型
    
    Note:
        支持识别以下交易所: 
            - 上海证券交易所(SSE)
            - 深圳证券交易所(SZSE) 
            - 北京证券交易所(BSE)
            - 香港交易所(HKEX)
            - 香港期货交易所(HKFE)
            - 美国市场(USA)
        自动根据代码规则推断证券类型(股票、债券等)
    """
    #print(f"detect_symbol: {input_str}")
    s = (input_str or "").strip()
    if not s:
        return Instrument(Exchange.SSE, InstrumentType.Unknown, "", "", 0, 0)
    s = s.lower()
    pure_code = s

    ticker = ""
    exchange = Exchange.UNKNOWN
    typ = InstrumentType.Unknown

    # 1. 判断前缀: sh600000
    if len(pure_code) >= 2 and pure_code[:2] in ALL_EXCHANGE_IDENTIFIERS:
        ticker = pure_code[2:]
        prefix_code = pure_code[:2]
        exchange = Exchange.parse(prefix_code)
        # 走指定市场规则
    # 2. 判断后缀: 600000.sh or APPL.us
    elif len(pure_code) >= 3 and pure_code[-3] == '.' and pure_code[-2:] in ALL_EXCHANGE_IDENTIFIERS:
        ticker = pure_code[:-3]
        suffix_code = pure_code[-2:]
        exchange = Exchange.parse(suffix_code)
        # 走指定市场规则
        print(f"ticker: {ticker}, exchange: {exchange}")
    else:
        # 纯数字或者字母
        code_len = len(pure_code)
        match code_len:
            case 4:
                if pure_code.isalpha():
                    exchange = Exchange.USA
                    typ = InstrumentType.STOCK
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
            case 5:
                if pure_code.isdigit():
                    exchange = Exchange.HKEX
                    typ = InstrumentType.STOCK
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
            case 6:
                # 1. 全局规则优先匹配
                cr = match_rule(pure_code, global_rules)
                if cr.exchange != Exchange.UNKNOWN:
                    exchange = cr.exchange
                    typ = cr.type
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
                # 2. 按市场匹配规则
                # 2.1 0、159和3开头，优先匹配深交所
                if pure_code.startswith(('0', '159', '3')):
                    cr = match_rule(pure_code, szse_rules)
                    if cr.exchange != Exchange.UNKNOWN:
                        exchange = cr.exchange
                        typ = cr.type
                        return Instrument(exchange, typ, pure_code, "", 0, 0)
                # 2.2 6和5开头，优先匹配上交所
                if pure_code.startswith(('6', '5')):
                    cr = match_rule(pure_code, sse_rules)
                    if cr.exchange != Exchange.UNKNOWN:
                        exchange = cr.exchange
                        typ = cr.type
                        return Instrument(exchange, typ, pure_code, "", 0, 0)
                # 2.3 匹配上交所
                cr = match_rule(pure_code, sse_rules)
                if cr.exchange != Exchange.UNKNOWN:
                    exchange = cr.exchange
                    typ = cr.type
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
                # 2.4 匹配深交所
                cr = match_rule(pure_code, szse_rules)
                if cr.exchange != Exchange.UNKNOWN:
                    exchange = cr.exchange
                    typ = cr.type
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
                # 2.5 匹配北交所
                cr = match_rule(pure_code, bse_rules)
                if cr.exchange != Exchange.UNKNOWN:
                    exchange = cr.exchange
                    typ = cr.type
                    return Instrument(exchange, typ, pure_code, "", 0, 0)
            case _:
                exchange = Exchange.UNKNOWN
                typ = InstrumentType.Unknown

    # 3. 如果exchange是UNKNOWN，则返回未知规则
    if exchange == Exchange.UNKNOWN:
        return Instrument(Exchange.UNKNOWN, InstrumentType.Unknown, "", "", 0, 0)

    if typ == InstrumentType.Unknown:
        rules = None
        if exchange == Exchange.SSE:
            rules = sse_rules
        elif exchange == Exchange.SZSE:
            rules = szse_rules
        elif exchange == Exchange.BSE:
            rules = bse_rules
        elif exchange == Exchange.HKEX:
            rules = hkex_rules
        elif exchange == Exchange.USA:
            return Instrument(exchange, InstrumentType.STOCK, pure_code, "", 0, 0)
        else:
            return Instrument(Exchange.UNKNOWN, InstrumentType.Unknown, "", "", 0, 0)

        cr = match_rule(ticker, rules)
        print(f"cr: {cr}")
        if cr.type != InstrumentType.Unknown:
            return Instrument(cr.exchange, cr.type, ticker, "", 0, 0)
        else:
            return Instrument(Exchange.UNKNOWN, InstrumentType.Unknown, "", "", 0, 0)
    else:
        return Instrument(exchange, typ, ticker, "", 0, 0)

def correct_security_code(code: str) -> str:
    """
    纠正证券代码格式，补全前缀或后缀
    
    Args:
        code (str): 输入的证券代码字符串，支持多种格式: 
            - 前缀形式: sh600000(上海交易所)
            - 后缀形式: 600000.sh 或 APPL.us
            - 纯数字形式: 600000(自动推断交易所)
            - 4字母全大写: AAPL(自动识别为美股)  
            - 6位数字: 600000(自动推断交易所)
            - 4字母全大写: AAPL(自动识别为美股)
    
    Returns:
        str: 纠正后的证券代码字符串
    """
    inst = detect_symbol(code)
    if inst.can_construct_symbol():
        return inst.symbol()
    else:
        raise ValueError(f"无法纠正证券代码: {code}")

PRE_MARKET_HOUR = 9
PRE_MARKET_MINUTE = 0
PRE_MARKET_SECOND = 0
cn_cron_expr_daily_init = f"0 {PRE_MARKET_HOUR} {PRE_MARKET_MINUTE} * * *"

__all__ = [
    "detect_instrument_type_by_rule",
    "detect_symbol",
    "cn_cron_expr_daily_init",
]


if __name__ == "__main__":
    symbol = detect_symbol("600000.sh")
    print(symbol.to_string())
    
    symbol = detect_symbol("hsi.hk")
    print(symbol.to_string())
    
    symbol = detect_symbol("85000.hk")
    print(symbol.to_string())