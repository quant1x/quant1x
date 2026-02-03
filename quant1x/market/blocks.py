# -*- coding: UTF-8 -*-
"""
Python port of market/blocks.go (partial):
- Exposes BlockInfo dataclass
- Implements BlockList() and GetBlockInfo() using existing data.sector helpers

This implementation focuses on the high-level APIs used elsewhere in the
codebase and defers low-level TDX binary parsing / network downloads to
existing modules (or future work).
"""
from __future__ import annotations

import json
from dataclasses import dataclass, field
from typing import List, Optional

from .. import config
from ..exchange.code import correct_security_code
import os

from quant1x.level1.client import get_std_conn
from quant1x.level1.block_info import BlockInfoRequest, BlockInfoResponse, BLOCK_CHUNKS_SIZE
from quant1x.level1 import protocol as l1protocol
import struct
import pandas as pd
from .. import exchange
from ..exchange import last_trade_date
from ..runtime.once import RollingOnce
from ..exchange.timestamp import PRE_MARKET_HOUR, PRE_MARKET_MINUTE
import threading
from ..exchange import timestamp

from ..exchange import status as market_status


@dataclass
class BlockInfo:
    Name: str = ""
    Code: str = ""
    Type: int = 0
    Count: int = 0
    Block: str = ""
    ConstituentStocks: List[str] = field(default_factory=list)


def get_sector_filename(date: str = "") -> str:
    name = "blocks"
    current_date = timestamp.Timestamp.now()
    if len(date) > 0:
        current_date = timestamp.Timestamp.parse(date)
    cache_date = exchange.last_trading_day(current_date).only_date()
    # place generated sector file in the configured meta path
    filename = f'{name}.{cache_date}'
    return os.path.join(config.meta_path, filename)


def _parse_constituent_field(val: str) -> List[str]:
    if not val:
        return []
    val = val.strip()
    try:
        lst = json.loads(val)
        if isinstance(lst, list):
            return [correct_security_code(s.strip()) for s in lst if s]
    except Exception:
        # fallback: comma or pipe separated list
        sep = ","
        if "|" in val:
            sep = "|"
        parts = [p.strip() for p in val.split(sep) if p.strip()]
        return [correct_security_code(p) for p in parts]
    return []


__onceBlockFiles = RollingOnce.daily(PRE_MARKET_HOUR, PRE_MARKET_MINUTE)


def load_cache_block_infos() -> None:
    """Load/refresh cached block infos (mirror of Go loadCacheBlockInfos).

    This function downloads/parses if needed, then reads the generated
    CSV and normalizes codes using `correct_security_code`.
    """
    global __onceBlockFiles, _global_block_list, _map_block
    try:
        sync_block_files()
    except Exception:
        # continue to trying to load existing cache
        pass

    bk_filename = get_sector_filename()
    try:
        df = pd.read_csv(bk_filename)
    except Exception:
        return

    _global_block_list = []
    _map_block = {}
    for _, row in df.iterrows():
        try:
            name = str(row.get('name') or '')
            code = correct_security_code(str(row.get('code') or ''))
            btype = int(row.get('type') or 0)
            block = str(row.get('block') or '')
            cs_field = row.get('constituent_stocks') or '[]'
            try:
                constituents = json.loads(cs_field) if isinstance(cs_field, str) else list(cs_field)
            except Exception:
                constituents = _parse_constituent_field(str(cs_field))
            constituents = [correct_security_code(s) for s in constituents]
            bi = BlockInfo(Name=name, Code=code, Type=btype, Count=len(constituents), Block=block, ConstituentStocks=constituents)
            _global_block_list.append(bi)
            _map_block[bi.Code] = bi
        except Exception:
            continue


def get_sector_list() -> List[BlockInfo]:
    """Return cached list of BlockInfo. Uses a RollingOnce to load cache once per window."""
    __onceBlockFiles.do(load_cache_block_infos)
    # return a shallow copy to avoid accidental mutation
    return list(_global_block_list)


def get_sector_info(code: str) -> Optional[BlockInfo]:
    """Return BlockInfo for given block code or None if not found."""
    __onceBlockFiles.do(load_cache_block_infos)
    code_fixed = correct_security_code(code)
    return _map_block.get(code_fixed)


# cache + initialization helpers (mirror Go semantics)
_block_init_lock = threading.Lock()
_block_inited = False
_global_block_list: List[BlockInfo] = []
_map_block: dict = {}


def _ensure_block_cache(force: bool = False):
    """Ensure the block CSV is available and load it into module cache.

    If the CSV is missing or `market_status.should_initialize_file` returns
    True for its mtime, `sync_block_files()` will be invoked to update it.
    """
    global _block_inited, _global_block_list, _map_block
    if _block_inited and not force:
        return
    with _block_init_lock:
        if _block_inited and not force:
            return

        sfn = get_sector_filename()
        try_initialize = False
        try:
            if not os.path.isfile(sfn):
                try_initialize = True
            else:
                if market_status.should_initialize_file(sfn):
                    try_initialize = True
        except Exception:
            try_initialize = True

        if try_initialize:
            try:
                sync_block_files()
            except Exception:
                pass

        # Load CSV into memory
        _global_block_list = []
        _map_block = {}
        try:
            df = pd.read_csv(sfn)
            for _, row in df.iterrows():
                name = str(row.get('name') or '')
                code = str(row.get('code') or '')
                btype = int(row.get('type') or 0)
                block = str(row.get('block') or '')
                cs_field = row.get('constituent_stocks') or '[]'
                try:
                    constituents = json.loads(cs_field) if isinstance(cs_field, str) else list(cs_field)
                except Exception:
                    constituents = _parse_constituent_field(str(cs_field))
                constituents = [correct_security_code(s) for s in constituents]
                bi = BlockInfo(Name=name, Code=correct_security_code(code), Type=btype, Count=len(constituents), Block=block, ConstituentStocks=constituents)
                _global_block_list.append(bi)
                _map_block[bi.Code] = bi
        except Exception:
            _global_block_list = []
            _map_block = {}

        _block_inited = True


def _get_block_info_from_level1(filename: str) -> Optional[bytes]:
    """Download raw block file via level1 protocol and return bytes or None."""
    try:
        with get_std_conn() as conn:
            start = 0
            result = bytearray()
            while True:
                req = BlockInfoRequest(filename, start)
                resp = BlockInfoResponse()
                l1protocol.process(conn, req, resp)
                if resp.size == 0:
                    return None
                if resp.size > 0:
                    result.extend(resp.data)
                if resp.size < BLOCK_CHUNKS_SIZE:
                    break
                start += resp.size
            return bytes(result)
    except Exception:
        return None


def download_block_raw_data(filename: str) -> Optional[str]:
    """Download a block raw file via level1 and save under `config.meta_path`.

    Returns the saved filepath or None on failure.
    """
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
    """Mirror Go's updateCacheBlockFile: create or update the generated sector CSV when needed."""
    block_file = get_sector_filename()
    print(f"update_cache_block_file: block_file={block_file}")
    create_or_update = False
    if not os.path.isfile(block_file):
        create_or_update = True
    else:
        try:
            if market_status.should_initialize_file(block_file):
                create_or_update = True
        except Exception:
            create_or_update = True

    if create_or_update:
        parse_and_generate_block_file()


def parse_raw_block_file(block_filename: str) -> List[dict]:
    """Parse a TDX raw block file saved under `config.meta_path`.

    Returns list of dicts: {"block_name","num","block_type","codes"}
    """
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


def parse_and_generate_block_file():
    """Parse standard block files and generate `blocks.<date>` CSV in meta path.

    Implements the same algorithm as Go:
    1. Load block index config entries from `tdxzs.cfg`/`tdxzs3.cfg`.
    2. Parse raw block files (`block.dat`, `block_gn.dat`, `block_fg.dat`, `block_zs.dat`)
       and build a mapping from block name -> raw info.
    3. For each block index entry, prefer the raw info match by name; otherwise
       fall back to industry mapping from `tdxhy.cfg`.
    4. Write CSV with fields: name, code, type, count, block, constituent_stocks
    """
    # helper: read block index config files
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

    # 1) load index block infos
    bks_cfg = ['tdxzs.cfg', 'tdxzs3.cfg']
    block_index = []
    for cfg in bks_cfg:
        block_index.extend(get_block_info_from_config(cfg))

    if not block_index:
        return None

    # block -> name mapping
    block2name = {v['block']: v['name'] for v in block_index if v.get('block')}

    # 2) parse raw block files and build name->rawinfo
    raw_files = ["block.dat", "block_gn.dat", "block_fg.dat", "block_zs.dat"]
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
    from quant1x.exchange.code import get_market
    for v in block_index:
        bn = v['name']
        entry_codes = []
        if bn in name2block:
            info = name2block[bn]
            for sc in info.get('codes', []):
                if len(sc) < 5:
                    continue
                market = get_market(sc)
                if market == 'bj':
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
    """Download required block files, unzip, parse and generate CSV cache."""
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
