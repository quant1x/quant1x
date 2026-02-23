# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import os
import csv
from typing import Optional
from quant1x.std import filesystem as fs
from quant1x.config import config
from quant1x.data import status
from quant1x.data.schema import XdxrInfo
from quant1x.data.meta.timestamp import Timestamp
from quant1x.data.market import Instrument, Exchange
from .level1 import XdxrInfoRequest, XdxrInfoResponse
from .protocol import process
from .client import get_std_conn
from quant1x.log import logger

def _get_xdxr_filename(inst: Instrument) -> str:
    """
    根据股票代码生成对应的除权除息数据文件路径
    
    Args:
        inst (Instrument): 股票代码对象，包含股票符号信息
    
    Returns:
        str: 除权除息数据文件的完整路径，格式为 {数据目录}/xdxr/{股票代码}.csv
    """
    dir = config.data_path
    sub = f'xdxr/{inst.cache_dir()}'
    symbol = inst.symbol()
    return f'{dir}/{sub}/{symbol}.csv'

def load_xdxr(inst: Instrument) -> list[XdxrInfo]:
    result = []
    try:
        filename = _get_xdxr_filename(inst)
        print(f"Loading Xdxr data from {filename}")
        if os.path.exists(filename):
            with open(filename, 'r', encoding='utf-8') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    info = XdxrInfo()
                    info.Date = row['date']
                    info.Category = int(row['category'])
                    info.Name = row['name']
                    info.FenHong = float(row['fen_hong'])
                    info.PeiGuJia = float(row['pei_gu_jia'])
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
            writer.writerow(["date", "category", "name", "fen_hong", "pei_gu_jia", "song_zhuan_gu",
                             "pei_gu", "suo_gu", "qian_liu_tong", "hou_liu_tong", "qian_zong_gu_ben", 
                             "hou_zong_gu_ben", "fen_shu", "xing_quan_jia"])
            for v in values:
                writer.writerow([v.Date, v.Category, v.Name, v.FenHong, v.PeiGuJia, v.SongZhuanGu,
                                 v.PeiGu, v.SuoGu, v.QianLiuTong, v.HouLiuTong, v.QianZongGuBen,
                                 v.HouZongGuBen, v.FenShu, v.XingQuanJia])
    except Exception:
        logger.exception(f"[dataset::xdxr] save failed")

def update_xdxr(inst: Instrument):
    try:
        conn = get_std_conn()
        req = XdxrInfoRequest(inst.exchange, inst.ticker)
        resp = XdxrInfoResponse()
        process(conn, req, resp)
        if resp.count > 0:
            save_xdxr(inst, resp.list)
    except Exception:
        logger.exception(f"[dataset::xdxr] update failed")

def get_xdxr_list(inst: Instrument) -> list[XdxrInfo]:
    filename = _get_xdxr_filename(inst)
    create_or_update = status.should_initialize_file(fname=filename, exchange=inst.exchange)
    if create_or_update:
        update_xdxr(inst)
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

    code = "600000"
    inst = get_instrument_info(code)
    print(inst)
    update_xdxr(inst)
    print(_get_xdxr_filename(inst))