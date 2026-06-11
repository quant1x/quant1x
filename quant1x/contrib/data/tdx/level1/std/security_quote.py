# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct
from dataclasses import dataclass, field
from typing import List

from ...command import Command
from ... import helpers
from ... import protocol
from quant1x.data.meta.code import (
    detect_market,
    assert_index_by_market_and_code,
    MarketType,
    get_market_flag,
)


@dataclass
class StockInfo:
    market: int = 0
    code: str = ""


class SecurityQuote(protocol.BaseMessage):
    """行情快照"""
    def __init__(self, codes: List[str]):
        super().__init__(Command.STD_SECURITY_QUOTES_OLD)
        self._padding = bytes.fromhex("0500000000000000")
        self.list: List[StockInfo] = []

        for security_code in codes:
            sc = security_code.strip()
            if not sc:
                continue
            market_id, _, symbol = detect_market(sc)
            self.list.append(StockInfo(market=market_id.value, code=symbol))

        self.count = 0
        self.quotes: List[dict] = []

    def serialize_request_body(self) -> bytes:
        count = len(self.list)
        body = bytearray()
        body.extend(self._padding)
        body.extend(struct.pack('<H', count))
        for stock in self.list:
            body.append(stock.market)
            code_bytes = stock.code.encode('ascii')[:6].ljust(6, b'\x00')
            body.extend(code_bytes)
        return bytes(body)

    def deserialize_response_body(self, data: bytes) -> None:
        self.quotes.clear()
        pos = 2  # skip 2 bytes
        if len(data) < pos + 2:
            return

        self.count = struct.unpack('<H', data[pos:pos+2])[0]
        pos += 2

        for _ in range(self.count):
            if pos + 7 > len(data):
                break

            market = data[pos]
            pos += 1
            code = data[pos:pos+6].decode('ascii', errors='ignore').rstrip('\x00')
            pos += 6

            base_unit = helpers.default_base_unit(market, code)

            active1 = struct.unpack('<H', data[pos:pos+2])[0]
            pos += 2

            price_base, pos = helpers.varint_decode(data, pos)
            price = float(price_base) / base_unit

            tmp, pos = helpers.varint_decode(data, pos)
            last_close = float(price_base + tmp) / base_unit

            tmp, pos = helpers.varint_decode(data, pos)
            open_price = float(price_base + tmp) / base_unit

            tmp, pos = helpers.varint_decode(data, pos)
            high = float(price_base + tmp) / base_unit

            tmp, pos = helpers.varint_decode(data, pos)
            low = float(price_base + tmp) / base_unit

            reversed_bytes0, pos = helpers.varint_decode(data, pos)
            reversed_bytes1, pos = helpers.varint_decode(data, pos)

            vol, pos = helpers.varint_decode(data, pos)
            vol *= 100

            cur_vol, pos = helpers.varint_decode(data, pos)

            if pos + 4 > len(data):
                break
            raw_amount = struct.unpack('<I', data[pos:pos+4])[0]
            pos += 4
            amount = helpers.int_to_float64(raw_amount)

            s_vol, pos = helpers.varint_decode(data, pos)
            b_vol, pos = helpers.varint_decode(data, pos)

            val, pos = helpers.varint_decode(data, pos)
            index_open_amount = val * 100

            val, pos = helpers.varint_decode(data, pos)
            stock_open_amount = val * 100

            # Bid/Ask levels (5 levels)
            bids = []
            asks = []
            for _ in range(5):
                if pos + 4 > len(data):
                    break
                bid_p_diff, pos = helpers.varint_decode(data, pos)
                ask_p_diff, pos = helpers.varint_decode(data, pos)
                bid_vol, pos = helpers.varint_decode(data, pos)
                ask_vol, pos = helpers.varint_decode(data, pos)
                bids.append((float(bid_p_diff) / base_unit, bid_vol))
                asks.append((float(ask_p_diff) / base_unit, ask_vol))

            if pos + 2 > len(data):
                break
            reversed_bytes4 = struct.unpack('<H', data[pos:pos+2])[0]
            pos += 2

            reversed_bytes5, pos = helpers.varint_decode(data, pos)
            reversed_bytes6, pos = helpers.varint_decode(data, pos)
            reversed_bytes7, pos = helpers.varint_decode(data, pos)
            reversed_bytes8, pos = helpers.varint_decode(data, pos)

            if pos + 4 > len(data):
                break
            reversed_bytes9 = struct.unpack('<h', data[pos:pos+2])[0]
            pos += 2
            rate = float(reversed_bytes9) / 100.0

            active2 = struct.unpack('<H', data[pos:pos+2])[0]
            pos += 2

            self.quotes.append({
                'market': market, 'code': code,
                'price': price, 'last_close': last_close,
                'open': open_price, 'high': high, 'low': low,
                'vol': vol, 'cur_vol': cur_vol, 'amount': amount,
                's_vol': s_vol, 'b_vol': b_vol,
                'bids': bids, 'asks': asks,
                'rate': rate,
            })
