"""Server detection ported from C++ level1::detect().

Provides:
- StandardServerList: candidate servers (Name, Host, Port)
- detect(...): parallel connect + handshake to rank servers by latency
- read_cache / write_cache to persist detection results to meta/server.bin

This module intentionally keeps the detect implementation conservative:
it uses blocking sockets with short timeouts and runs workers in threads so
it works in environments without asyncio.
"""
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
    {"Name": "通达信", "Host": "110.41.147.114", "Port": 7709},
    {"Name": "通达信", "Host": "110.41.2.72", "Port": 7709},
    {"Name": "通达信", "Host": "110.41.4.4", "Port": 7709},
    {"Name": "通达信", "Host": "47.113.94.204", "Port": 7709},
    {"Name": "通达信", "Host": "8.129.174.169", "Port": 7709},
    {"Name": "通达信", "Host": "110.41.154.219", "Port": 7709},
    {"Name": "通达信", "Host": "124.70.176.52", "Port": 7709},
    {"Name": "通达信", "Host": "47.100.236.28", "Port": 7709},
    {"Name": "通达信", "Host": "123.60.186.45", "Port": 7709},
    {"Name": "通达信", "Host": "123.60.164.122", "Port": 7709},
    {"Name": "通达信", "Host": "47.116.105.28", "Port": 7709},
    {"Name": "通达信", "Host": "124.70.199.56", "Port": 7709},
    {"Name": "通达信", "Host": "121.36.54.217", "Port": 7709},
    {"Name": "通达信", "Host": "121.36.81.195", "Port": 7709},
    {"Name": "通达信", "Host": "123.249.15.60", "Port": 7709},
    {"Name": "通达信", "Host": "124.71.85.110", "Port": 7709},
    {"Name": "通达信", "Host": "139.9.51.18", "Port": 7709},
    {"Name": "通达信", "Host": "139.159.239.163", "Port": 7709},
    {"Name": "通达信", "Host": "106.14.201.131", "Port": 7709},
    {"Name": "通达信", "Host": "106.14.190.242", "Port": 7709},
    {"Name": "通达信", "Host": "121.36.225.169", "Port": 7709},
    {"Name": "通达信", "Host": "123.60.70.228", "Port": 7709},
    {"Name": "通达信", "Host": "123.60.73.44", "Port": 7709},
    {"Name": "通达信", "Host": "124.70.133.119", "Port": 7709},
    {"Name": "通达信", "Host": "124.71.187.72", "Port": 7709},
    {"Name": "通达信", "Host": "124.71.187.122", "Port": 7709},
    {"Name": "通达信", "Host": "119.97.185.59", "Port": 7709},
    {"Name": "通达信", "Host": "47.107.64.168", "Port": 7709},
    {"Name": "通达信", "Host": "124.70.75.113", "Port": 7709},
    {"Name": "通达信", "Host": "124.71.9.153", "Port": 7709},
    {"Name": "通达信", "Host": "123.60.84.66", "Port": 7709},
    {"Name": "通达信", "Host": "47.107.228.47", "Port": 7719},
    {"Name": "通达信", "Host": "120.46.186.223", "Port": 7709},
    {"Name": "通达信", "Host": "124.70.22.210", "Port": 7709},
    {"Name": "通达信", "Host": "139.9.133.247", "Port": 7709},
    {"Name": "通达信", "Host": "116.205.163.254", "Port": 7709},
    {"Name": "通达信", "Host": "116.205.171.132", "Port": 7709},
    {"Name": "通达信", "Host": "116.205.183.150", "Port": 7709},
    {"Name": "中信证券", "Host": "180.153.18.170", "Port": 7709},
    {"Name": "中信证券", "Host": "180.153.18.171", "Port": 7709},
    {"Name": "中信证券", "Host": "202.108.253.130", "Port": 7709},
    {"Name": "中信证券", "Host": "202.108.253.131", "Port": 7709},
    {"Name": "中信证券", "Host": "60.191.117.167", "Port": 7709},
    {"Name": "中信证券", "Host": "115.238.56.198", "Port": 7709},
    {"Name": "中信证券", "Host": "218.75.126.9", "Port": 7709},
    {"Name": "中信证券", "Host": "115.238.90.165", "Port": 7709},
    {"Name": "中信证券", "Host": "124.160.88.183", "Port": 7709},
    {"Name": "中信证券", "Host": "60.12.136.250", "Port": 7709},
    {"Name": "中信证券", "Host": "218.108.98.244", "Port": 7709},
    {"Name": "中信证券", "Host": "218.108.47.69", "Port": 7709},
    {"Name": "中信证券", "Host": "27.221.115.131", "Port": 7709},
    {"Name": "中信证券", "Host": "58.56.180.60", "Port": 7709},
    {"Name": "中信证券", "Host": "14.17.75.71", "Port": 7709},
    {"Name": "中信证券", "Host": "114.80.63.12", "Port": 7709},
    {"Name": "中信证券", "Host": "114.80.63.35", "Port": 7709},
    {"Name": "中信证券", "Host": "180.153.39.51", "Port": 7709},
    {"Name": "中信证券", "Host": "123.125.108.23", "Port": 7709},
    {"Name": "中信证券", "Host": "123.125.108.24", "Port": 7709},
    {"Name": "中信证券", "Host": "121.201.83.106", "Port": 7709},
    {"Name": "中信证券", "Host": "218.6.170.55", "Port": 7709},
    {"Name": "华泰证券", "Host": "180.101.48.170", "Port": 7709},
    {"Name": "华泰证券", "Host": "180.101.48.171", "Port": 7709},
    {"Name": "华泰证券", "Host": "120.195.71.155", "Port": 7709},
    {"Name": "华泰证券", "Host": "120.195.71.156", "Port": 7709},
    {"Name": "华泰证券", "Host": "122.96.107.242", "Port": 7709},
    {"Name": "华泰证券", "Host": "122.96.107.243", "Port": 7709},
    {"Name": "华泰证券", "Host": "52.83.39.241", "Port": 7709},
    {"Name": "华泰证券", "Host": "52.83.199.101", "Port": 7709},
    {"Name": "华泰证券", "Host": "8.135.57.58", "Port": 7709},
    {"Name": "华泰证券", "Host": "8.135.62.177", "Port": 7709},
    {"Name": "华泰证券", "Host": "124.70.183.173", "Port": 7709},
    {"Name": "华泰证券", "Host": "124.71.163.106", "Port": 7709},
    {"Name": "国泰君安", "Host": "182.118.47.141", "Port": 7709},
    {"Name": "国泰君安", "Host": "182.118.47.168", "Port": 7709},
    {"Name": "国泰君安", "Host": "182.118.47.169", "Port": 7709},
    {"Name": "国泰君安", "Host": "119.97.164.184", "Port": 7709},
    {"Name": "国泰君安", "Host": "119.97.164.189", "Port": 7709},
    {"Name": "国泰君安", "Host": "116.211.121.102", "Port": 7709},
    {"Name": "国泰君安", "Host": "116.211.121.108", "Port": 7709},
    {"Name": "国泰君安", "Host": "116.211.121.31", "Port": 7709},
    {"Name": "国泰君安", "Host": "202.100.166.117", "Port": 7709},
    {"Name": "国泰君安", "Host": "202.100.166.118", "Port": 7709},
    {"Name": "国泰君安", "Host": "222.73.139.166", "Port": 7709},
    {"Name": "国泰君安", "Host": "123.125.108.213", "Port": 7709},
    {"Name": "国泰君安", "Host": "123.125.108.214", "Port": 7709},
    {"Name": "国泰君安", "Host": "222.73.139.151", "Port": 7709},
    {"Name": "国泰君安", "Host": "222.73.139.152", "Port": 7709},
    {"Name": "国泰君安", "Host": "148.70.110.41", "Port": 7709},
    {"Name": "国泰君安", "Host": "148.70.93.117", "Port": 7709},
    {"Name": "国泰君安", "Host": "148.70.31.16", "Port": 7709},
    {"Name": "国泰君安", "Host": "148.70.111.63", "Port": 7709},
    {"Name": "国泰君安", "Host": "139.159.143.228", "Port": 7709},
    {"Name": "国泰君安", "Host": "139.159.183.76", "Port": 7709},
    {"Name": "国泰君安", "Host": "139.159.193.118", "Port": 7709},
    {"Name": "国泰君安", "Host": "139.159.195.177", "Port": 7709},
    {"Name": "国泰君安", "Host": "139.159.202.253", "Port": 7709},
    {"Name": "国泰君安", "Host": "139.159.214.78", "Port": 7709},
    {"Name": "国泰君安", "Host": "139.9.38.206", "Port": 7709},
    {"Name": "国泰君安", "Host": "139.9.43.104", "Port": 7709},
    {"Name": "国泰君安", "Host": "139.9.43.31", "Port": 7709},
    {"Name": "国泰君安", "Host": "139.9.50.246", "Port": 7709},
    {"Name": "国泰君安", "Host": "139.9.52.158", "Port": 7709},
    {"Name": "国泰君安", "Host": "139.9.90.169", "Port": 7709},
    {"Name": "国泰君安", "Host": "101.226.180.73", "Port": 7709},
    {"Name": "国泰君安", "Host": "101.226.180.74", "Port": 7709},
    {"Name": "国泰君安", "Host": "103.251.85.200", "Port": 7709},
    {"Name": "国泰君安", "Host": "103.251.85.201", "Port": 7709},
    {"Name": "国泰君安", "Host": "103.221.142.65", "Port": 7709},
    {"Name": "国泰君安", "Host": "103.221.142.66", "Port": 7709},
    {"Name": "国泰君安", "Host": "103.221.142.67", "Port": 7709},
    {"Name": "国泰君安", "Host": "103.221.142.68", "Port": 7709},
    {"Name": "国泰君安", "Host": "103.221.142.69", "Port": 7709},
    {"Name": "国泰君安", "Host": "103.221.142.70", "Port": 7709},
    {"Name": "国泰君安", "Host": "103.221.142.71", "Port": 7709},
    {"Name": "国泰君安", "Host": "103.221.142.72", "Port": 7709},
    {"Name": "国泰君安", "Host": "117.34.114.13", "Port": 7709},
    {"Name": "国泰君安", "Host": "117.34.114.14", "Port": 7709},
    {"Name": "国泰君安", "Host": "117.34.114.15", "Port": 7709},
    {"Name": "国泰君安", "Host": "117.34.114.16", "Port": 7709},
    {"Name": "国泰君安", "Host": "117.34.114.17", "Port": 7709},
    {"Name": "国泰君安", "Host": "117.34.114.18", "Port": 7709},
    {"Name": "国泰君安", "Host": "117.34.114.20", "Port": 7709},
    {"Name": "国泰君安", "Host": "117.34.114.27", "Port": 7709},
    {"Name": "国泰君安", "Host": "117.34.114.30", "Port": 7709},
    {"Name": "国泰君安", "Host": "103.251.85.202", "Port": 7709},
    {"Name": "国泰君安", "Host": "183.60.224.142", "Port": 7709},
    {"Name": "国泰君安", "Host": "183.60.224.143", "Port": 7709},
    {"Name": "国泰君安", "Host": "183.60.224.144", "Port": 7709},
    {"Name": "国泰君安", "Host": "183.60.224.145", "Port": 7709},
    {"Name": "国泰君安", "Host": "183.60.224.146", "Port": 7709},
    {"Name": "国泰君安", "Host": "183.60.224.147", "Port": 7709},
    {"Name": "国泰君安", "Host": "183.60.224.148", "Port": 7709}
]

# Extension/expanded candidate list
ExtensionServerList: List[Dict[str, Any]] = [
    {"Name": "通达信", "Host": "112.74.214.43", "Port": 7727},
    {"Name": "通达信", "Host": "120.25.218.6", "Port": 7727},
    {"Name": "通达信", "Host": "47.107.75.159", "Port": 7727},
    {"Name": "通达信", "Host": "47.106.204.218", "Port": 7727},
    {"Name": "通达信", "Host": "47.106.209.131", "Port": 7727},
    {"Name": "通达信", "Host": "119.97.185.5", "Port": 7727},
    {"Name": "通达信", "Host": "47.115.94.72", "Port": 7727},
    {"Name": "通达信", "Host": "106.14.95.149", "Port": 7727},
    {"Name": "通达信", "Host": "47.102.108.214", "Port": 7727},
    {"Name": "通达信", "Host": "47.103.86.229", "Port": 7727},
    {"Name": "通达信", "Host": "47.103.88.146", "Port": 7727},
    {"Name": "通达信", "Host": "116.205.143.214", "Port": 7727},
    {"Name": "通达信", "Host": "124.71.223.19", "Port": 7727},
    {"Name": "中信证券", "Host": "180.153.18.176", "Port": 7721},
    {"Name": "中信证券", "Host": "202.108.253.154", "Port": 7721},
    {"Name": "中信证券", "Host": "115.238.56.196", "Port": 7721},
    {"Name": "中信证券", "Host": "115.238.90.170", "Port": 7721},
    {"Name": "中信证券", "Host": "60.12.136.251", "Port": 7721},
    {"Name": "中信证券", "Host": "218.108.98.244", "Port": 7721},
    {"Name": "中信证券", "Host": "27.221.115.133", "Port": 7721},
    {"Name": "中信证券", "Host": "58.56.180.60", "Port": 7721},
    {"Name": "中信证券", "Host": "14.17.75.71", "Port": 7721},
    {"Name": "中信证券", "Host": "121.201.83.104", "Port": 7721},
    {"Name": "华泰证券", "Host": "180.101.48.170", "Port": 7721},
    {"Name": "华泰证券", "Host": "180.101.48.171", "Port": 7721},
    {"Name": "华泰证券", "Host": "120.195.71.155", "Port": 7721},
    {"Name": "华泰证券", "Host": "120.195.71.156", "Port": 7721},
    {"Name": "华泰证券", "Host": "122.96.107.242", "Port": 7721},
    {"Name": "华泰证券", "Host": "122.96.107.243", "Port": 7721},
    {"Name": "华泰证券", "Host": "52.83.39.241", "Port": 7721},
    {"Name": "华泰证券", "Host": "52.83.199.101", "Port": 7721},
    {"Name": "华泰证券", "Host": "8.135.57.58", "Port": 7721},
    {"Name": "华泰证券", "Host": "8.135.62.177", "Port": 7721},
    {"Name": "华泰证券", "Host": "124.70.183.173", "Port": 7721},
    {"Name": "华泰证券", "Host": "124.71.163.106", "Port": 7721},
    {"Name": "国泰君安", "Host": "103.221.142.80", "Port": 7721},
    {"Name": "国泰君安", "Host": "114.118.82.205", "Port": 7721},
    {"Name": "国泰君安", "Host": "117.34.114.31", "Port": 7721},
    {"Name": "国泰君安", "Host": "139.9.52.158", "Port": 7721},
    {"Name": "国泰君安", "Host": "103.251.85.204", "Port": 7721},
    {"Name": "国泰君安", "Host": "114.118.82.204", "Port": 7721},
    {"Name": "国泰君安", "Host": "103.221.142.73", "Port": 7721}
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
            yaml.safe_dump(servers, f, allow_unicode=True)
    except Exception:
        log.exception("Failed to write server cache %s", fn)


def read_cache() -> List[Dict[str, Any]]:
    fn = _cache_filename()
    try:
        if not os.path.isfile(fn):
            return []
        with open(fn, "r", encoding="utf-8") as f:
            data = yaml.safe_load(f) or []
            if isinstance(data, list):
                return data
    except Exception:
        log.exception("Failed to read server cache %s", fn)
    return []


def _try_probe_one(candidate: Dict[str, Any], timeout_ms: int, result_list: List[Dict[str, Any]], lock: threading.Lock) -> None:
    # defensive conversions to satisfy static type checkers and avoid None
    host = str(candidate.get("Host") or "")
    port = int(candidate.get("Port") or 0)
    name = str(candidate.get("Name") or "")
    start = time.monotonic()
    sock = None
    try:
        sock = socket.create_connection((host, port), timeout=timeout_ms / 1000.0)
        # set TCP_NODELAY where possible
        try:
            sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        except Exception:
            pass

        # perform protocol handshake using level1 ProtocolHandler so we reuse same logic
        try:
            # import lazily to avoid circular import at module import time
            from quant1x.level1.client import ProtocolHandler

            handler = ProtocolHandler()
            ok = handler.handshake(sock)
        except Exception:
            log.exception("Handshake exception for %s:%s", host, port)
            ok = False

        if ok:
            elapsed = int((time.monotonic() - start) * 1000)
            entry: Dict[str, Any] = {"Name": name, "Host": host, "Port": port, "latency_ms": elapsed}
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
    # this module as additional, extended endpoints, but they are NOT probed
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
