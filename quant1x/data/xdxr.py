# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import logging
import os
import csv
from typing import List, Optional

from quant1x.level1 import protocol, xdxr_info
from quant1x.level1.client import get_std_conn
from quant1x.data.market import correct_security_code
from quant1x.data.meta.timestamp import Timestamp
from quant1x.config import config
from . import adapter
from .adapter import DataAdapter, DEFAULT_DATA_PROVIDER
from .base import BASEDATA_XDXR

log = logging.getLogger(__name__)

__all__ = ['load_xdxr', 'save_xdxr', 'DataXdxr']

def load_xdxr(code: str) -> List[xdxr_info.XdxrInfo]:
    result = []
    try:
        filename = config.get_xdxr_filename(code)
        print(f"Loading Xdxr data from {filename}")
        if os.path.exists(filename):
            with open(filename, 'r', encoding='utf-8') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    info = xdxr_info.XdxrInfo()
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
    except Exception as e:
        log.error(f"[dataset::xdxr] Load failed: {e}")
    return result

def save_xdxr(code: str, date: Timestamp, values: List[xdxr_info.XdxrInfo]):
    security_code = correct_security_code(code)
    # date is ignored as in C++ implementation
    
    filename = config.get_xdxr_filename(security_code)
    try:
        os.makedirs(os.path.dirname(filename), exist_ok=True)
        with open(filename, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(["date", "category", "name", "fen_hong", "pei_gu_jia", "song_zhuan_gu",
                             "pei_gu", "suo_gu", "qian_liu_tong", "hou_liu_tong", "qian_zong_gu_ben", 
                             "hou_zong_gu_ben", "fen_shu", "xing_quan_jia"])
            for v in values:
                writer.writerow([v.Date, v.Category, v.Name, v.FenHong, v.PeiGuJia, v.SongZhuanGu,
                                 v.PeiGu, v.SuoGu, v.QianLiuTong, v.HouLiuTong, v.QianZongGuBen,
                                 v.HouZongGuBen, v.FenShu, v.XingQuanJia])
    except Exception as e:
        log.error(f"[dataset::xdxr] Save failed: {e}")

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
        
    def print(self, code: str, dates: Optional[List[Timestamp]] = None):
        pass
        
    def update(self, code: str, date: Optional[Timestamp] = None):
        try:
            req = xdxr_info.XdxrInfoRequest(code)
            
            conn = get_std_conn()
            resp = xdxr_info.XdxrInfoResponse()
            protocol.process(conn, req, resp)
            
            if resp.count > 0:
                save_xdxr(code, date, resp.list)
                
        except Exception as e:
            log.error(f"[dataset::xdxr] 获取除权除息异常: {e}")


# 注册插件
_data_xdxr_plugin = adapter.register(DataXdxr)


if __name__ == "__main__":
    DataXdxr().update("sh600000", None)

