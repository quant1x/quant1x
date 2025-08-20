import pandas as pd
import requests
from typing import List


def gen_fc(stock_code: str) -> str:
    '''
    生成东方财富专用的 secid

    Parameters
    ----------
    stock_code : 6 位股票代码

    Return
    ------
    str: 指定格式的字符串

    '''
    # 沪市指数
    if stock_code[:3] == '000':
        return f'{stock_code}02'
    # 深证指数
    if stock_code[:3] == '399':
        return f'{stock_code}01'
    # 深市股票
    if stock_code[0] != '6':
        return f'{stock_code}02'
    # 沪市股票
    return f'{stock_code}01'


def get_public_dates(stock_code: str, top: int = 4) -> List[str]:
    '''
    获取指定股票公开股东信息的日期
    Parameters
    ----------
    stock_code : 6 位 A 股股票代码
    top : 最新的 top 个日期

    Return
    ------
    公开股东信息的日期列表

    '''
    headers = {
        'User-Agent': 'Mozilla/5.0 (iPhone; CPU iPhone OS 14_4 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/15E148 color=b eastmoney_ios appversion_9.3 pkg=com.eastmoney.iphone mainBagVersion=9.3 statusBarHeight=20.000000 titleBarHeight=44.000000 density=2.000000 fontsize=3',
        'Content-Type': 'application/json;charset=utf-8',
        'Host': 'emh5.eastmoney.com',
        'Origin': 'null',
        'Cache-Control': 'public',
    }
    fc = gen_fc(stock_code)
    data = {"fc": fc}
    response = requests.post(
        'https://emh5.eastmoney.com/api/GuBenGuDong/GetFirstRequest2Data', headers=headers, json=data)
    items: list[dict] = response.json()[
        'Result']['SDLTGDBGQ']
    items = items.get('ShiDaLiuTongGuDongBaoGaoQiList')

    if items is None:
        return []

    df = pd.DataFrame(items)
    if 'BaoGaoQi' not in df:
        return []
    dates = df['BaoGaoQi'][:top]
    return dates


def get_top10_stock_holder_info(stock_code: str, top: int = 4) -> pd.DataFrame:
    '''
    获取前十大股东信息

    Parameters
    ----------
    stock_code: 6 位股票代码
    top : 最新 top 个前 10 大流通股东公开信息

    Return
    ------
    DataFrame
    '''

    fields = {
        'GuDongDaiMa': '股东代码',
        'GuDongMingCheng': '股东名称',
        'ChiGuShu': '持股数',
        'ChiGuBiLi': '持股比例',
        'ZengJian': '增减',
        'BianDongBiLi': '变动率',

    }
    headers = {
        'User-Agent': 'Mozilla/5.0 (iPhone; CPU iPhone OS 14_4 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/15E148 color=b eastmoney_ios appversion_9.3 pkg=com.eastmoney.iphone mainBagVersion=9.3 statusBarHeight=20.000000 titleBarHeight=44.000000 density=2.000000 fontsize=3',
        'Content-Type': 'application/json;charset=utf-8',
        'Host': 'emh5.eastmoney.com',
        'Origin': 'null',
        'Cache-Control': 'public',
    }
    fc = gen_fc(stock_code)
    dates = get_public_dates(stock_code)
    dfs: List[pd.DataFrame] = []
    for date in dates[:top]:

        data = {"fc": fc, "BaoGaoQi": date}
        response = requests.post(
            'https://emh5.eastmoney.com/api/GuBenGuDong/GetShiDaLiuTongGuDong',
            headers=headers,
            json=data)
        response.encoding = 'utf-8'

        try:
            items: list[dict] = response.json(
            )['Result']['ShiDaLiuTongGuDongList']

        except:
            df = pd.DataFrame(columns=fields.values())
            df.insert(0, '股票代码', [stock_code] * len(len(df)))
            df.insert(1, '更新日期', [date] * len(df))
            return df
        df = pd.DataFrame(items)
        df.rename(columns=fields, inplace=True)
        df.insert(0, '股票代码', [stock_code for _ in range(len(df))])
        df.insert(1, '更新日期', [date for _ in range(len(df))])
        del df['IsLink']
        dfs.append(df)

    return pd.concat(dfs, axis=0)


if __name__ == '__main__':
    # 股票代码
    stock_code = '002423'
    # 前 12 个公开前 10 大股东信息日（一般是每季度更新一次）
    top = 12
    save_path = f'{stock_code}-前10流通股东历史数据.csv'
    # 核心函数，根据股票代码和要获取的公开日期数获取前 10 大流通股东信息
    df = get_top10_stock_holder_info(stock_code=stock_code, top=top)
    # 存储获取到的数据到 csv 文件中
    df.to_csv(save_path,
              index=None,
              encoding='utf-8-sig')
    print(f'股票代码为 {stock_code} 的股票前 10 大流通股东信息已存储到文件 {save_path} 中')
