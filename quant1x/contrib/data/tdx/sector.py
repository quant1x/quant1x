# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

from enum import IntEnum
import os
import struct
import json
from dataclasses import dataclass, field
from typing import List, Optional
import pandas as pd

from quant1x.log import logger
from quant1x.runtime.once import RollingOnce
from quant1x.data.meta import Timestamp
from quant1x.data.schema import Sector
from quant1x.config import config
from quant1x.data import status as market_status
from quant1x.data import market
from quant1x.data.meta import tradinghours

from .client import get_std_conn
from .level1 import BlockFileContext, BLOCK_CHUNKS_SIZE
from . import protocol as l1protocol
from quant1x.data.meta.calendar import last_trading_day

class SectorType(IntEnum):
    """板块类型枚举"""
    UNKNOWN = 0  # 未知类型
    HANGYE = 2  # 行业
    DIQU = 3  # 地区
    GAINIAN = 4  # 概念
    FENGGE = 5  # 风格
    ZHISHU = 6  # 指数
    YJHY = 12  # 研究行业


# 板块类型名称映射
_SECTOR_TYPE_NAME_MAP = {
    SectorType.HANGYE: "行业",
    SectorType.DIQU: "地区",
    SectorType.GAINIAN: "概念",
    SectorType.FENGGE: "风格",
    SectorType.ZHISHU: "指数",
    SectorType.YJHY: "研究行业",
}


def sector_type_name_by_code(sector_code: int) -> str | None:
    """通过板块类型代码获取板块类型名称

    Args:
        sector_code: 板块类型代码

    Returns:
        板块类型名称, 找不到返回 None
    """
    sector_type = SectorType(sector_code)
    return sector_type_name_by_type(sector_type)


def sector_type_name_by_type(sector_type: SectorType) -> str | None:
    """通过板块类型代码获取板块类型名称

    Args:
        sector_type: 板块类型枚举

    Returns:
        板块类型名称, 找不到返回 None
    """
    return _SECTOR_TYPE_NAME_MAP.get(sector_type)


BLOCK_ZHISHU  = "block_zs.dat" # 指数
BLOCK_FENGGE  = "block_fg.dat" # 风格
BLOCK_GAINIAN = "block_gn.dat" # 概念
BLOCK_DEFAULT = "block.dat"    # 早期的板块数据文件, 与block_zs.dat

# BLK_ZIP_FILENAME = "zhb.zip"
# BLK_ZS_FILENAME  = "tdxzs.cfg"
# BLK_ZS3_FILENAME = "tdxzs3.cfg"

def get_sector_filename(date: str = "") -> str:
    name = "blocks"
    current_date = Timestamp.now()
    if len(date) > 0:
        current_date = Timestamp.parse(date)
    cache_date = last_trading_day(current_date).only_date()
    # place generated sector file in the configured meta path
    filename = f'{name}.{cache_date}'
    return os.path.join(config.meta_path, filename)


def _parse_constituent_field(val: str) -> List[str]:
    if not val:
        return []
    val = val.strip()
    try:
        lst = json.loads(val)
        if isinstance(lst, List):
            return [str(market.detect_symbol(s.strip())) for s in lst if s]
    except Exception:
        # fallback: comma or pipe separated list
        sep = ","
        if "|" in val:
            sep = "|"
        parts = [p.strip() for p in val.split(sep) if p.strip()]
        return [str(market.detect_symbol(p)) for p in parts]
    return []


_global_block_list: List[Sector] = []
_map_block: dict = {}

def _get_block_info_from_level1(filename: str) -> Optional[bytes]:
    try:
        with get_std_conn() as conn:
            start = 0
            result = bytearray()
            while True:
                msg = BlockFileContext(filename, start)
                l1protocol.transact_message_sync(conn, msg)
                if msg.size == 0:
                    return None
                if msg.size > 0:
                    result.extend(msg.data)
                if msg.size < BLOCK_CHUNKS_SIZE:
                    break
                start += msg.size
            return bytes(result)
    except Exception:
        return None


def download_block_raw_data(filename: str) -> Optional[str]:
    dst_dir = config.meta_path
    os.makedirs(dst_dir, exist_ok=True)
    fn = os.path.join(dst_dir, filename)

    # mirror Go: if file exists and is not due for initialization, skip download
    try:
        if os.path.isfile(fn):
            try:
                if not market_status.should_initialize_file(fn):
                    return fn
            except Exception:
                # on any error fall through to attempt download
                pass
    except Exception:
        pass

    data = _get_block_info_from_level1(filename)
    if not data:
        return None
    try:
        with open(fn, 'wb') as fh:
            fh.write(data)
        return fn
    except Exception:
        return None


def update_cache_block_file() -> None:
    block_file = get_sector_filename()
    logger.info(f"update_cache_block_file: block_file={block_file}")
    create_or_update = market_status.should_initialize_file(block_file)
    if create_or_update:
        parse_and_generate_block_file()


def parse_raw_block_file(block_filename: str) -> List[dict]:
    fn = os.path.join(config.meta_path, block_filename)
    if not os.path.isfile(fn):
        return []
    try:
        with open(fn, 'rb') as fh:
            # skip 384 bytes header
            header = fh.read(384)
            cnt_bytes = fh.read(2)
            if len(cnt_bytes) < 2:
                return []
            count = struct.unpack('<H', cnt_bytes)[0]
            records = []
            for i in range(count):
                rec = fh.read(2813)
                if len(rec) < 2813:
                    break
                name_bytes = rec[0:9]
                num = struct.unpack('<H', rec[9:11])[0]
                block_type = struct.unpack('<H', rec[11:13])[0]
                codes = []
                offset = 13
                for j in range(400):
                    code_bytes = rec[offset:offset+7]
                    offset += 7
                    if not code_bytes:
                        continue
                    # strip nul and decode
                    try:
                        code = code_bytes.split(b'\x00', 1)[0].decode('ascii', errors='ignore')
                    except Exception:
                        code = code_bytes.split(b'\x00', 1)[0].decode('gbk', errors='ignore')
                    if code:
                        codes.append(code)
                try:
                    name = name_bytes.split(b'\x00', 1)[0].decode('gbk', errors='ignore')
                except Exception:
                    name = name_bytes.split(b'\x00', 1)[0].decode('ascii', errors='ignore')
                records.append({
                    'block_name': name,
                    'num': num,
                    'block_type': block_type,
                    'codes': codes,
                })
            return records
    except Exception:
        return []


def get_block_info_from_config(name: str):
    fn = os.path.join(config.meta_path, name)
    if not os.path.isfile(fn):
        return []
    res = []
    try:
        with open(fn, 'r', encoding='gbk', errors='ignore') as fh:
            for line in fh:
                line = line.strip()
                if not line:
                    continue
                arr = line.split('|')
                bi = {
                    'name': arr[0] if len(arr) > 0 else '',
                    'code': arr[1] if len(arr) > 1 else '',
                    'type': int(arr[2]) if len(arr) > 2 and arr[2].isdigit() else 0,
                    'block': arr[5] if len(arr) > 5 else '',
                }
                res.append(bi)
    except Exception:
        return []
    return res

@dataclass
class IndustryInfo:
    MarketId: int = 0
    Code: str = ''
    Block: str = ''
    Block5: str = ''
    XBlock: str = ''
    XBlock5: str = ''

def load_industry_blocks():
    fn = os.path.join(config.meta_path, 'tdxhy.cfg')
    if not os.path.isfile(fn):
        return []
    out = []
    try:
        with open(fn, 'r', encoding='gbk', errors='ignore') as fh:
            for line in fh:
                line = line.strip()
                if not line:
                    continue
                arr = line.split('|')
                bc = arr[2] if len(arr) > 2 else ''
                bc5 = bc[:5] if len(bc) >= 5 else bc
                xbc5 = arr[5] if len(arr) > 5 else ''
                xbc = xbc5[:5] if len(xbc5) >= 5 else xbc5
                try:
                    mid = int(arr[0]) if len(arr) > 0 and arr[0].isdigit() else 0
                except Exception:
                    mid = 0
                info = IndustryInfo(
                    MarketId=mid,
                    Code=arr[1] if len(arr) > 1 else '',
                    Block=bc,
                    Block5=bc5,
                    XBlock=xbc,
                    XBlock5=xbc5,
                )
                out.append(info)
    except Exception:
        return []
    return out

def industry_constituent_stock_list(hys: List[IndustryInfo], block: str) -> List[str]:
    lst = []
    for v in hys:
        if v.Block5.startswith(block) or v.XBlock5.startswith(block):
            lst.append(v.Code)
        elif v.Block5 == block or v.Block == block or v.XBlock5 == block or v.XBlock == block:
            lst.append(v.Code)
    lst.sort()
    return lst

def parse_and_generate_block_file():
    # 1) 加载zs*配置文件
    bks_cfg = ['tdxzs.cfg', 'tdxzs3.cfg']
    block_index = []
    tmp_map = {}
    for cfg in bks_cfg:
        bi = get_block_info_from_config(cfg)
        if len(bi) == 0:
            continue
        for v in bi:
            if v['code'] in tmp_map:
                continue
            tmp_map[v['code']] = v
            block_index.append(v)
    
    if not block_index:
        return None

    # block -> name mapping
    block2name = {v['block']: v['name'] for v in block_index if v.get('block')}

    # 2) parse raw block files and build name->rawinfo
    raw_files = [BLOCK_DEFAULT, BLOCK_GAINIAN, BLOCK_FENGGE, BLOCK_ZHISHU]
    name2block = {}
    for f in raw_files:
        recs = parse_raw_block_file(f)
        for bk in recs:
            block_name = bk['block_name']
            if block_name in block2name:
                block_name = block2name[block_name]
            name2block[block_name] = bk

    # 3) code->hy mapping
    code2hy = {}
    for v in block_index:
        if v['name'] != v['block']:
            code2hy[v['block']] = v['name']

    # 4) industry blocks
    hys = load_industry_blocks()

    # assemble final block entries
    rows = []
    for v in block_index:
        bn = v['name']
        entry_codes = []
        if bn in name2block:
            info = name2block[bn]
            for sc in info.get('codes', []):
                if len(sc) < 5:
                    continue
                entry_codes.append(sc)
            count = int(info.get('num', 0))
            rows.append({
                'name': v['name'],
                'code': v['code'],
                'type': v['type'],
                'count': count,
                'block': v['block'],
                'constituent_stocks': json.dumps(entry_codes, ensure_ascii=False),
            })
            continue

        # fallback: industry mapping
        bc = v['block']
        stock_list = industry_constituent_stock_list(hys, bc)
        if stock_list:
            rows.append({
                'name': v['name'],
                'code': v['code'],
                'type': v['type'],
                'count': len(stock_list),
                'block': v['block'],
                'constituent_stocks': json.dumps(stock_list, ensure_ascii=False),
            })

    # filter empty
    rows = [r for r in rows if r.get('constituent_stocks') and r.get('constituent_stocks') != '[]']
    if not rows:
        return None
    df = pd.DataFrame(rows, columns=['name', 'code', 'type', 'count', 'block', 'constituent_stocks'])
    out_fn = get_sector_filename()
    os.makedirs(os.path.dirname(out_fn), exist_ok=True)
    df.to_csv(out_fn, index=False)
    return out_fn


def sync_block_files():
    # industry config
    download_block_raw_data('tdxhy.cfg')

    # download zip and unzip
    zhb = download_block_raw_data('zhb.zip')
    if zhb:
        try:
            import zipfile
            need_files = ["tdxzs.cfg", "tdxzs3.cfg"]
            with zipfile.ZipFile(zhb, 'r') as z:
                for member in z.namelist():
                    base = os.path.basename(member)
                    if base in need_files:
                        try:
                            # extract member into meta path
                            z.extract(member, config.meta_path)
                            # if member contains a path, move to meta root
                            extracted = os.path.join(config.meta_path, member)
                            target = os.path.join(config.meta_path, base)
                            if os.path.exists(extracted) and extracted != target:
                                try:
                                    os.replace(extracted, target)
                                except Exception:
                                    pass
                        except Exception:
                            pass
        except Exception:
            pass

    # download standard block files
    for fname in ('block.dat', 'block_gn.dat', 'block_fg.dat', 'block_zs.dat'):
        download_block_raw_data(fname)

    # parse and generate CSV (use update_cache_block_file to match Go flow)
    update_cache_block_file()

def load_cache_block_infos() -> None:
    global _onceBlockFiles, _global_block_list, _map_block
    
    bk_filename = get_sector_filename()
    create_or_update = market_status.should_initialize_file(bk_filename)
    if create_or_update:
        try:
            sync_block_files()
        except Exception:
            logger.exception("sync_block_files error")
            pass

    try:
        df = pd.read_csv(bk_filename)
    except Exception:
        return

    _global_block_list = []
    _map_block = {}
    for _, row in df.iterrows():
        try:
            name = str(row.get('name') or '')
            code = str(market.detect_symbol(str(row.get('code') or '')))
            btype = int(row.get('type') or 0)
            block = str(row.get('block') or '')
            cs_field = row.get('constituent_stocks') or '[]'
            try:
                constituents = json.loads(cs_field) if isinstance(cs_field, str) else list(cs_field)
            except Exception:
                constituents = _parse_constituent_field(str(cs_field))
            constituents = [str(market.detect_symbol(s)) for s in constituents]
            bi = Sector(name=name, code=code, type=btype, count=len(constituents), block=block, constituent_stocks=constituents)
            _global_block_list.append(bi)
            _map_block[bi.code] = bi
        except Exception:
            continue


_onceBlockFiles = RollingOnce(name='sector', cron=tradinghours.cn_cron_expr_daily_init)

def get_sector_list() -> List[Sector]:
    _onceBlockFiles.do(load_cache_block_infos)
    return list(_global_block_list)


def get_sector_info(symbol: str) -> Optional[Sector]:
    _onceBlockFiles.do(load_cache_block_infos)
    inst = market.detect_symbol(symbol)
    return _map_block.get(inst.symbol())

if __name__ == '__main__':
    list = get_sector_list()
    print(list)
    _ = list