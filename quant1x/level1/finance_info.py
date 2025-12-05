# -*- coding: UTF-8 -*-

from __future__ import annotations

import struct
from dataclasses import dataclass

from quant1x.exchange import detect_market
from quant1x.level1 import protocol


@dataclass
class FinanceInfo:
    code: str = ''
    liu_tong_gu_ben: float = 0.0
    province: int = 0
    industry: int = 0
    updated_date: int = 0
    ipo_date: int = 0
    zong_gu_ben: float = 0.0
    guo_jia_gu: float = 0.0
    fa_qi_ren_fa_ren_gu: float = 0.0
    fa_ren_gu: float = 0.0
    b_gu: float = 0.0
    h_gu: float = 0.0
    zhi_gong_gu: float = 0.0
    zong_zi_chan: float = 0.0
    liu_dong_zi_chan: float = 0.0
    gu_ding_zi_chan: float = 0.0
    wu_xing_zi_chan: float = 0.0
    gu_dong_ren_shu: float = 0.0
    liu_dong_fu_zhai: float = 0.0
    chang_qi_fu_zhai: float = 0.0
    zi_ben_gong_ji_jin: float = 0.0
    jing_zi_chan: float = 0.0
    zhu_ying_shou_ru: float = 0.0
    zhu_ying_li_run: float = 0.0
    ying_shou_zhang_kuan: float = 0.0
    ying_ye_li_run: float = 0.0
    tou_zi_shou_yu: float = 0.0
    jing_ying_xian_jin_liu: float = 0.0
    zong_xian_jin_liu: float = 0.0
    cun_huo: float = 0.0
    li_run_zong_he: float = 0.0
    shui_hou_li_run: float = 0.0
    jing_li_run: float = 0.0
    wei_fen_li_run: float = 0.0
    mei_gu_jing_zi_chan: float = 0.0
    bao_liu2: float = 0.0

    def is_delisting(self) -> bool:
        return self.ipo_date == 0 and self.zong_gu_ben == 0 and self.liu_tong_gu_ben == 0


class FinanceRequest:
    def __init__(self, security_code: str):
        self.zip_flag = protocol.FLAG_UNCOMPRESSED
        self.seq_id = protocol.sequence_id()
        self.packet_type = 0x01
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.method = protocol.COMMAND_FINANCE_INFO

        market_id, _, symbol = detect_market(security_code)
        self.count = 1
        self.market = market_id
        self.code = symbol

    def serialize(self) -> bytes:
        # Body: Count(u16) + Market(u8) + Code(6s)
        # 2 + 1 + 6 = 9 bytes
        market_val = self.market.value if hasattr(self.market, 'value') else self.market
        body = struct.pack('<H B 6s', self.count, market_val, self.code.encode('utf-8'))

        self.pkg_len1 = 2 + len(body)
        self.pkg_len2 = self.pkg_len1

        header = struct.pack('<B I B H H H',
                             self.zip_flag, self.seq_id, self.packet_type,
                             self.pkg_len1, self.pkg_len2, self.method)
        return header + body


class FinanceResponse:
    def __init__(self):
        self.count = 0
        self.info = FinanceInfo()

    def deserialize(self, data: bytes) -> None:
        if len(data) < 2:
            return

        self.count = struct.unpack('<H', data[:2])[0]
        if self.count == 0:
            return

        # RawFinanceInfo struct size
        # B(1) + 6s(6) + f(4) + H(2) + H(2) + I(4) + I(4) + 30f(120) = 143 bytes
        offset = 2
        # struct format: < B 6s f H H I I 30f
        fmt = '< B 6s f H H I I ' + 'f' * 30
        struct_size = struct.calcsize(fmt)

        if len(data) < offset + struct_size:
            return

        unpacked = struct.unpack(fmt, data[offset:offset + struct_size])

        # Unpack fields
        raw_market = unpacked[0]
        raw_code = unpacked[1]
        raw_liu_tong_gu_ben = unpacked[2]
        raw_province = unpacked[3]
        raw_industry = unpacked[4]
        raw_updated_date = unpacked[5]
        raw_ipo_date = unpacked[6]
        # unpacked[7] starts the 30 floats
        raw_floats = unpacked[7:]

        base_unit = 10000.0

        self.info.code = raw_code.decode('utf-8').rstrip('\x00')
        if raw_market == 0:
            self.info.code = f"sz{self.info.code}"
        elif raw_market == 1:
            self.info.code = f"sh{self.info.code}"

        self.info.liu_tong_gu_ben = raw_liu_tong_gu_ben * base_unit
        self.info.province = raw_province
        self.info.industry = raw_industry
        self.info.updated_date = raw_updated_date
        self.info.ipo_date = raw_ipo_date

        # Map the rest of floats
        self.info.zong_gu_ben = raw_floats[0] * base_unit
        self.info.guo_jia_gu = raw_floats[1] * base_unit
        self.info.fa_qi_ren_fa_ren_gu = raw_floats[2] * base_unit
        self.info.fa_ren_gu = raw_floats[3] * base_unit
        self.info.b_gu = raw_floats[4] * base_unit
        self.info.h_gu = raw_floats[5] * base_unit
        self.info.zhi_gong_gu = raw_floats[6] * base_unit
        self.info.zong_zi_chan = raw_floats[7] * base_unit
        self.info.liu_dong_zi_chan = raw_floats[8] * base_unit
        self.info.gu_ding_zi_chan = raw_floats[9] * base_unit
        self.info.wu_xing_zi_chan = raw_floats[10] * base_unit
        self.info.gu_dong_ren_shu = raw_floats[11]  # No base_unit
        self.info.liu_dong_fu_zhai = raw_floats[12] * base_unit
        self.info.chang_qi_fu_zhai = raw_floats[13] * base_unit
        self.info.zi_ben_gong_ji_jin = raw_floats[14] * base_unit
        self.info.jing_zi_chan = raw_floats[15] * base_unit
        self.info.zhu_ying_shou_ru = raw_floats[16] * base_unit
        self.info.zhu_ying_li_run = raw_floats[17] * base_unit
        self.info.ying_shou_zhang_kuan = raw_floats[18] * base_unit
        self.info.ying_ye_li_run = raw_floats[19] * base_unit
        self.info.tou_zi_shou_yu = raw_floats[20] * base_unit
        self.info.jing_ying_xian_jin_liu = raw_floats[21] * base_unit
        self.info.zong_xian_jin_liu = raw_floats[22] * base_unit
        self.info.cun_huo = raw_floats[23] * base_unit
        self.info.li_run_zong_he = raw_floats[24] * base_unit
        self.info.shui_hou_li_run = raw_floats[25] * base_unit
        self.info.jing_li_run = raw_floats[26] * base_unit
        self.info.wei_fen_li_run = raw_floats[27] * base_unit
        self.info.mei_gu_jing_zi_chan = raw_floats[28] * base_unit  # BaoLiu1
        self.info.bao_liu2 = raw_floats[29]  # No base_unit
