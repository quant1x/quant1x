# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations
from typing import List, Union
from quant1x.data import DataHandler, Exchange, Instrument, InstrumentType, Sector, PlateCategory, Timestamp
from . import sector
from .instruments import get_instrument_info
from .kline import get_cross_section_forward_adjusted_klines
from .trans import checkout_transaction_data


def is_need_ignore(code: str) -> bool:
    """
    证券代码是否需要忽略, 这是一个不参与数据和策略处理的开关
    """
    #logger.warning(f"is_need_ignore: {code}")
    instrument = get_instrument_info(code)
    if not instrument:
        # 没找到直接忽略
        return True

    # 需要检查的关键字列表
    #ignored_keywords = ["ST", "退", "摘牌"]
    ignored_keywords = ["退", "摘牌"]

    # 转换名称为大写
    upper_name = instrument.name.upper()

    # 检查是否存在任意关键字
    return any(keyword in upper_name for keyword in ignored_keywords)

# A股指数列表
A_SHARE_INDEX_LIST = [
    "sh000001", # 上证综合指数
    "sz399001", # 深证成份指数
    "bj899050", # 北证50指数
    "sz399006", # 创业板指
    
    "sh000016", # 上证50
    "sh000300", # 沪深300指数
    "sh000688", # 科创50指数
    "sh000905", # 中证500指数
    "sh000852", # 中证1000指数
    
    "sh880005", # 通达信板块-涨跌家数
    
    "sh510050", # 上证50ETF
    "sh510300", # 沪深300ETF
    "sh588000", # 科创50ETF
    "sh510500", # 中证500ETF
    "sh512100", # 中证1000ETF
    
    "sh510900", # H股ETF
    
    "sh518880", # 黄金ETF
    "sh512480", # 半导体ETF
    "sh562500", # 机器人ETF
]

class TdxDataSource(DataHandler):
    
    def get_market_list(self) -> List[Exchange]:
        return [Exchange.SSE, Exchange.SZSE, Exchange.BSE]
    
    def get_index_list(self, market: Union[List, str] = "all") -> List[Instrument]:
        """
        获取指定市场的指数列表
        
        Args:
            market (Union[List[str], str]): 市场代码或代码列表，默认为"all"表示所有市场
                str: 市场/行业/指数简称，如"all"/"sse"/"szse"/"bse"
                list: 股票ID列表，如["ID1", "ID2"]
        
        Returns:
            List[Instrument]: 包含指数信息的Instrument对象列表，包含上证50、沪深300等主要A股指数
        """
        index_list: List[Instrument] = []
        
        for code in A_SHARE_INDEX_LIST:
            inst = get_instrument_info(code)
            if inst is None:
                continue
            # TODO: 过滤不符合条件的指数
            index_list.append(inst)
        _ = market
        return index_list
    
    def get_sector_list(self, category: PlateCategory=PlateCategory.UNKNOWN) -> List[Sector]:
        return sector.get_sector_list()
    
    def get_stock_list(self, market: Union[List, str] = "all") -> List[Instrument]:
        """
        获取指定市场的股票列表

        Args:
            market (Union[List[str], str]): 市场代码或代码列表，默认为"all"表示所有市场

                str: 市场/行业/指数简称，如"all"/"sse"/"szse"/"bse"

                list: 股票ID列表，如["ID1", "ID2"]

        Returns:
            List[Instrument]: 包含股票信息的Instrument对象列表，包含上证50、沪深300等主要A股指数

        """

        stock_list: List[Instrument] = []

        all_codes = []
    
        # 上海证券交易所 (sh600000-sh609999)
        for i in range(600000, 610000):
            fc = f"sh{i:06d}"
            if not is_need_ignore(fc):
                all_codes.append(fc)

        # 科创板 (sh688000-sh689999)
        for i in range(688000, 690000):
            fc = f"sh{i:06d}"
            if not is_need_ignore(fc):
                all_codes.append(fc)

        # 深圳主板 (sz000000-sz000999)
        for i in range(0, 1000):
            fc = f"sz{i:06d}"
            if not is_need_ignore(fc):
                all_codes.append(fc)

        # 中小板 (sz001000-sz009999)
        for i in range(1000, 10000):
            fc = f"sz{i:06d}"
            if not is_need_ignore(fc):
                all_codes.append(fc)

        # 创业板 (sz300000-sz300999)
        for i in range(300000, 310000):
            fc = f"sz{i:06d}"
            if not is_need_ignore(fc):
                all_codes.append(fc)

        # 北交所 (bj920000-bj920999)
        for i in range(920000, 921000):
            fc = f"bj{i:06d}"
            if not is_need_ignore(fc):
                all_codes.append(fc)


        for code in all_codes:
            inst = get_instrument_info(code)
            if inst is None:
                continue
            # TODO: 过滤不符合条件的指数
            stock_list.append(inst)
        return stock_list
    
    def list_instruments(self, market: Union[List, str] = "all") -> List[Instrument]:
        """
        加载全部指数、板块和个股的代码
        """
        code_list:List[Instrument] = []
        # 1. 指数
        code_list.extend(self.get_index_list())
        
        # 2. 板块
        sectors = self.get_sector_list()
        # 简化：直接按 `blocks.get_sector_list()` 返回的 list[BlockInfo] 处理
        for s in sectors:
            if s.code in A_SHARE_INDEX_LIST:
                continue
            try:
                inst = get_instrument_info(s.code)
                if inst is None:
                    continue
                code_list.append(inst)
            except Exception:
                continue

        # 3. 个股, 包括场内开放式ETF基金
        stock_list = self.get_stock_list()
        code_list.extend(stock_list)
        
        return code_list
    
    def get_instrument(self, symbol: str) -> Instrument:
        """
        获取指定代码的合约信息
        
        Args:
            symbol (str): 合约代码字符串
        
        Returns:
            Instrument: 对应的合约对象
        
        Raises:
            ValueError: 当找不到指定代码的合约时抛出
        """
        inst = get_instrument_info(symbol)
        if inst is not None:
            return inst
        raise ValueError(f"Instrument not found: {symbol}")
    
    def klines(self, symbol: str, start_date: str | None, end_date: str | None, freq: str | None):
        """
        获取指定日期范围的K线数据
        """
        _ = start_date
        _ = freq
        if end_date is None:
            as_of_ts = Timestamp.now()
        else:
            as_of_ts = Timestamp.parse(end_date)
        as_of_date = as_of_ts.only_date()
        return get_cross_section_forward_adjusted_klines(symbol, as_of_date)
    
    def transactions(self, symbol: str, date: str | None, **kwargs):
        """
        获取指定日期的交易数据
        """
        if date is None:
            timestamp = Timestamp.now()
        else: 
            timestamp = Timestamp.parse(date)
        return checkout_transaction_data(symbol, timestamp, ignore_previous_data=False)

    
if __name__ == "__main__":
    D = TdxDataSource()
    sectors = D.get_sector_list()
    print(sectors)
    indexes = D.get_index_list()
    print("index: ", len(indexes))
    sectors = D.get_sector_list()
    print("sector: ", len(sectors))
    stocks = D.get_stock_list()
    print("stock: ", len(stocks))
    codes = D.list_instruments()
    print("total: ", len(codes))