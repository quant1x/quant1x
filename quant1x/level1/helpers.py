# -*- coding: UTF-8 -*-
from typing import Tuple
from quant1x.exchange import code as exchange_code

def varint_encode(value: int) -> bytes:
    """
    Encode an integer into varint bytes.
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
    Decode a varint from data starting at pos.
    Returns (value, new_pos).
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
    Get default base unit for price calculation.
    
    Args:
        market_id: Market ID (0=ShenZhen, 1=ShangHai, etc.)
        code: Security code
        
    Returns:
        Base unit (100.0 or 1000.0)
    """
    # market_id: 0=ShenZhen, 1=ShangHai
    # Using exchange_code.MarketType values if possible, but here we take int
    
    if (market_id == 1 and code.startswith('5')) or \
       (market_id == 0 and code.startswith('159')):
        return 1000.0
    return 100.0

def get_datetime_from_uint32(category: int, zipday: int, tminutes: int) -> Tuple[int, int, int, int, int]:
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

