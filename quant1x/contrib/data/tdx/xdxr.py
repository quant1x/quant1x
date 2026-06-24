# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
import csv
from typing import Optional, List
from quant1x.std import filesystem as fs
from quant1x.std.numeric import float_round
from quant1x.config import config
from quant1x.data import status
from quant1x.data.schema import XdxrInfo, XdxrCategory
from quant1x.data.meta.timestamp import Timestamp
from quant1x.data.market import Instrument, Exchange
from .level1 import XdxrInfoContext
from . import protocol
from .client import get_std_conn, get_ext_conn
from quant1x.log import logger

def _get_xdxr_filename(inst: Instrument) -> str:
    """
    根据股票代码生成对应的除权除息数据文件路径
    
    Args:
        inst (Instrument): 股票代码对象, 包含股票符号信息
    
    Returns:
        str: 除权除息数据文件的完整路径, 格式为 {数据目录}/xdxr/{股票代码}.csv
    """
    dir = config.data_path
    sub = f'xdxr/{inst.cache_dir()}'
    symbol = inst.symbol()
    return f'{dir}/{sub}/{symbol}.csv'

def load_xdxr(inst: Instrument) -> list[XdxrInfo]:
    result = []
    try:
        filename = _get_xdxr_filename(inst)
        logger.debug(f"Loading Xdxr data from {filename}")
        if os.path.exists(filename):
            with open(filename, 'r', encoding='utf-8') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    info = XdxrInfo()
                    info.Date = row['date']
                    info.Category = int(row['category'])
                    info.Name = row['name']
                    info.FenHong = float(row['fen_hong'])
                    info.dividend_currency = row['dividend_currency']
                    info.PeiGuJia = float(row['pei_gu_jia'])
                    info.rights_currency = row['rights_currency']
                    info.SongZhuanGu = float(row['song_zhuan_gu'])
                    info.PeiGu = float(row['pei_gu'])
                    info.SuoGu = float(row['suo_gu'])
                    info.QianLiuTong = float(row['qian_liu_tong'])
                    info.HouLiuTong = float(row['hou_liu_tong'])
                    info.QianZongGuBen = float(row['qian_zong_gu_ben'])
                    info.HouZongGuBen = float(row['hou_zong_gu_ben'])
                    info.FenShu = float(row['fen_shu'])
                    info.XingQuanJia = float(row['xing_quan_jia'])
                    result.append(info)
    except Exception:
        logger.exception(f"[dataset::xdxr] load failed")
    return result

def save_xdxr(inst: Instrument, values: list[XdxrInfo]):
    filename = _get_xdxr_filename(inst)
    try:
        fs.mkdirs(os.path.dirname(filename))
        with open(filename, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(["date", "category", "name", "fen_hong", "dividend_currency", "pei_gu_jia","rights_currency", "song_zhuan_gu",
                             "pei_gu", "suo_gu", "qian_liu_tong", "hou_liu_tong", "qian_zong_gu_ben", 
                             "hou_zong_gu_ben", "fen_shu", "xing_quan_jia"])
            for v in values:
                writer.writerow([v.Date, v.Category, v.Name,
                                 v.FenHong, v.dividend_currency,
                                 v.PeiGuJia, v.rights_currency,
                                 v.SongZhuanGu,
                                 v.PeiGu, v.SuoGu, v.QianLiuTong, v.HouLiuTong, v.QianZongGuBen,
                                 v.HouZongGuBen, v.FenShu, v.XingQuanJia])
    except Exception:
        logger.exception(f"[dataset::xdxr] save failed")

def update_xdxr_from_std(inst: Instrument):
    try:
        conn = get_std_conn()
        msg = XdxrInfoContext(inst=inst)
        protocol.transact_message_sync(conn, msg)
        if msg.count > 0:
            save_xdxr(inst, msg.list)
    except Exception:
        logger.exception(f"[dataset::xdxr] update failed")

from .level1.ext import CompanyInfoCategories, CompanyInfoContent
from .level1.ext.xdxr_hkex import parse_text_to_list


def update_xdxr_from_ext_0x24b9(inst: Instrument):
    """
    从扩展行情更新除权除息数据
    """
    try:
        conn = get_ext_conn()
        from .level1.ext import CompanyInfoCategories, CompanyInfoContent
        categories = CompanyInfoCategories(market=inst.ext_market, ticker=inst.market_ticker())
        protocol.transact_message_sync(conn, categories)
        if categories.reply:
            # 捡出 分红送股
            for category in categories.reply:
                if category.title == '分红送股':
                    xdxr_info = CompanyInfoContent(market=categories.market, ticker=categories.ticker, filename=category.filename, offset=category.offset, size=category.size)
                    protocol.transact_message_sync(conn, xdxr_info)
                    if xdxr_info.reply:
                        xdxr_records = parse_text_to_list(xdxr_info.reply, inst.exchange.region.currency)
                        converted: List[XdxrInfo] = []
                        for rec in xdxr_records:
                            logger.debug(f"{rec}")
                            x = XdxrInfo()
                            # common fields
                            x.Date = rec.get('ex_date', '')
                            category_str = rec.get('category', '')
                            x.Name = category_str
                            # map text category to numeric code when possible
                            if category_str in ("分红派息", "供股", "拆分合并"):
                                x.Category = XdxrCategory.EX_DIVIDEND.value
                            else:
                                # leave as default (0) if unknown, but still keep the name
                                x.Category = 0
                            if x.Category > 0:
                                # 分红
                                try:
                                    dividend_amount = float(rec.get('dividend_amount', 0) or 0)
                                except Exception:
                                    dividend_amount = 0
                                x.dividend_currency = rec.get('dividend_currency', '')
                                try:
                                    ratio_shares = float(rec.get('ratio_shares', 0) or 0)
                                except Exception:
                                    ratio_shares = 0
                                # 转换为10股
                                x.FenHong = (dividend_amount / ratio_shares) * 10 if ratio_shares != 0 else 0
                                
                                # 送股: 供股/配股
                                restructuring_type = rec.get('restructuring_type', '')
                                if restructuring_type == '拆股':
                                    split_ratio = float(rec.get('split_ratio', 0) or 0)
                                    x.SongZhuanGu = (split_ratio-1) * 10
                                if category_str == '供股':
                                    entitlement_ratio = float(rec.get('entitlement_ratio', 0) or 0)
                                    x.PeiGu = entitlement_ratio * 10
                                    subscription_price = float(rec.get('subscription_price', 0) or 0)
                                    x.PeiGuJia = subscription_price
                                    price_currency = rec.get('price_currency', '')
                                    x.rights_currency = price_currency
                            logger.debug(f"{x}")
                            converted.append(x)
                        save_xdxr(inst, converted)
                        break
    except Exception:
        logger.exception(f"[dataset::xdxr] update failed")

def update_xdxr_from_ext_7615(inst: Instrument):
    from .level1.ext.hk_f10 import get_ext_xdxr_info
    try:
        rows = get_ext_xdxr_info(inst=inst)
        save_xdxr(inst, rows)
    except Exception as e:
        logger.exception(f"[dataset::xdxr] update failed")

def update_xdxr(inst: Instrument):
    if inst.exchange.is_std_quote():
        update_xdxr_from_std(inst)
    elif inst.exchange.is_ext_quote():
        update_xdxr_from_ext_7615(inst)
    

def get_xdxr_list(inst: Instrument) -> list[XdxrInfo]:
    filename = _get_xdxr_filename(inst)
    create_or_update = status.should_initialize_file(fname=filename, exchange=inst.exchange)
    if create_or_update:
        logger.debug(f"[dataset::xdxr] update xdxr data for {inst}")
        update_xdxr(inst)
    else:
        logger.debug(f"[dataset::xdxr] load xdxr data for {inst}")
    return load_xdxr(inst)

from quant1x.data import adapter
from quant1x.data.adapter import DataAdapter, DEFAULT_DATA_PROVIDER
from quant1x.data.base import BASEDATA_XDXR

class DataXdxr(DataAdapter):
    def kind(self):
        return BASEDATA_XDXR
        
    def owner(self):
        return DEFAULT_DATA_PROVIDER
        
    def key(self):
        return "xdxr"
        
    def name(self):
        return "除权除息"
        
    def usage(self):
        return ""
        
    def print(self, inst: Instrument, date: Optional[Timestamp] = None):
        pass
        
    def update(self, inst: Instrument, date: Optional[Timestamp] = None):
        update_xdxr(inst)

# 注册插件
_data_xdxr_plugin = adapter.register(DataXdxr)

if __name__ == "__main__":
    from .instruments import get_instrument_info
    import pandas as pd
    code = "hk00700"
    code = "00077.hk"
    inst = get_instrument_info(code)
    print(inst)
    update_xdxr(inst)
    print(_get_xdxr_filename(inst))
    rows = get_xdxr_list(inst)
    df = pd.DataFrame(rows)
    print(df)