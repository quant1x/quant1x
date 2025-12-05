# -*- coding: UTF-8 -*-
from __future__ import annotations

import os
import socket
import threading
import time
import yaml
import logging
from typing import List, Tuple, Dict, Optional, Any

from quant1x import config as qconfig

log = logging.getLogger(__name__)


# Complete server candidate lists copied from the C++ source
StandardServerList: List[Dict[str, Any]] = [
    {"source": "通达信", "name": "深圳双线主站1", "host": "110.41.147.114", "port": 7709},
    {"source": "通达信", "name": "深圳双线主站2", "host": "110.41.2.72", "port": 7709},
    {"source": "通达信", "name": "深圳双线主站3", "host": "110.41.4.4", "port": 7709},
    {"source": "通达信", "name": "深圳双线主站4", "host": "47.113.94.204", "port": 7709},
    {"source": "通达信", "name": "深圳双线主站5", "host": "8.129.174.169", "port": 7709},
    {"source": "通达信", "name": "深圳双线主站6", "host": "110.41.154.219", "port": 7709},
    {"source": "通达信", "name": "上海双线主站1", "host": "124.70.176.52", "port": 7709},
    {"source": "通达信", "name": "上海双线主站2", "host": "47.100.236.28", "port": 7709},
    {"source": "通达信", "name": "上海双线主站3", "host": "123.60.186.45", "port": 7709},
    {"source": "通达信", "name": "上海双线主站4", "host": "123.60.164.122", "port": 7709},
    {"source": "通达信", "name": "上海双线主站5", "host": "47.116.105.28", "port": 7709},
    {"source": "通达信", "name": "上海双线主站6", "host": "124.70.199.56", "port": 7709},
    {"source": "通达信", "name": "北京双线主站1", "host": "121.36.54.217", "port": 7709},
    {"source": "通达信", "name": "北京双线主站2", "host": "121.36.81.195", "port": 7709},
    {"source": "通达信", "name": "北京双线主站3", "host": "123.249.15.60", "port": 7709},
    {"source": "通达信", "name": "广州双线主站1", "host": "124.71.85.110", "port": 7709},
    {"source": "通达信", "name": "广州双线主站2", "host": "139.9.51.18", "port": 7709},
    {"source": "通达信", "name": "广州双线主站3", "host": "139.159.239.163", "port": 7709},
    {"source": "通达信", "name": "上海双线主站7", "host": "106.14.201.131", "port": 7709},
    {"source": "通达信", "name": "上海双线主站8", "host": "106.14.190.242", "port": 7709},
    {"source": "通达信", "name": "上海双线主站9", "host": "121.36.225.169", "port": 7709},
    {"source": "通达信", "name": "上海双线主站10", "host": "123.60.70.228", "port": 7709},
    {"source": "通达信", "name": "上海双线主站11", "host": "123.60.73.44", "port": 7709},
    {"source": "通达信", "name": "上海双线主站12", "host": "124.70.133.119", "port": 7709},
    {"source": "通达信", "name": "上海双线主站13", "host": "124.71.187.72", "port": 7709},
    {"source": "通达信", "name": "上海双线主站14", "host": "124.71.187.122", "port": 7709},
    {"source": "通达信", "name": "武汉电信主站1", "host": "119.97.185.59", "port": 7709},
    {"source": "通达信", "name": "深圳双线主站7", "host": "47.107.64.168", "port": 7709},
    {"source": "通达信", "name": "北京双线主站4", "host": "124.70.75.113", "port": 7709},
    {"source": "通达信", "name": "广州双线主站4", "host": "124.71.9.153", "port": 7709},
    {"source": "通达信", "name": "上海双线主站15", "host": "123.60.84.66", "port": 7709},
    {"source": "通达信", "name": "深圳双线主站8", "host": "47.107.228.47", "port": 7719},
    {"source": "通达信", "name": "北京双线主站5", "host": "120.46.186.223", "port": 7709},
    {"source": "通达信", "name": "北京双线主站6", "host": "124.70.22.210", "port": 7709},
    {"source": "通达信", "name": "北京双线主站7", "host": "139.9.133.247", "port": 7709},
    {"source": "通达信", "name": "广州双线主站5", "host": "116.205.163.254", "port": 7709},
    {"source": "通达信", "name": "广州双线主站6", "host": "116.205.171.132", "port": 7709},
    {"source": "通达信", "name": "广州双线主站7", "host": "116.205.183.150", "port": 7709},
    {"source": "中信证券", "name": "上海电信主站Z1", "host": "180.153.18.170", "port": 7709},
    {"source": "中信证券", "name": "上海电信主站Z2", "host": "180.153.18.171", "port": 7709},
    {"source": "中信证券", "name": "北京联通主站Z1", "host": "202.108.253.130", "port": 7709},
    {"source": "中信证券", "name": "北京联通主站Z2", "host": "202.108.253.131", "port": 7709},
    {"source": "中信证券", "name": "杭州电信主站J1", "host": "60.191.117.167", "port": 7709},
    {"source": "中信证券", "name": "杭州电信主站J2", "host": "115.238.56.198", "port": 7709},
    {"source": "中信证券", "name": "杭州电信主站J3", "host": "218.75.126.9", "port": 7709},
    {"source": "中信证券", "name": "杭州电信主站J4", "host": "115.238.90.165", "port": 7709},
    {"source": "中信证券", "name": "杭州联通主站J1", "host": "124.160.88.183", "port": 7709},
    {"source": "中信证券", "name": "杭州联通主站J2", "host": "60.12.136.250", "port": 7709},
    {"source": "中信证券", "name": "杭州华数主站J1", "host": "218.108.98.244", "port": 7709},
    {"source": "中信证券", "name": "杭州华数主站J2", "host": "218.108.47.69", "port": 7709},
    {"source": "中信证券", "name": "济南联通主站W1", "host": "27.221.115.131", "port": 7709},
    {"source": "中信证券", "name": "青岛电信主站W1", "host": "58.56.180.60", "port": 7709},
    {"source": "中信证券", "name": "深圳电信主站Z1", "host": "14.17.75.71", "port": 7709},
    {"source": "中信证券", "name": "云行情上海电信Z1", "host": "114.80.63.12", "port": 7709},
    {"source": "中信证券", "name": "云行情上海电信Z2", "host": "114.80.63.35", "port": 7709},
    {"source": "中信证券", "name": "上海电信主站Z3", "host": "180.153.39.51", "port": 7709},
    {"source": "中信证券", "name": "云行情北京联通Z1", "host": "123.125.108.23", "port": 7709},
    {"source": "中信证券", "name": "云行情北京联通Z2", "host": "123.125.108.24", "port": 7709},
    {"source": "中信证券", "name": "云行情广州电信Z1", "host": "121.201.83.106", "port": 7709},
    {"source": "中信证券", "name": "云行情成都电信Z1", "host": "218.6.170.55", "port": 7709},
    {"source": "华泰证券", "name": "华泰证券(南京电信一)", "host": "180.101.48.170", "port": 7709},
    {"source": "华泰证券", "name": "华泰证券(南京电信二)", "host": "180.101.48.171", "port": 7709},
    {"source": "华泰证券", "name": "华泰证券(南京移动一)", "host": "120.195.71.155", "port": 7709},
    {"source": "华泰证券", "name": "华泰证券(南京移动二)", "host": "120.195.71.156", "port": 7709},
    {"source": "华泰证券", "name": "华泰证券(南京联通一)", "host": "122.96.107.242", "port": 7709},
    {"source": "华泰证券", "name": "华泰证券(南京联通二)", "host": "122.96.107.243", "port": 7709},
    {"source": "华泰证券", "name": "华泰证券(亚马逊一)", "host": "52.83.39.241", "port": 7709},
    {"source": "华泰证券", "name": "华泰证券(亚马逊二)", "host": "52.83.199.101", "port": 7709},
    {"source": "华泰证券", "name": "华泰证券(华南阿里云一)", "host": "8.135.57.58", "port": 7709},
    {"source": "华泰证券", "name": "华泰证券(华南阿里云二)", "host": "8.135.62.177", "port": 7709},
    {"source": "华泰证券", "name": "华泰证券(华东华为云一)", "host": "124.70.183.173", "port": 7709},
    {"source": "华泰证券", "name": "华泰证券(华东华为云二)", "host": "124.71.163.106", "port": 7709},
    {"source": "国泰君安", "name": "郑州网通行情一", "host": "182.118.47.141", "port": 7709},
    {"source": "国泰君安", "name": "郑州网通行情二", "host": "182.118.47.168", "port": 7709},
    {"source": "国泰君安", "name": "郑州网通行情三", "host": "182.118.47.169", "port": 7709},
    {"source": "国泰君安", "name": "武汉电信行情一", "host": "119.97.164.184", "port": 7709},
    {"source": "国泰君安", "name": "武汉电信行情二", "host": "119.97.164.189", "port": 7709},
    {"source": "国泰君安", "name": "武汉电信行情三", "host": "116.211.121.102", "port": 7709},
    {"source": "国泰君安", "name": "武汉电信行情四", "host": "116.211.121.108", "port": 7709},
    {"source": "国泰君安", "name": "武汉电信行情五", "host": "116.211.121.31", "port": 7709},
    {"source": "国泰君安", "name": "新疆电信云行情一", "host": "202.100.166.117", "port": 7709},
    {"source": "国泰君安", "name": "新疆电信云行情二", "host": "202.100.166.118", "port": 7709},
    {"source": "国泰君安", "name": "上海电信行情八", "host": "222.73.139.166", "port": 7709},
    {"source": "国泰君安", "name": "上海电信行情九", "host": "222.73.139.167", "port": 7709},
    {"source": "国泰君安", "name": "上海电信行情十", "host": "222.73.139.168", "port": 7709},
    {"source": "国泰君安", "name": "上海BGP行情一", "host": "103.251.85.90", "port": 7709},
    {"source": "国泰君安", "name": "北京联通行情一", "host": "123.125.108.213", "port": 7709},
    {"source": "国泰君安", "name": "北京联通行情二", "host": "123.125.108.214", "port": 7709},
    {"source": "国泰君安", "name": "上海电信行情六", "host": "222.73.139.151", "port": 7709},
    {"source": "国泰君安", "name": "上海电信行情七", "host": "222.73.139.152", "port": 7709},
    {"source": "国泰君安", "name": "成都BGP行情一", "host": "148.70.110.41", "port": 7709},
    {"source": "国泰君安", "name": "成都BGP行情二", "host": "148.70.93.117", "port": 7709},
    {"source": "国泰君安", "name": "成都BGP行情三", "host": "148.70.31.16", "port": 7709},
    {"source": "国泰君安", "name": "成都BGP行情四", "host": "148.70.111.63", "port": 7709},
    {"source": "国泰君安", "name": "广州BGP行情一", "host": "139.159.143.228", "port": 7709},
    {"source": "国泰君安", "name": "广州BGP行情二", "host": "139.159.183.76", "port": 7709},
    {"source": "国泰君安", "name": "广州BGP行情三", "host": "139.159.193.118", "port": 7709},
    {"source": "国泰君安", "name": "广州BGP行情四", "host": "139.159.195.177", "port": 7709},
    {"source": "国泰君安", "name": "广州BGP行情五", "host": "139.159.202.253", "port": 7709},
    {"source": "国泰君安", "name": "广州BGP行情六", "host": "139.159.214.78", "port": 7709},
    {"source": "国泰君安", "name": "广州BGP行情七", "host": "139.9.38.206", "port": 7709},
    {"source": "国泰君安", "name": "广州BGP行情八", "host": "139.9.43.104", "port": 7709},
    {"source": "国泰君安", "name": "广州BGP行情九", "host": "139.9.43.31", "port": 7709},
    {"source": "国泰君安", "name": "广州BGP行情十", "host": "139.9.50.246", "port": 7709},
    {"source": "国泰君安", "name": "广州BGP行情十一", "host": "139.9.52.158", "port": 7709},
    {"source": "国泰君安", "name": "广州BGP行情十二", "host": "139.9.90.169", "port": 7709},
    {"source": "国泰君安", "name": "上海电信行情十一", "host": "101.226.180.73", "port": 7709},
    {"source": "国泰君安", "name": "上海电信行情十二", "host": "101.226.180.74", "port": 7709},
    {"source": "国泰君安", "name": "上海BGP行情六", "host": "103.251.85.200", "port": 7709},
    {"source": "国泰君安", "name": "上海BGP行情七", "host": "103.251.85.201", "port": 7709},
    {"source": "国泰君安", "name": "南京电信行情一", "host": "103.221.142.65", "port": 7709},
    {"source": "国泰君安", "name": "南京电信行情二", "host": "103.221.142.66", "port": 7709},
    {"source": "国泰君安", "name": "南京电信行情三", "host": "103.221.142.67", "port": 7709},
    {"source": "国泰君安", "name": "南京电信行情四", "host": "103.221.142.68", "port": 7709},
    {"source": "国泰君安", "name": "南京电信行情五", "host": "103.221.142.69", "port": 7709},
    {"source": "国泰君安", "name": "南京电信行情六", "host": "103.221.142.70", "port": 7709},
    {"source": "国泰君安", "name": "南京电信行情七", "host": "103.221.142.71", "port": 7709},
    {"source": "国泰君安", "name": "南京电信行情八", "host": "103.221.142.72", "port": 7709},
    {"source": "国泰君安", "name": "西安电信行情一", "host": "117.34.114.13", "port": 7709},
    {"source": "国泰君安", "name": "西安电信行情二", "host": "117.34.114.14", "port": 7709},
    {"source": "国泰君安", "name": "西安电信行情三", "host": "117.34.114.15", "port": 7709},
    {"source": "国泰君安", "name": "西安电信行情四", "host": "117.34.114.16", "port": 7709},
    {"source": "国泰君安", "name": "西安电信行情五", "host": "117.34.114.17", "port": 7709},
    {"source": "国泰君安", "name": "西安电信行情六", "host": "117.34.114.18", "port": 7709},
    {"source": "国泰君安", "name": "西安电信行情七", "host": "117.34.114.20", "port": 7709},
    {"source": "国泰君安", "name": "西安电信行情八", "host": "117.34.114.27", "port": 7709},
    {"source": "国泰君安", "name": "西安电信行情九", "host": "117.34.114.30", "port": 7709},
    {"source": "国泰君安", "name": "上海BGP行情八", "host": "103.251.85.202", "port": 7709},
    {"source": "国泰君安", "name": "东莞电信行情一", "host": "183.60.224.142", "port": 7709},
    {"source": "国泰君安", "name": "东莞电信行情二", "host": "183.60.224.143", "port": 7709},
    {"source": "国泰君安", "name": "东莞电信行情三", "host": "183.60.224.144", "port": 7709},
    {"source": "国泰君安", "name": "东莞电信行情四", "host": "183.60.224.145", "port": 7709},
    {"source": "国泰君安", "name": "东莞电信行情五", "host": "183.60.224.146", "port": 7709},
    {"source": "国泰君安", "name": "东莞电信行情六", "host": "183.60.224.147", "port": 7709},
    {"source": "国泰君安", "name": "东莞电信行情七", "host": "183.60.224.148", "port": 7709},
]

ExtensionServerList: List[Dict[str, Any]] = [
    {"source": "通达信", "name": "扩展市场深圳双线1", "host": "112.74.214.43", "port": 7727},
    {"source": "通达信", "name": "扩展市场深圳双线2", "host": "120.25.218.6", "port": 7727},
    {"source": "通达信", "name": "扩展市场深圳双线3", "host": "47.107.75.159", "port": 7727},
    {"source": "通达信", "name": "扩展市场深圳双线4", "host": "47.106.204.218", "port": 7727},
    {"source": "通达信", "name": "扩展市场深圳双线5", "host": "47.106.209.131", "port": 7727},
    {"source": "通达信", "name": "扩展市场武汉主站1", "host": "119.97.185.5", "port": 7727},
    {"source": "通达信", "name": "扩展市场深圳双线6", "host": "47.115.94.72", "port": 7727},
    {"source": "通达信", "name": "扩展市场上海双线1", "host": "106.14.95.149", "port": 7727},
    {"source": "通达信", "name": "扩展市场上海双线2", "host": "47.102.108.214", "port": 7727},
    {"source": "通达信", "name": "扩展市场上海双线3", "host": "47.103.86.229", "port": 7727},
    {"source": "通达信", "name": "扩展市场上海双线4", "host": "47.103.88.146", "port": 7727},
    {"source": "通达信", "name": "扩展市场广州双线1", "host": "116.205.143.214", "port": 7727},
    {"source": "通达信", "name": "扩展市场广州双线2", "host": "124.71.223.19", "port": 7727},
    {"source": "中信证券", "name": "上海电信主站Z1", "host": "180.153.18.176", "port": 7721},
    {"source": "中信证券", "name": "北京联通主站Z1", "host": "202.108.253.154", "port": 7721},
    {"source": "中信证券", "name": "杭州电信主站J1", "host": "115.238.56.196", "port": 7721},
    {"source": "中信证券", "name": "杭州电信主站J2", "host": "115.238.90.170", "port": 7721},
    {"source": "中信证券", "name": "杭州联通主站J1", "host": "60.12.136.251", "port": 7721},
    {"source": "中信证券", "name": "杭州华数主站J1", "host": "218.108.98.244", "port": 7721},
    {"source": "中信证券", "name": "济南联通主站W1", "host": "27.221.115.133", "port": 7721},
    {"source": "中信证券", "name": "青岛电信主站W1", "host": "58.56.180.60", "port": 7721},
    {"source": "中信证券", "name": "深圳电信主站Z1", "host": "14.17.75.71", "port": 7721},
    {"source": "中信证券", "name": "广州云电信主站Z1", "host": "121.201.83.104", "port": 7721},
    {"source": "华泰证券", "name": "华泰证券(南京电信一)", "host": "180.101.48.170", "port": 7721},
    {"source": "华泰证券", "name": "华泰证券(南京电信二)", "host": "180.101.48.171", "port": 7721},
    {"source": "华泰证券", "name": "华泰证券(南京移动一)", "host": "120.195.71.155", "port": 7721},
    {"source": "华泰证券", "name": "华泰证券(南京移动二)", "host": "120.195.71.156", "port": 7721},
    {"source": "华泰证券", "name": "华泰证券(南京联通一)", "host": "122.96.107.242", "port": 7721},
    {"source": "华泰证券", "name": "华泰证券(南京联通二)", "host": "122.96.107.243", "port": 7721},
    {"source": "华泰证券", "name": "华泰证券(亚马逊一)", "host": "52.83.39.241", "port": 7721},
    {"source": "华泰证券", "name": "华泰证券(亚马逊二)", "host": "52.83.199.101", "port": 7721},
    {"source": "华泰证券", "name": "华泰证券(华南阿里云一)", "host": "8.135.57.58", "port": 7721},
    {"source": "华泰证券", "name": "华泰证券(华南阿里云二)", "host": "8.135.62.177", "port": 7721},
    {"source": "华泰证券", "name": "华泰证券(华东华为云一)", "host": "124.70.183.173", "port": 7721},
    {"source": "华泰证券", "name": "华泰证券(华东华为云二)", "host": "124.71.163.106", "port": 7721},
    {"source": "国泰君安", "name": "扩展行情主站1", "host": "103.221.142.80", "port": 7721},
    {"source": "国泰君安", "name": "扩展行情主站2", "host": "114.118.82.205", "port": 7721},
    {"source": "国泰君安", "name": "扩展行情主站3", "host": "117.34.114.31", "port": 7721},
    {"source": "国泰君安", "name": "扩展行情主站4", "host": "139.9.52.158", "port": 7721},
    {"source": "国泰君安", "name": "扩展行情主站5", "host": "103.251.85.204", "port": 7721},
    {"source": "国泰君安", "name": "扩展行情主站6", "host": "114.118.82.204", "port": 7721},
    {"source": "国泰君安", "name": "扩展行情主站7", "host": "103.221.142.73", "port": 7721},
]


def _cache_filename() -> str:
    meta = getattr(qconfig, "quant1x_config", None)
    if meta is None:
        # fallback to ~/.quant1x/meta/server.bin
        home = os.path.expanduser("~")
        return os.path.join(home, ".quant1x", "meta", "server.bin")
    return os.path.join(meta.meta_path, "server.bin")


def write_cache(servers: List[Dict[str, Any]]) -> None:
    fn = _cache_filename()
    try:
        os.makedirs(os.path.dirname(fn), exist_ok=True)
        with open(fn, "w", encoding="utf-8") as f:
            # Align with Go's serverListWrapper structure
            data = {"standard": servers, "extension": []}
            yaml.safe_dump(data, f, allow_unicode=True)
    except Exception:
        log.exception("Failed to write server cache %s", fn)


def read_cache() -> List[Dict[str, Any]]:
    fn = _cache_filename()
    try:
        if not os.path.isfile(fn):
            return []
        with open(fn, "r", encoding="utf-8") as f:
            data = yaml.safe_load(f) or {}
            # Handle both list (legacy) and dict (new wrapper) formats
            if isinstance(data, list):
                return data
            if isinstance(data, dict):
                return data.get("standard", [])
    except Exception:
        log.exception("Failed to read server cache %s", fn)
    return []


def _try_probe_one(candidate: Dict[str, Any], timeout_ms: int, result_list: List[Dict[str, Any]], lock: threading.Lock) -> None:
    # defensive conversions to satisfy static type checkers and avoid None
    host = str(candidate.get("host") or "")
    port = int(candidate.get("port") or 0)
    name = str(candidate.get("name") or "")
    source = str(candidate.get("source") or "")
    start = time.monotonic()
    sock = None
    try:
        sock = socket.create_connection((host, port), timeout=timeout_ms / 1000.0)
        # set TCP_NODELAY where possible
        try:
            sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        except Exception:
            pass

        # perform protocol handshake using level1 StandardProtocolHandler so we reuse same logic
        try:
            # import lazily to avoid circular import at module import time
            from quant1x.level1.client import StandardProtocolHandler

            handler = StandardProtocolHandler()
            ok = handler.handshake(sock)
        except Exception as e:
            # log.debug instead of exception to avoid noisy tracebacks during detection
            log.debug("Handshake failed for %s:%s: %s", host, port, e)
            ok = False

        if ok:
            elapsed = int((time.monotonic() - start) * 1000)
            entry: Dict[str, Any] = {"source": source, "name": name, "host": host, "port": port, "latency_ms": elapsed}
            with lock:
                result_list.append(entry)
    except Exception:
        # connect failed or timed out
        log.debug("Probe failed for %s:%s", host, port)
    finally:
        try:
            if sock is not None:
                sock.close()
        except Exception:
            pass


def detect(elapsed_time_ms: int = 200, conn_limit: int = 10, connect_timeout_ms: int = 1000) -> List[Dict[str, Any]]:
    """Detect available level1 servers.

    Returns a list of server dicts with fields: Name, Host, Port, latency_ms.
    """
    # IMPORTANT: By design (same as the C++ implementation), detection only
    # probes the `StandardServerList`. `ExtensionServerList` entries exist in
    # this module as additional, extension endpoints, but they are NOT probed
    # by default because their protocol/usage may differ. If you need to
    # test extension servers, do that explicitly with a separate routine.
    candidates = list(StandardServerList)
    if not candidates:
        return []

    num_threads = min(len(candidates), max(1, (os.cpu_count() or 1)))
    threads: List[threading.Thread] = []
    results: List[Dict[str, Any]] = []
    lock = threading.Lock()

    # round-robin distribute candidates across threads
    for cand in candidates:
        t = threading.Thread(target=_try_probe_one, args=(cand, connect_timeout_ms, results, lock), daemon=True)
        threads.append(t)
        t.start()

    # wait for threads with a global timeout (slightly larger than connect timeout)
    deadline = time.monotonic() + (connect_timeout_ms / 1000.0) + 1.0
    for t in threads:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            break
        t.join(timeout=remaining)

    # sort by latency and return top conn_limit items
    # ensure the sort key is numeric to satisfy type checkers
    results.sort(key=lambda x: int(x.get("latency_ms", 999999)))
    selected = results[:conn_limit]
    return selected
