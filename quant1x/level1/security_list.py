"""从 level1 服务器获取 SECURITY_LIST 页面（Python 实现）。

本模块提供一个小工具函数 `fetch_security_list(market, start, count)`，
其行为与 native（C++/Rust）实现保持一致：构造 SECURITY_LIST 请求，
通过连接池发送请求，并将响应体解析为字典列表返回。

注意：实现偏稳健与保守：遇到任意 I/O 或解析错误时返回 `None`，由调用方决定如何处理。
"""
from __future__ import annotations

import struct
import logging
from typing import Optional, List, Dict

from quant1x.level1 import client as l1client
from quant1x.level1 import protocol

log = logging.getLogger(__name__)

# 保持与 C++/Rust 实现类似的默认分页大小。C++ 头文件使用
# `security_list_pre_request_max`（通常为 1600），这里也使用 1600 作为默认值。
PRE_REQUEST_MAX = 1600


def _int_to_float64(v: int) -> float:
    # 移植自 Rust 中 level1/helpers.rs 的 `int_to_float64` 实现
    if v == 0:
        return 0.0
    log_point = ((v >> 24) & 0xFF)
    hleax = ((v >> 16) & 0xFF)
    lheax = ((v >> 8) & 0xFF)
    lleax = (v & 0xFF)

    dw_ecx = log_point * 2 - 0x7F
    dw_edx = log_point * 2 - 0x86
    dw_esi = log_point * 2 - 0x8E
    dw_eax = log_point * 2 - 0x96

    def pow2(i: int) -> float:
        return 2.0 ** i

    tmp_eax = -dw_ecx if dw_ecx < 0 else dw_ecx
    dbl_xmm6 = pow2(tmp_eax)
    if dw_ecx < 0:
        dbl_xmm6 = 1.0 / dbl_xmm6

    if hleax > 0x80:
        dwtmpeax = dw_edx + 1
        tmpdbl_xmm3 = pow2(dwtmpeax)
        dbl_xmm0 = pow2(dw_edx) * 128.0
        dbl_xmm0 += (hleax & 0x7F) * tmpdbl_xmm3
        dbl_xmm4 = dbl_xmm0
    elif dw_edx >= 0:
        dbl_xmm4 = pow2(dw_edx) * float(hleax)
    else:
        dbl_xmm4 = (1.0 / pow2(-dw_edx)) * float(hleax)

    dbl_xmm3 = pow2(dw_esi) * float(lheax)
    dbl_xmm1 = pow2(dw_eax) * float(lleax)

    if (hleax & 0x80) != 0:
        dbl_xmm3 *= 2.0
        dbl_xmm1 *= 2.0

    return dbl_xmm6 + dbl_xmm4 + dbl_xmm3 + dbl_xmm1


def fetch_security_list(market: int, start: int, count: int) -> Optional[List[Dict]]:
    """从 level1 服务器获取一页 SECURITY_LIST。

    返回一个字典列表，字典包含字段：`Code`（6 字符字符串）、`VolUnit`（整数）、
    `DecimalPoint`（整数）、`Name`（字符串）、`PreClose`（浮点）。出现错误时返回 `None`。
    """
    try:
        class SecurityListRequest:
            def __init__(self, market, start, count):
                self.market = market
                self.start = start
                self.count = count
            
            def serialize(self):
                payload = struct.pack('<H I I I', int(self.market) & 0xFFFF, int(self.start) & 0xFFFFFFFF, int(self.count) & 0xFFFFFFFF, 0)
                zip_flag = 0x0C
                seq_id = protocol.sequence_id()
                packet_type = 0x01
                pkg_len1 = 2 + len(payload)
                pkg_len2 = pkg_len1
                method = 0x044d
                header = struct.pack('<B I B H H H', zip_flag, seq_id, packet_type, pkg_len1, pkg_len2, method)
                return header + payload

        class SecurityListResponse:
            def __init__(self):
                self.body = b''
            
            def deserialize(self, data):
                self.body = data

        req = SecurityListRequest(market, start, count)
        resp = SecurityListResponse()

        with l1client.get_std_conn() as conn:
            protocol.process(conn, req, resp)
        
        body = resp.body

        if not body:
            # 响应体为空 -> 表示没有证券记录
            return []

        # 解析：先读取 u16 的计数，然后依次解析记录
        offset = 0
        if len(body) < 2:
            return []
        (cnt,) = struct.unpack_from('<H', body, offset)
        offset += 2
        result = []
        # 每条记录至少为 25 字节（与 Rust 实现一致）
        for _ in range(cnt):
            if offset + 25 > len(body):
                log.warning('Insufficient data when parsing SECURITY_LIST payload')
                break
            code_bytes = body[offset:offset+6]
            offset += 6
            (vol_unit,) = struct.unpack_from('<H', body, offset)
            offset += 2
            name_buf = body[offset:offset+16]
            offset += 16
            # 跳过 4 字节（保留字段）
            offset += 4
            (decimal_point,) = struct.unpack_from('<B', body, offset)
            offset += 1
            (tmp_u32,) = struct.unpack_from('<I', body, offset)
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

            pre_close = _int_to_float64(tmp_u32)

            result.append({
                'Code': code,
                'VolUnit': int(vol_unit),
                'DecimalPoint': int(decimal_point),
                'Name': name,
                'PreClose': pre_close,
            })

        log.info('security_list fetched market=%s start=%s count=%s parsed=%s', market, start, count, len(result))
        return result
    except Exception as e:
        log.exception('fetch_security_list failed: %s', e)
        return None


if __name__ == '__main__':
    list = fetch_security_list(1, 0, 10)
    print(list)