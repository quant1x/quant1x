# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct

from quant1x import data
from quant1x.data.meta import Exchange, Instrument
from quant1x.data.schema import Transaction as TransactionRecord

from quant1x.contrib.data.tdx import helpers, protocol
from quant1x.contrib.data.tdx.command import Command

# Constants for transaction direction
TICK_BUY = 0
TICK_SELL = 1
TICK_NEUTRAL = 2
TICK_UNKNOWN = 3

TICK_TRANSACTION_PER_REQUEST_MAX = 1800


class TransactionContext(protocol.BaseFrame):
    """分笔成交数据"""
    def __init__(self, inst: Instrument, start: int, count: int,
                 price_precision: int = 2, is_index: bool = False):
        super().__init__(Command.STD_TRANSACTION_DATA)
        self.request_header.packet_ctrl = 0x00
        self._market = helpers.exchange_to_market(inst.exchange)
        self._ticker = inst.market_ticker()
        self._start = start
        self._count = count
        self._price_precision = price_precision
        self._is_index = is_index

        self.count = 0
        self.list: list[TransactionRecord] = []

    def serialize_request_body(self) -> bytes:
        code_bytes = self._ticker.encode('ascii')
        if len(code_bytes) < 6:
            code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
        else:
            code_bytes = code_bytes[:6]
        return struct.pack('<H 6s H H', self._market, code_bytes, self._start, self._count)

    def deserialize_response_body(self, data: bytes) -> None:
        self.list.clear()
        if len(data) < 2:
            return
        self.count = struct.unpack('<H', data[:2])[0]
        pos = 2
        base_unit = 10 ** self._price_precision
        last_price = 0
        for _ in range(self.count):
            if pos >= len(data):
                break
            if pos + 2 > len(data):
                break
            minutes = struct.unpack('<H', data[pos:pos+2])[0]
            pos += 2
            h = minutes // 60
            m = minutes % 60
            time_str = f"{h:02d}:{m:02d}"
            raw_price, pos = helpers.varint_decode(data, pos)
            vol, pos = helpers.varint_decode(data, pos)
            num, pos = helpers.varint_decode(data, pos)
            buy_or_sell, pos = helpers.varint_decode(data, pos)
            last_price += raw_price
            price = float(last_price) / base_unit
            amount = 0.0
            if self._is_index:
                amount = float(vol * 100)
                vol = int(amount / price) if price > 0 else 0
            else:
                vol *= 100
                amount = float(vol) * price
            _, pos = helpers.varint_decode(data, pos)
            self.list.append(TransactionRecord(time=time_str, price=price, volume=vol, num=num, amount=amount, direction=buy_or_sell))


class HistoricalTransactionContext(protocol.BaseFrame):
    """历史分笔成交数据"""
    def __init__(self, inst: Instrument, date: int, start: int, count: int,
                 price_precision: int = 2, is_index: bool = False):
        super().__init__(Command.STD_HISTORY_TRANSACTION_DATA)
        self.request_header.packet_ctrl = 0x00
        self._market = helpers.exchange_to_market(inst.exchange)
        self._ticker = inst.market_ticker()
        self._date = date
        self._start = start
        self._count = count
        self._price_precision = price_precision
        self._is_index = is_index

        self.count = 0
        self.list: list[TransactionRecord] = []

    def serialize_request_body(self) -> bytes:
        code_bytes = self._ticker.encode('ascii')
        if len(code_bytes) < 6:
            code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
        else:
            code_bytes = code_bytes[:6]
        return struct.pack('<I H 6s H H', self._date, self._market, code_bytes, self._start, self._count)

    def deserialize_response_body(self, data: bytes) -> None:
        self.list.clear()
        if len(data) < 2:
            return
        self.count = struct.unpack('<H', data[:2])[0]
        pos = 2
        pos += 4  # C++: bs.skip(4); 历史分笔成交记录, 跳过4个字节
        base_unit = 10 ** self._price_precision
        last_price = 0
        for _ in range(self.count):
            if pos >= len(data):
                break
            if pos + 2 > len(data):
                break
            minutes = struct.unpack('<H', data[pos:pos+2])[0]
            pos += 2
            h = minutes // 60
            m = minutes % 60
            time_str = f"{h:02d}:{m:02d}"
            raw_price, pos = helpers.varint_decode(data, pos)
            vol, pos = helpers.varint_decode(data, pos)
            buy_or_sell, pos = helpers.varint_decode(data, pos)
            last_price += raw_price
            price = float(last_price) / base_unit
            amount = 0.0
            if self._is_index:
                amount = float(vol * 100)
                vol = int(amount / price) if price > 0 else 0
            else:
                vol *= 100
                amount = float(vol) * price
            _, pos = helpers.varint_decode(data, pos)
            self.list.append(TransactionRecord(time=time_str, price=price, volume=vol, num=0, amount=amount, direction=buy_or_sell))

if __name__ == '__main__':
    import pandas as pd
    from quant1x.contrib.data.tdx.client import get_std_conn
    conn = get_std_conn()
    
    # # 测试 单个xdxr
    # req = Xdxr(exchange=Exchange.SZSE, ticker='000001')
    # protocol.transact_message_sync(conn, req)
    # if req.list:
    #     print(f"xdxr: count={req.count}")
    #     df = pd.DataFrame(req.list)
    #     print(df)
    
    # 测试 批量transaction details
    inst = data.detect_symbol("000001.sh")
    req = TransactionContext(inst=inst, is_index=True, start=4746, count=3)
    protocol.transact_message_sync(conn, req)
    if req.list:
        print(f"transaction details: count={req.count}")
        df = pd.DataFrame(req.list)
        print(df)
    