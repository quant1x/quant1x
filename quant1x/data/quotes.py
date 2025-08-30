#!/usr/bin/python
# -*- coding: UTF-8 -*-
import datetime
import os.path

import akshare as ak
import mootdx.utils.holiday as mooUtils
import pandas as pd
import requests
from mootdx.quotes import Quotes
from tqdm import tqdm

from quant1x.formula.formula import *
from quant1x.data import *

# 中国内地证券市场
MARKET_SZ = 0  # 深市
MARKET_SH = 1  # 沪市
MARKET_BJ = 2  # 北交


class DataHandler:
    """
    数据集
    """

    def __init__(self):
        """
        无参数构造函数
        """
        self.__root = os.path.expanduser(quant1x_home)
        self.__data_cn = os.path.expanduser(quant1x_data_cn)
        self.__data_hk = os.path.expanduser(quant1x_data_hk)
        self.__info_cn = os.path.expanduser(quant1x_info_cn)
        self.__info_hk = os.path.expanduser(quant1x_info_hk)

        # 根路径
        if not os.path.exists(self.__root):
            os.makedirs(self.__root)
        # 数据路径
        self.__path = os.path.expanduser(quant1x_data)
        if not os.path.exists(self.__data_cn):
            os.makedirs(self.__data_cn)
        if not os.path.exists(self.__data_hk):
            os.makedirs(self.__data_hk)
        # 咨询路径
        self.__info = os.path.expanduser(quant1x_info)
        if not os.path.exists(self.__info_cn):
            os.makedirs(self.__info_cn)
        if not os.path.exists(self.__info_hk):
            os.makedirs(self.__info_hk)

        self.__stock_list = {}
        # 自选股csv文件路径
        zxg_csv = self.__path + '/zxg.csv'
        self.__stock_list = pd.read_csv(zxg_csv)

        # 标准市场
        self.__client = Quotes.factory(market='std', multithread=True, heartbeat=True)

    def __NotExistsToMake(self, path):
        """
        如果不存在则创建路径
        :param path:
        :return:
        """
        if not os.path.exists(path):
            os.makedirs(path)

    def get_stock_market(self, symbol="", string=False):
        """
        判断股票ID对应的证券市场匹配规则
        ['50', '51', '60', '90', '110'] 为 sh
        ['00', '12'，'13', '18', '15', '16', '18', '20', '30', '39', '115'] 为 sz
        ['5', '6', '9'] 开头的为 sh， 其余为 sz

        :param string: False 返回市场ID，否则市场缩写名称
        :param symbol: 股票ID, 若以 'sz', 'sh' 开头直接返回对应类型，否则使用内置规则判断
        :return 'sh' or 'sz'
        """

        assert isinstance(symbol, str), "stock code need str type"
        market = "sh"
        if symbol.startswith(("sh", "sz", "SH", "SZ")):
            market = symbol[:2].lower()
        elif symbol.startswith(("50", "51", "60", "68", "90", "110", "113", "132", "204")):
            market = "sh"
        elif symbol.startswith(("00", "12", "13", "18", "15", "16", "18", "20", "30", "39", "115", "1318")):
            market = "sz"
        elif symbol.startswith(("5", "6", "9", "7")):
            market = "sh"
        elif symbol.startswith(("4", "8")):
            market = "bj"
        if string is False:
            if market == "sh":
                market = MARKET_SH
            if market == "sz":
                market = MARKET_SZ
            if market == "bj":
                market = MARKET_BJ
        return market

    def apply(self, func_name: str, func) -> pd.DataFrame:
        """
        自选股迭代
        :param func_name:
        :param func:
        :return:
        """
        total = len(self.__stock_list)
        print("自选股[%s]处理, 共计[%d]:" % (func_name, total))
        values = enumerate(self.__stock_list.values)
        pbar = tqdm(values, total=total)
        df = pd.DataFrame()
        for key, value in pbar:
            code = value[1][2:]
            name = value[2]
            try:
                pbar.set_description_str("[%s]进行中" % code)
                s = func(code, name)
                df1 = pd.DataFrame([s])
                df = pd.concat([df, df1], ignore_index=True)
            except ValueError:
                pass
            finally:
                pbar.set_description_str("[%s]完成" % code)
        pbar.close()
        print("自选股[%s], 处理完成." % func_name)
        return df

    def holiday(self, dt: datetime):
        """
        判断datetime是否假日
        :param dt:
        :return:
        """
        # date = dt.fromtimestamp("%Y-%m-%d")
        date = dt.strftime("%Y-%m-%d")
        return mooUtils.holiday(date)

    def xdxr(self, code) -> pd.DataFrame:
        """
        读取除权信息
        :param code: 股票代码
        :return:
        """
        filename = self.__data_cn + '/' + code + '-xdxr.csv'
        if not os.path.exists(filename):
            # 如果历史数据文件不存在则更新
            self.__update_xdxr(code)
        df = pd.read_csv(filename)

        # df['date'] = "%4d-%2d-%2d" % (df['year'], df['month'], df['day'])
        date = df.apply(lambda row: "%04d-%02d-%02d" % (1 * row['year'], 1 * row['month'], 1 * row['day']), axis=1)
        df.drop('year', axis=1, inplace=True)
        df.drop('month', axis=1, inplace=True)
        df.drop('day', axis=1, inplace=True)
        # df = pd.DataFrame({'date': date, df.values})
        df.insert(0, 'date', date)
        return df

    def __down_xdxr(self, code) -> pd.DataFrame:
        """
        除权信息
        :param code: 股票代码
        :return:
        """
        data = self.__client.xdxr(symbol=code)
        return data

    def __update_xdxr(self, code):
        """
        更新单只个股的除权信息
        :param code:
        :return:
        """
        data = self.__down_xdxr(code)
        data.to_csv(self.__data_cn + '/' + code + '-xdxr.csv', index=False)

    def update_xdxr(self):
        """
        更新全量除权信息
        :return:
        """
        total = len(self.__stock_list)
        print("自选 股除权信息, 共计[%d]:" % total)
        values = enumerate(self.__stock_list.values)
        pbar = tqdm(values, total=total)
        for key, value in pbar:
            code = value[1][2:]
            pbar.set_description_str("同步[%s]进行中" % code)
            self.__update_xdxr(code)
            pbar.set_description_str("同步[%s]完成" % code)
            break
        pbar.close()
        print("自选股 除权信息, 处理完成.\n")

    def __kline_cn(self, code):
        """
        获取全量历史K线数据
        """
        symbol = code[-6:]
        data = ak.stock_zh_a_hist(symbol=symbol, period="daily", adjust="qfq")
        return data

    def update_history(self):
        """
        更新全量历史数据
        :return:
        """
        total = len(self.__stock_list)
        print("自选股 历史K线数据, 共计[%d]:" % total)
        values = enumerate(self.__stock_list.values)
        pbar = tqdm(values, total=total)
        for key, value in pbar:
            # print('key:', key, ', value: ', value)
            code = value[1][2:]
            # print("code:%s" % code)
            pbar.set_description_str("同步[%s]进行中" % code)
            data = self.__kline_cn(code)
            data.to_csv(self.__data_cn + '/' + code + '.csv', index=False)
            pbar.set_description_str("同步[%s]完成" % code)
        pbar.close()
        print("自选股 历史K线数据, 处理完成.\n")

    def dataset(self, code) -> pd.DataFrame:
        """
        读取历史数据
        :param code:
        :return:
        """
        filename = self.__data_cn + '/' + code + '.csv'
        if not os.path.exists(filename):
            # 如果历史数据文件不存在则更新
            data = self.__kline_cn(code)
            data.to_csv(self.__data_cn + '/' + code + '.csv', index=False)
        df = pd.read_csv(filename)
        # 选择列, 是为了改变表头
        df = df[["日期", "开盘", "收盘", "最高", "最低", "成交量", "成交额"]]
        # 变更表头
        df.columns = ['date', 'open', 'close', 'high', 'low', 'volume', 'amount']
        # 更正排序
        df['date'] = pd.to_datetime(df['date'])
        # df.set_index('date', inplace=True)

        return df

    def __finance(self, code):
        """
        获取个股基本信息
        :param code:
        :return:
        """
        symbol = code[-6:]
        data = self.__client.finance(symbol=symbol)
        return data

    def update_info(self):
        """
        更新全量个股信息
        :return:
        """

        total = len(self.__stock_list)
        print("自选股 基本面信息, 共计[%d]:" % total)
        values = enumerate(self.__stock_list.values)
        pbar = tqdm(values, total=total)
        for key, value in pbar:
            # print('key:', key, ', value: ', value)
            code = value[1][2:]
            # print("code:%s" % code)
            pbar.set_description_str("同步[%s]进行中" % code)
            data = self.__finance(code=code)
            data.to_csv(self.__info_cn + '/' + code + '.csv', index=False)
            pbar.set_description_str("同步[%s]完成" % code)
        pbar.close()
        print("自选股 基本面信息, 处理完成.\n")

    def finance(self, code):
        """
        读取本地基本面
        :param code:
        :return:
        """
        filename = self.__info_cn + '/' + code + '.csv'
        if not os.path.exists(filename):
            return
        df = pd.read_csv(filename)
        return df

    def tick(self, code: str, date: str) -> pd.DataFrame:
        """
        读取分笔成交记录
        :param code: 股票代码
        :return:
        """
        filename = self.__data_cn + '/' + code + '-' + date + '-tick.csv'
        if not os.path.exists(filename):
            # 如果历史数据文件不存在则更新
            self.__update_tick(code, date)
        df = pd.DataFrame()
        if os.path.exists(filename):
            df = pd.read_csv(filename)
        return df

    def __down_tick(self, code: str, date: str) -> pd.DataFrame:
        """
        分笔成交
        """
        offset = 1800
        start = 0
        df = pd.DataFrame()
        while True:
            data = self.__client.transactions(symbol=code, date=date, start=start, offset=1800)
            if len(data) > 0:
                df = pd.concat([data, df], ignore_index=True)
                start += offset
            if len(data) < offset:
                break
        return df

    def __update_tick(self, code: str, date: str):
        """
        更新分笔成交记录
        :param code:
        :return:
        """
        data = self.__down_tick(code, date)
        if len(data) > 0:
            filename = self.__data_cn + '/' + code + '-' + date + '-tick.csv'
            data.to_csv(filename, index=False)

    def update_tick(self):
        """
        更新全量分笔成交记录
        :return:
        """
        total = len(self.__stock_list)
        print("自选股 全量分笔成交记录, 共计[%d]:" % total)
        values = enumerate(self.__stock_list.values)
        pbar = tqdm(values, total=total)
        for key, value in pbar:
            code = value[1][2:]
            pbar.set_description_str("同步[%s]进行中" % code)
            info = self.finance(code)
            start = info.iloc[0, 6] * 1
            d = datetime.datetime.now()
            # today = int("%04d%02d%02d" % (d.year, d.month, d.day))
            today = "%04d%02d%02d" % (d.year, d.month, d.day)
            for dt in pd.date_range(start=str(start), end=today, freq='B'):
                date = dt.strftime('%Y%m%d')
                filename = self.__data_cn + '/' + code + '-' + str(date) + '-tick.csv'
                if not os.path.exists(filename):
                    self.__update_tick(code, str(date))
            pbar.set_description_str("同步[%s]完成" % code)
        pbar.close()
        print("自选股 全量分笔成交记录, 处理完成.\n")

    def forecast(self, code: str) -> pd.DataFrame:
        """
        读取 估值分析
        :param code: 股票代码
        :return:
        """
        filename = self.__data_cn + '/' + code + '-forecast.csv'
        if not os.path.exists(filename):
            # 如果历史数据文件不存在则更新
            self.__update_forecast(code)
        df = pd.DataFrame()
        if os.path.exists(filename):
            df = pd.read_csv(filename)
        return df

    def __down_forecast(self, symbol: str):
        """
        下载 估值分析
        东方财富网-数据中心-研究报告-盈利预测
        https://data.eastmoney.com/report/profitforecast.jshtml
        :param symbol: "", 默认为获取全部数据; symbol="船舶制造", 则获取具体行业板块的数据; 行业板块可以通过 ak.stock_board_industry_name_em() 接口获取
        :type symbol: str
        :return: 盈利预测
        :rtype: pandas.DataFrame
        """
        url = "https://datacenter-web.eastmoney.com/api/data/v1/get"
        params = {
            'reportName': 'RPT_VALUEANALYSIS_DET',
            'columns': 'ALL',
            'pageNumber': '1',
            'pageSize': '5000',
            'sortColumns': 'TRADE_DATE',
            'sortTypes': '1',
            'source': 'WEB',
            'client': 'WEB',
            'filter': '',
            '_': '1676520096781',
        }
        if symbol:
            params.update({'filter': f'(SECURITY_CODE="{symbol}")'})

        r = requests.get(url, params=params)
        data_json = r.json()
        # page_num = int(data_json['result']['pages'])
        df = pd.DataFrame(data_json['result']['data'])

        # date = df.apply(lambda row: row['TRADE_DATE'].strftime("%Y-%m-%d"), axis=1)
        date = df.apply(lambda row: row['TRADE_DATE'][0:10], axis=1)
        df.drop('TRADE_DATE', axis=1, inplace=True)
        df.insert(0, 'date', date)
        return df

    def __update_forecast(self, code: str):
        """
        更新分笔成交记录
        :param code:
        :return:
        """
        data = self.__down_forecast(code)
        if len(data) > 0:
            filename = self.__data_cn + '/' + code + '-forecast.csv'
            data.to_csv(filename, index=False)

    def update_forecast(self):
        """
        更新全量分笔成交记录
        :return:
        """
        total = len(self.__stock_list)
        print("自选股 估值分析, 共计[%d]:" % total)
        values = enumerate(self.__stock_list.values)
        pbar = tqdm(values, total=total)
        for key, value in pbar:
            code = value[1][2:]
            pbar.set_description_str("同步[%s]进行中" % code)
            self.__update_forecast(code)
            pbar.set_description_str("同步[%s]完成" % code)
        pbar.close()
        print("自选股 估值分析, 处理完成.\n")

    def stock_hist(self,
                   symbol: str = "000001",
                   period: str = "daily",
                   start_date: str = "19700101",
                   end_date: str = "20500101",
                   adjust: str = "",
                   ) -> pd.DataFrame:
        """
        东方财富网-行情首页-沪深京 A 股-每日行情
        https://quote.eastmoney.com/concept/sh603777.html?from=classic
        :param symbol: 股票代码
        :type symbol: str
        :param period: choice of {'daily', 'weekly', 'monthly'}
        :type period: str
        :param start_date: 开始日期
        :type start_date: str
        :param end_date: 结束日期
        :type end_date: str
        :param adjust: choice of {"qfq": "前复权", "hfq": "后复权", "": "不复权"}
        :type adjust: str
        :return: 每日行情
        :rtype: pd.DataFrame
        """
        makertId = self.get_stock_market(symbol)
        adjust_dict = {"qfq": "1", "hfq": "2", "": "0"}
        period_dict = {"daily": "101", "weekly": "102", "monthly": "103"}
        url = "http://push2his.eastmoney.com/api/qt/stock/kline/get"
        params = {
            "fields1": "f1,f2,f3,f4,f5,f6",
            "fields2": "f51,f52,f53,f54,f55,f56,f57,f58,f59,f60,f61,f116",
            # "fs": "m:0 f:8,m:1 f:8",
            # "fields1":"f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f12,f13,f14,f15,f16,f17,f18,f20,f21,f23,f24,f25,f26,f22,f11,f62,f128,f136,f115,f152",
            "ut": "7eea3edcaed734bea9cbfc24409ed989",
            "klt": period_dict[period],
            "fqt": adjust_dict[adjust],
            "secid": f"{makertId}.{symbol}",
            "beg": start_date,
            "end": end_date,
            "_": "1623766962675",
        }
        r = requests.get(url, params=params)
        data_json = r.json()
        if not (data_json["data"] and data_json["data"]["klines"]):
            return pd.DataFrame()
        code = symbol
        name = data_json['data']['name']
        temp_df = pd.DataFrame(
            [item.split(",") for item in data_json["data"]["klines"]]
        )
        temp_df.columns = [
            "日期",
            "开盘",
            "收盘",
            "最高",
            "最低",
            "成交量",
            "成交额",
            "振幅",
            "涨跌幅",
            "涨跌额",
            "换手率",
        ]
        temp_df.index = pd.to_datetime(temp_df["日期"])
        temp_df.reset_index(inplace=True, drop=True)

        temp_df["开盘"] = pd.to_numeric(temp_df["开盘"])
        temp_df["收盘"] = pd.to_numeric(temp_df["收盘"])
        temp_df["最高"] = pd.to_numeric(temp_df["最高"])
        temp_df["最低"] = pd.to_numeric(temp_df["最低"])
        temp_df["成交量"] = pd.to_numeric(temp_df["成交量"])
        temp_df["成交额"] = pd.to_numeric(temp_df["成交额"])
        temp_df["振幅"] = pd.to_numeric(temp_df["振幅"])
        temp_df["涨跌幅"] = pd.to_numeric(temp_df["涨跌幅"])
        temp_df["涨跌额"] = pd.to_numeric(temp_df["涨跌额"])
        temp_df["换手率"] = pd.to_numeric(temp_df["换手率"])

        dLen = len(temp_df)
        df = pd.DataFrame()
        # 交易日(Date)
        df['交易日(Date)'] = temp_df['日期']
        # 股票代码(Symbol)
        df['股票代码(Symbol)'] = np.repeat(code, dLen)
        # 股票名称(Name)
        df['股票名称(Name)'] = np.repeat(name, dLen)
        # 当日开盘价(Open)
        df['当日开盘价(Open)'] = temp_df["开盘"]
        # 当日收盘价(Close)
        df['当日收盘价(Close)'] = temp_df["收盘"]
        # 当日最高价(High)
        df['当日最高价(High)'] = temp_df["最高"]
        # 当日最低价(Low)
        df['当日最低价(Low)'] = temp_df['最低']
        # 平均价(AvgPrice)
        df['平均价(AvgPrice)'] = np.round(temp_df['成交额'] / temp_df['成交量'] / 100, 3)
        # 昨日收盘价(PrevClose), 收盘价-涨跌额
        df['昨日收盘价(PrevClose)'] = np.round(temp_df['收盘'] - temp_df["涨跌额"], 3)
        # 涨跌额(Change)
        df['涨跌额(Change)'] = temp_df["涨跌额"]
        # 涨跌幅(PctChg)
        df['涨跌幅(PctChg)'] = temp_df["涨跌幅"]
        # 振幅(Amplitude)
        df['振幅(Amplitude)'] = temp_df["振幅"]
        # 换手率(TurnoverRatio)
        df['换手率(TurnoverRatio)'] = temp_df["换手率"]
        # 成交量(Volume)
        df['成交量(Volume)'] = temp_df["成交量"]
        # 成交额(Turnover)
        df['成交额(Turnover)'] = temp_df["成交额"]
        # 总市值(MarketCAP)
        df['总市值(MarketCAP)'] = np.repeat(0.00, dLen)
        # 总股本(SharedOutstanding)
        df['总股本(SharedOutstanding)'] = np.repeat(0.00, dLen)
        # 流通值(FloatCAP)
        df['流通值(FloatCAP)'] = np.repeat(0.00, dLen)
        # 流通股(ShsFloat)
        df['流通股(ShsFloat)'] = np.repeat(0.00, dLen)
        # 市盈率TTM(PETTM)
        df['市盈率TTM(PETTM)'] = np.repeat(0.00, dLen)
        # 市盈率静(PEStatic)
        df['市盈率静(PEStatic)'] = np.repeat(0.00, dLen)
        # 市净率(PB)
        df['市净率(PB)'] = np.repeat(0.00, dLen)
        # 委比(BidAskPct)
        df['委比(BidAskPct)'] = np.repeat(0.00, dLen)
        # 量比(VolumePct)
        df['量比(VolumePct)'] = np.repeat(0.00, dLen)
        vol = temp_df["成交量"].copy()
        bl = vol.rolling(5).sum() / (240 * 5)
        ms = self.__client.minute(symbol=code)
        minutes = len(ms) if len(ms) else 240
        df['量比(VolumePct)'] = (vol / minutes) / REF(bl, 1)

        # 净流入量(NetInflowVolume)
        df['净流入量(NetInflowVolume)'] = np.repeat(0.00, dLen)
        # 净流入额(NetInflowAmount)
        df['净流入额(NetInflowAmount)'] = np.repeat(0.00, dLen)

        # 处理流通股本
        xdxr = self.xdxr(code)
        xdxr['date'] = pd.to_datetime(xdxr['date'])
        xdxr.set_index('date', inplace=True, drop=True)
        forecast = self.forecast(code)
        forecast['date'] = pd.to_datetime(forecast['date'])
        forecast.set_index('date', inplace=True, drop=True)
        # 处理流通股本
        if len(df) > 0:
            df = df.copy()
            dr = None
            values = enumerate(df['交易日(Date)'])
            pbar = tqdm(values, total=len(df))
            for idx, row_ in pbar:
                # for idx, row in df.iterrows():
                row = df.loc[idx]
                # 遍历除权信息
                dt = row['交易日(Date)']
                # print(dt)
                # 处理分笔成交数据
                fb = self.tick(code, str(dt).replace('-', ''))
                buy_volume = 0
                buy_amount = 0.00
                sell_volume = 0
                sell_amount = 0.00
                for fbidx, fbrow in fb.iterrows():
                    if fbrow['buyorsell'] == 1:
                        sell_volume += fbrow['vol']
                        sell_amount += fbrow['vol'] * fbrow['price'] * 100
                    else:
                        buy_volume += fbrow['vol']
                        buy_amount += fbrow['vol'] * fbrow['price'] * 100
                df.loc[idx, '净流入量(NetInflowVolume)'] = buy_volume - sell_volume
                df.loc[idx, '净流入额(NetInflowAmount)'] = buy_amount - sell_amount
                # df.iloc[idx] = row
                try:
                    info = xdxr.loc[dt]
                    if info is None:
                        continue
                    if isinstance(info, np.int64) and info['category'] != 5:
                        continue
                    if isinstance(info['panhouliutong'], np.float64) and info['panhouliutong'] < 1:
                        continue
                    if not dr is None:
                        dr = info
                        continue
                    dr = info
                except KeyError:
                    pass
                if not dr is None and isinstance(dr['panhouliutong'], np.float64):
                    df.loc[idx, '流通股(ShsFloat)'] = dr['panhouliutong'] * 10000
                    df.loc[idx, '流通值(FloatCAP)'] = row['当日收盘价(Close)'] * dr['panhouliutong'] * 10000
                    df.loc[idx, '总股本(SharedOutstanding)'] = dr['houzongguben'] * 10000
                    df.loc[idx, '总市值(MarketCAP)'] = row['当日收盘价(Close)'] * dr['houzongguben'] * 10000
                try:
                    fc = forecast.loc[dt]
                    df.loc[idx, '市盈率TTM(PETTM)'] = fc['PE_TTM']
                    df.loc[idx, '市盈率静(PEStatic)'] = fc['PE_LAR']
                    df.loc[idx, '市净率(PB)'] = fc['PB_MRQ']
                except KeyError:
                    pass
            pbar.close()
        return df
