# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import struct
from .command import (
    FLAG_UNCOMPRESSED,
    COMMAND_SECURITY_LIST,
)
from . import helpers
from quant1x.data.market import Exchange, Instrument, detect_instrument_type_by_rule
from quant1x.log import logger

SECURITY_LIST_PRE_REQUEST_MAX = 1600 # 预请求最大数量

class SecurityListRequest:
    def __init__(self, exchange: Exchange, start, count):
        self.market_id = helpers.exchange_to_market(exchange)
        self.start = start
        self.count = count
    
    def serialize(self):
        payload = struct.pack('<H I I I', int(self.market_id) & 0xFFFF, int(self.start) & 0xFFFFFFFF, int(self.count) & 0xFFFFFFFF, 0)
        zip_flag = FLAG_UNCOMPRESSED
        seq_id = helpers.msg_sequence_id()
        packet_type = 0x01
        pkg_len1 = 2 + len(payload)
        pkg_len2 = pkg_len1
        method = COMMAND_SECURITY_LIST
        header = struct.pack('<B I B H H H', zip_flag, seq_id, packet_type, pkg_len1, pkg_len2, method)
        return header + payload

class SecurityListResponse:
    def __init__(self, exchange: Exchange):
        self.exchange = exchange
        self.list: list[Instrument] = []
    
    def deserialize(self, data):
        if not data:
            # 响应体为空 -> 表示没有证券记录
            return

        # 解析：先读取 u16 的计数，然后依次解析记录
        offset = 0
        if len(data) < 2:
            return
        (cnt,) = struct.unpack_from('<H', data, offset)
        offset += 2
        # 每条记录至少为 25 字节（与 Rust 实现一致）
        for _ in range(cnt):
            if offset + 25 > len(data):
                logger.warning('Insufficient data when parsing SECURITY_LIST payload')
                break
            code_bytes = data[offset:offset+6]
            offset += 6
            (vol_unit,) = struct.unpack_from('<H', data, offset)
            offset += 2
            name_buf = data[offset:offset+16]
            offset += 16
            # 跳过 4 字节（保留字段）
            offset += 4
            (decimal_point,) = struct.unpack_from('<B', data, offset)
            offset += 1
            (tmp_u32,) = struct.unpack_from('<I', data, offset)
            offset += 4
            # 跳过最后 4 字节（保留/未使用）
            offset += 4

            # 解码代码和名称字段
            try:
                code = code_bytes.decode('ascii', errors='ignore').rstrip('\x00')
            except Exception:
                code = code_bytes.decode('utf-8', errors='ignore').rstrip('\x00')
            # 名称使用 GBK 编码，直到第一个 NUL 字节为止
            try:
                nul_pos = name_buf.index(0)
            except ValueError:
                nul_pos = len(name_buf)
            try:
                name = name_buf[:nul_pos].decode('gbk', errors='ignore')
            except Exception:
                name = name_buf[:nul_pos].decode('utf-8', errors='ignore')

            # 解码前收盘价
            pre_close = helpers.int_to_float64(tmp_u32)
            _ = pre_close # 避免未使用警告
            typ_ = detect_instrument_type_by_rule(self.exchange, code)
            inst = Instrument(exchange=self.exchange, type=typ_, ticker=code, name=name, lot_size=vol_unit, price_precision=decimal_point)
            self.list.append(inst)

        logger.debug('security_list fetched market={} start={} count={} parsed={}', self.exchange, 0, cnt, len(self.list))
