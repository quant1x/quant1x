# -*- coding: UTF-8 -*-
import struct
from enum import Enum
from typing import List, Tuple, Dict, Any
from dataclasses import dataclass

from quant1x.level1 import protocol, helpers
from quant1x.exchange import code as exchange_code

class XdxrCategory(Enum):
    EX_DIVIDEND = 1                       # 除权除息
    BONUS_SHARES_LISTING = 2              # 送股上市（无偿）
    RESTRICTED_SHARES_LISTING = 3         # 非流通股上市（受限股解禁）
    UNSPECIFIED_CAPITAL_ADJUSTMENT = 4    # 未知股本变动
    GENERAL_CAPITAL_ADJUSTMENT = 5        # 股本变化（保留，但慎用）
    NEW_SHARE_ISSUANCE = 6                # 增发新股
    SHARE_REPURCHASE = 7                  # 股份回购
    NEW_SHARES_LISTING = 8                # 增发新股上市
    TRANSFERRED_RIGHTS_SHARES_LISTING = 9 # 转配股上市（中国特有）
    CONVERTIBLE_BOND_LISTING = 10         # 可转债上市
    STOCK_SPLIT_OR_REVERSE_SPLIT = 11     # 拆股或合股
    RESTRICTED_SHARES_CONSOLIDATION = 12  # 非流通股缩股
    ISSUE_CALL_WARRANTS = 13              # 送认购权证
    ISSUE_PUT_WARRANTS = 14               # 送认沽权证

    @staticmethod
    def to_string(category: int) -> str:
        try:
            return {
                1: "除权除息",
                2: "送配股上市",
                3: "非流通股上市",
                4: "未知股本变动",
                5: "股本变化",
                6: "增发新股",
                7: "股份回购",
                8: "增发新股上市",
                9: "转配股上市",
                10: "可转债上市",
                11: "扩缩股",
                12: "非流通股缩股",
                13: "送认购权证",
                14: "送认沽权证"
            }.get(category, f"Unknown({category})")
        except:
            return f"Unknown({category})"

@dataclass
class XdxrInfo:
    Date: str = ""           # 日期 YYYY-MM-DD格式
    Category: int = 0        # 类型编号
    Name: str = ""           # 类型名称
    FenHong: float = 0.0     # 分红(元)
    PeiGuJia: float = 0.0    # 配股价(元)
    SongZhuanGu: float = 0.0 # 送转股(股)
    PeiGu: float = 0.0       # 配股(股)
    SuoGu: float = 0.0       # 缩股(股)
    QianLiuTong: float = 0.0 # 除权前流通股(万股)
    HouLiuTong: float = 0.0  # 除权后流通股(万股)
    QianZongGuBen: float = 0.0 # 除权前总股本(万股)
    HouZongGuBen: float = 0.0  # 除权后总股本(万股)
    FenShu: float = 0.0      # 权证份数
    XingQuanJia: float = 0.0 # 行权价格(元)

    def is_adjust(self) -> bool:
        count = self.FenHong
        count += self.PeiGu
        count += self.SongZhuanGu
        count += self.SuoGu
        count += self.FenShu
        return count > 0.00

    def adjust_factor(self) -> Tuple[float, float]:
        m = 0.0
        a = 0.0
        
        A = self.compute_monetary_adjustment()
        B = self.compute_share_adjustment_ratio()
        
        if abs(1.0 + B) > 1e-10:
            m = 1.0 / (1.0 + B)
            a = A * m
        else:
            m = 1.0
            a = 0.0
            
        return m, a

    def compute_monetary_adjustment(self) -> float:
        return (self.PeiGu * self.PeiGuJia - self.FenHong + self.FenShu * self.XingQuanJia) / 10.0

    def compute_share_adjustment_ratio(self) -> float:
        return (self.SongZhuanGu + self.PeiGu - self.SuoGu + self.FenShu) / 10.0

    def is_capital_change(self) -> bool:
        if self.Category in [
            XdxrCategory.EX_DIVIDEND.value,
            XdxrCategory.STOCK_SPLIT_OR_REVERSE_SPLIT.value,
            XdxrCategory.RESTRICTED_SHARES_CONSOLIDATION.value,
            XdxrCategory.ISSUE_CALL_WARRANTS.value,
            XdxrCategory.ISSUE_PUT_WARRANTS.value
        ]:
            return False
        
        if self.HouLiuTong > 0 and self.HouZongGuBen > 0:
            return True
        return False

    def adjust(self):
        """
        生成复权计算函数
        返回: 计算复权价格的函数对象 (callable)
        """
        song_zhuangu = self.SongZhuanGu
        pei_gu = self.PeiGu
        suo_gu = self.SuoGu
        xdxr_gu_shu = (song_zhuangu + pei_gu - suo_gu) / 10.0
        
        fen_hong = self.FenHong
        pei_gu_jia = self.PeiGuJia
        xdxr_fen_hong = (pei_gu_jia * pei_gu - fen_hong) / 10.0

        def calculator(p: float) -> float:
            return (p + xdxr_fen_hong) / (1 + xdxr_gu_shu)
            
        return calculator

class XdxrInfoRequest:
    def __init__(self, security_code: str):
        self.zip_flag = protocol.FLAG_UNCOMPRESSED
        self.seq_id = protocol.sequence_id()
        self.packet_type = 0x01
        self.method = protocol.COMMAND_XDXR_INFO
        
        market_id, _, symbol = exchange_code.detect_market(security_code)
        self.market = market_id.value if hasattr(market_id, 'value') else market_id
        self.code = symbol
        self.padding = bytes.fromhex('0100')

    def serialize(self) -> bytes:
        # Body: padding(2) + Market(1) + Code(6) = 9 bytes
        # PkgLen = BodyLen + 2 = 11
        body_len = 2 + 1 + 6
        pkg_len = body_len + 2
        
        header = struct.pack('<B I B H H H', 
                             self.zip_flag, self.seq_id, self.packet_type, 
                             pkg_len, pkg_len, self.method)
        
        code_bytes = self.code.encode('ascii')
        if len(code_bytes) < 6:
            code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
        else:
            code_bytes = code_bytes[:6]
            
        body = struct.pack('<2s B 6s', self.padding, self.market, code_bytes)
        return header + body

class XdxrInfoResponse:
    def __init__(self):
        self.count = 0
        self.list: List[XdxrInfo] = []

    def deserialize(self, data: bytes):
        if len(data) < 9:
            return
            
        pos = 9
        if pos + 2 > len(data):
            return
            
        self.count = struct.unpack('<H', data[pos:pos+2])[0]
        pos += 2
        
        for _ in range(self.count):
            if pos + 29 > len(data): # 1+6+1+4+1+16 = 29 bytes per record
                break
                
            # Market(1), Code(6), Unknown(1), Date(4), Category(1), Data(16)
            pos += 1 # Market
            pos += 6 # Code
            pos += 1 # Unknown
            
            date_int = struct.unpack('<I', data[pos:pos+4])[0]
            pos += 4
            
            category = struct.unpack('<B', data[pos:pos+1])[0]
            pos += 1
            
            record_data = data[pos:pos+16]
            pos += 16
            
            year, month, day, _, _ = helpers.get_datetime_from_uint32(9, date_int, 0)
            
            info = XdxrInfo()
            info.Category = category
            info.Date = f"{year:04d}-{month:02d}-{day:02d}"
            info.Name = XdxrCategory.to_string(category)
            
            if category == 1: # 除权除息
                info.FenHong = struct.unpack('<f', record_data[0:4])[0]
                info.PeiGuJia = struct.unpack('<f', record_data[4:8])[0]
                info.SongZhuanGu = struct.unpack('<f', record_data[8:12])[0]
                info.PeiGu = struct.unpack('<f', record_data[12:16])[0]
            elif category in [11, 12]:
                # Skip 8 bytes
                info.SuoGu = struct.unpack('<f', record_data[8:12])[0]
            elif category in [13, 14]:
                info.XingQuanJia = struct.unpack('<f', record_data[0:4])[0]
                # Skip 8 bytes (4-12)
                info.FenShu = struct.unpack('<f', record_data[12:16])[0]
            else:
                v1 = struct.unpack('<I', record_data[0:4])[0]
                info.QianLiuTong = self._get_v(v1)
                v2 = struct.unpack('<I', record_data[4:8])[0]
                info.QianZongGuBen = self._get_v(v2)
                v3 = struct.unpack('<I', record_data[8:12])[0]
                info.HouLiuTong = self._get_v(v3)
                v4 = struct.unpack('<I', record_data[12:16])[0]
                info.HouZongGuBen = self._get_v(v4)
                
            self.list.append(info)

    def _get_v(self, v: int) -> float:
        if v == 0:
            return 0.0
        return helpers.int_to_float64(v)
