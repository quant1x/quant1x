# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations
from typing import Tuple
import threading
from quant1x.data.market import Exchange

_seq_lock = threading.Lock()
_seq_id = 0


def msg_sequence_id() -> int:
    """
    生成并返回一个全局唯一的序列ID
    
    每次调用时，序列ID会递增1，并保证在32位无符号整数范围内循环（0xFFFFFFFF）
    
    Returns:
        int: 32位无符号整数范围内的唯一序列ID
    """
    global _seq_id
    with _seq_lock:
        _seq_id = (_seq_id + 1) & 0xFFFFFFFF
        return _seq_id

def varint_encode(value: int) -> bytes:
    """
    将整数编码为 varint 字节序列。

    返回值为包含编码后字节的 `bytes`。
    """
    buffer = bytearray()
    sign = value < 0
    abs_value = abs(value)

    # Process first 6-bit block
    first_byte = abs_value & 0x3F
    abs_value >>= 6
    
    # Set sign bit (0x40) and continuation bit (0x80)
    if sign:
        first_byte |= 0x40
    if abs_value != 0:
        first_byte |= 0x80
        
    buffer.append(first_byte)

    # Process subsequent 7-bit blocks
    while abs_value != 0:
        byte = abs_value & 0x7F
        abs_value >>= 7
        if abs_value != 0:
            byte |= 0x80
        buffer.append(byte)

    return bytes(buffer)

def varint_decode(data: bytes, pos: int) -> Tuple[int, int]:
    """
    从 `data` 的位置 `pos` 解码一个 varint。

    返回 `(value, new_pos)`，其中 `new_pos` 是下一个未读取的索引位置。
    """
    if pos >= len(data):
        raise IndexError("Index out of range")
        
    byte = data[pos]
    pos += 1
    sign = (byte & 0x40) != 0
    value = byte & 0x3F
    shift = 6
    
    while byte & 0x80:
        if pos >= len(data):
            raise IndexError("Index out of range")
        byte = data[pos]
        pos += 1
        value |= (byte & 0x7F) << shift
        shift += 7
        
    if sign:
        value = -value
    return value, pos

def default_base_unit(market_id: int, code: str) -> float:
    """
    获取价格计算所用的默认基数（单位）。

    参数：
        market_id: 市场编号（例如 0=深市，1=沪市）
        code: 证券代码

    返回：基数（`100.0` 或 `1000.0`）。
    """
    # market_id: 0=ShenZhen, 1=ShangHai
    # Using exchange_code.MarketType values if possible, but here we take int
    
    if (market_id == 1 and code.startswith('5')) or \
       (market_id == 0 and code.startswith('159')):
        return 1000.0
    return 100.0

def get_datetime_from_uint32(category: int, zipday: int, tminutes: int) -> Tuple[int, int, int, int, int]:
    """
    根据不同的 `category` 格式，从压缩日期/分钟信息中恢复年、月、日、时、分。

    返回 `(year, month, day, hour, minute)`。
    """
    year = 0
    month = 0
    day = 0
    hour = 15
    minute = 0

    if category < 4 or category == 7 or category == 8:
        year = (zipday >> 11) + 2004
        month = int((zipday % 2048) / 100)
        day = int((zipday % 2048) % 100)
        hour = int(tminutes / 60)
        minute = int(tminutes % 60)
    else:
        year = int(zipday / 10000)
        month = int((zipday % 10000) / 100)
        day = int(zipday % 100)

    return year, month, day, hour, minute

def int_to_float64(integer: int) -> float:
    """
    将 32 位无符号整数解释并转换为浮点数（与 level1 协议中使用的转换一致）。

    该函数把输入分解成四个字节并依照协议的位权与指数规则重建浮点值。
    """
    # Ensure input is treated as 32-bit unsigned integer
    uinteger = integer & 0xFFFFFFFF

    # Decompose into 4 bytes
    log_point = (uinteger >> 24) & 0xFF
    hleax = (uinteger >> 16) & 0xFF
    lheax = (uinteger >> 8) & 0xFF
    lleax = uinteger & 0xFF

    # Calculate exponents
    dw_ecx = log_point * 2 - 0x7F
    dw_edx = log_point * 2 - 0x86
    dw_esi = log_point * 2 - 0x8E
    dw_eax = log_point * 2 - 0x96

    # Calculate dblXmm6
    dbl_xmm6 = 0.0
    tmp_eax = abs(dw_ecx)
    dbl_xmm6 = pow(2.0, tmp_eax)
    if dw_ecx < 0:
        dbl_xmm6 = 1.0 / dbl_xmm6

    # Calculate dblXmm4
    dbl_xmm4 = 0.0
    if hleax > 0x80:
        dw_tmpeax = dw_edx + 1
        tmp_dbl_xmm3 = pow(2.0, dw_tmpeax)
        dbl_xmm0 = pow(2.0, dw_edx) * 128.0
        dbl_xmm0 += (hleax & 0x7F) * tmp_dbl_xmm3
        dbl_xmm4 = dbl_xmm0
    else:
        if dw_edx >= 0:
            dbl_xmm4 = pow(2.0, dw_edx) * hleax
        else:
            dbl_xmm4 = (1.0 / pow(2.0, -dw_edx)) * hleax

    # Calculate dblXmm3 and dblXmm1
    dbl_xmm3 = pow(2.0, dw_esi) * lheax
    dbl_xmm1 = pow(2.0, dw_eax) * lleax

    # If hleax highest bit is 1, multiply by 2
    if (hleax & 0x80) != 0:
        dbl_xmm3 *= 2.0
        dbl_xmm1 *= 2.0

    return dbl_xmm6 + dbl_xmm4 + dbl_xmm3 + dbl_xmm1


_EXCHANGE_TO_MARKET = {
    Exchange.SSE: 1,
    Exchange.SZSE: 0,
    Exchange.BSE: 2,
}

_MARKET_TO_EXCHANGE = {v: k for k, v in _EXCHANGE_TO_MARKET.items()}

def exchange_to_market(ex: Exchange) -> int:
    if ex not in _EXCHANGE_TO_MARKET:
        raise ValueError(f"Unsupported exchange: {ex}")
    return _EXCHANGE_TO_MARKET[ex]

def market_to_exchange(market_id: int) -> Exchange:
    if market_id not in _MARKET_TO_EXCHANGE:
        raise ValueError(f"Unsupported market id: {market_id}")
    return _MARKET_TO_EXCHANGE[market_id]