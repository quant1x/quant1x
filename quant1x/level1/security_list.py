"""Fetch SECURITY_LIST pages from level1 servers (Python port).

This module provides a small helper `fetch_security_list(market, start, count)`
which mirrors the Rust/C++ behavior used by the native `exchange` code. It
builds a SECURITY_LIST request, sends it over a pooled level1 connection and
parses the response payload into a list of dicts.

Note: This implementation aims to be robust but conservative: on any I/O or
parse error it returns None so callers can decide how to proceed.
"""
from __future__ import annotations

import struct
import logging
from typing import Optional, List, Dict

from quant1x.level1 import client as l1client
from quant1x.level1 import protocol

log = logging.getLogger(__name__)

# Keep a default page size similar to C++/Rust code. The C++ header uses
# `security_list_pre_request_max` (commonly 1600). Use 1600 as default.
PRE_REQUEST_MAX = 1600


def _int_to_float64(v: int) -> float:
    # Port of Rust `int_to_float64` from level1/helpers.rs
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
    """Fetch one page of SECURITY_LIST from a level1 server.

    Returns a list of dicts with keys: Code (6-char string), VolUnit (int),
    DecimalPoint (int), Name (str), PreClose (float). Returns None on error.
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

        with l1client.client() as conn:
            protocol.process(conn.socket, req, resp)
        
        body = resp.body

        if not body:
            # empty body -> no securities
            return []

        # parse: first u16 count, then records
        offset = 0
        if len(body) < 2:
            return []
        (cnt,) = struct.unpack_from('<H', body, offset)
        offset += 2
        result = []
        # each record expected to be 25 bytes minimum as in Rust implementation
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
            # skip 4 bytes
            offset += 4
            (decimal_point,) = struct.unpack_from('<B', body, offset)
            offset += 1
            (tmp_u32,) = struct.unpack_from('<I', body, offset)
            offset += 4
            # skip last 4 bytes
            offset += 4

            # decode code and name
            try:
                code = code_bytes.decode('ascii', errors='ignore').rstrip('\x00')
            except Exception:
                code = code_bytes.decode('utf-8', errors='ignore').rstrip('\x00')
            # name is GBK encoded up to first NUL
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