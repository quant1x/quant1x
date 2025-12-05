# -*- coding: UTF-8 -*-
import logging
import os
import csv
from typing import List

from quant1x.level1 import client, protocol, xdxr_info
from quant1x.exchange import code as exchange_code
from quant1x.exchange import Timestamp
from quant1x.config import config

log = logging.getLogger(__name__)

def load_xdxr(code: str) -> List[xdxr_info.XdxrInfo]:
    result = []
    try:
        filename = config.get_xdxr_filename(code)
        if os.path.exists(filename):
            with open(filename, 'r', encoding='utf-8') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    info = xdxr_info.XdxrInfo()
                    info.Date = row['Date']
                    info.Category = int(row['Category'])
                    info.Name = row['Name']
                    info.FenHong = float(row['FenHong'])
                    info.PeiGuJia = float(row['PeiGuJia'])
                    info.SongZhuanGu = float(row['SongZhuanGu'])
                    info.PeiGu = float(row['PeiGu'])
                    info.SuoGu = float(row['SuoGu'])
                    info.QianLiuTong = float(row['QianLiuTong'])
                    info.HouLiuTong = float(row['HouLiuTong'])
                    info.QianZongGuBen = float(row['QianZongGuBen'])
                    info.HouZongGuBen = float(row['HouZongGuBen'])
                    info.FenShu = float(row['FenShu'])
                    info.XingQuanJia = float(row['XingQuanJia'])
                    result.append(info)
    except Exception as e:
        log.error(f"[dataset::xdxr] Load failed: {e}")
    return result

def save_xdxr(code: str, date: Timestamp, values: List[xdxr_info.XdxrInfo]):
    security_code = exchange_code.correct_security_code(code)
    # date is ignored as in C++ implementation
    
    filename = config.get_xdxr_filename(security_code)
    try:
        os.makedirs(os.path.dirname(filename), exist_ok=True)
        with open(filename, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(["Date", "Category", "Name", "FenHong", "PeiGuJia", "SongZhuanGu",
                             "PeiGu", "SuoGu", "QianLiuTong", "HouLiuTong", "QianZongGuBen", 
                             "HouZongGuBen", "FenShu", "XingQuanJia"])
            for v in values:
                writer.writerow([v.Date, v.Category, v.Name, v.FenHong, v.PeiGuJia, v.SongZhuanGu,
                                 v.PeiGu, v.SuoGu, v.QianLiuTong, v.HouLiuTong, v.QianZongGuBen,
                                 v.HouZongGuBen, v.FenShu, v.XingQuanJia])
    except Exception as e:
        log.error(f"[dataset::xdxr] Save failed: {e}")

class DataXdxr:
    def kind(self):
        return "BaseXdxr"
        
    def owner(self):
        return "default"
        
    def key(self):
        return "xdxr"
        
    def name(self):
        return "除权除息"
        
    def usage(self):
        return ""
        
    def print(self, code: str, dates: List[Timestamp]):
        pass
        
    def update(self, code: str, date: Timestamp):
        try:
            req = xdxr_info.XdxrInfoRequest(code)
            
            conn = client.client()
            sock = conn.socket
            resp = xdxr_info.XdxrInfoResponse()
            protocol.process(sock, req, resp)
            
            if resp.count > 0:
                save_xdxr(code, date, resp.list)
                
        except Exception as e:
            log.error(f"[dataset::xdxr] 获取除权除息异常: {e}")

