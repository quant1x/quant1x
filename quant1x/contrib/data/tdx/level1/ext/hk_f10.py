# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from numpy import fix
import requests, json, csv

from dataclasses import dataclass
from typing import List, Dict
from datetime import date, datetime

import scipy as sp

from quant1x.base.numeric import float_round
from quant1x.data.meta.instrument import Instrument
from quant1x.data.schema.adjustment import XdxrInfo
from quant1x.data.meta.forex import ExchangeRateCache
from quant1x.log import logger
from quant1x.data.meta.region import Region
from quant1x.contrib.data.tdx.money import parse_scheme_info

TDX_URL_HK_F10 = 'http://page1.tdx.com.cn:7615/TQLEX?Entry=CWServ.skef10_hk_gsds'
TDX_URL_TQLEX = 'http://page1.tdx.com.cn:7615/TQLEX'
TDX_F10_CONFIG = {
    "hk": {
        "dirname": "hkf10", # 香港F10数据目录
        "menus": [
            ("最新提示", "skef10_hk_zxts"),
            ("公司大事", "skef10_hk_gsds"),
            ("公司概况", "skef10_hk_gsgk"),
            ("关联证券", "skef10_hk_glzq"),
            ("高管资料", "skef10_hk_ggzl"),
            ("股本结构", "skef10_hk_gbjg"),
            ("股东持股", "skef10_hk_gdcg"),
            ("财务分析", "skef10_hk_cwfx"),
            ("财务指标", "skef10_hk_cwzb"),
            ("经营分析", "skef10_hk_jyfx"),
            ("行业比较", "skef10_hk_hybj"),
            ("市场行情", "skef10_hk_schq"),
            ("公司资讯", "skef10_hk_gszx"),
            ("热点题材", "skef10_hk_rdtc"),
            ("深度分析", "skef10_hk_sdfx"),
        ],
    },
    "sb": {
        "dirname": "sbf10", # 三板F10数据目录
        "menus": [
            ("最新提示", "skef10_sb_zxts"),
            ("交易大事", "skef10_sb_jyds"),
            ("公司概况", "skef10_sb_gsgk"),
            ("股本结构", "skef10_sb_gbjg"),
            ("股东研究", "skef10_sb_gdyj"),
            ("合规运营", "skef10_sb_hgyy"),
            ("分红融资", "skef10_sb_fhpx"),
            ("财务分析", "skef10_sb_cwfx"),
            ("经营分析", "skef10_sb_jyfx"),
            ("重大事项", "skef10_sb_zdsx"),
            ("公司资讯", "skef10_sb_gszx"),
            ("行业比较", "skef10_sb_hybj"),
        ],
    },
    "mg": {
        "dirname": "mgf10", # 美股F10数据目录
        "menus": [
            ("最新提示", "skef10_mg_zxts"),
            ("公司概况", "skef10_mg_gsgk"),
            ("财务分析", "skef10_mg_cwfx"),
            ("经营分析", "skef10_mg_jyfx"),
            ("分红融资", "skef10_mg_fhrz"),
            ("股本结构", "skef10_mg_gbjg"),
            ("股东权益", "skef10_mg_sdgd"),
            ("董事高管", "skef10_mg_dsgg"),
            ("市场行情", "skef10_mg_schq"),
            ("公司资讯", "skef10_mg_gszx"),
        ],
    },
    "jj": {
        "dirname": "jjf10", # 基金F10数据目录
        "menus": [
            ("最新提示", "skef10_jj_jjzl"),
            ("规模份额", "skef10_jj_gmfe"),
            ("净值回报", "skef10_jj_jzhb"),
            ("投资组合", "skef10_jj_tzzh"),
            ("持股情况", "skef10_jj_cgqk"),
            ("财务状况", "skef10_jj_cwzk"),
            ("销售信息", "skef10_jj_xsxx"),
        ],
    },
}

# 市场类型映射
F10_MARKET_MAP = {
    Region.US: "mg", # 美股
    Region.HK: "hk", # 港股
    # "sb": "三板",
    # "jj": "基金",
}

F10_HK_CATEGORY = {
    '1': '分红送股',
    '2': '配股',
    '3': '增发',
    '4': '增发配股',
    '5': '配股增发',
    '6': '增发配股增发',
    '7': '其他'
}

F10_CONFIG = {
    "hk": {
        'profile':{ # 公司概况
            'entry':'skef10_hk_gsgk', # 公司概况
            # 'info':{
            #     'name':'公司名称',
            #     'code':'公司代码',
            #     'industry':'所属行业',
            #     'website':'公司网址',
            #     'found_date':'成立日期',
            #     'chairman':'董事长',
            #     'ceo':'CEO',
            #     'reg_capital':'注册资本',
            #     'reg_address':'注册地址',
            #     'reg_number':'注册号',
            #     'ipo_date':'上市日期',
            #     'ipo_price':'发行价',
            #     'ipo_amount':'发行数量',
            #     'ipo_price_usd':'发行价(美元)',
            #     'ipo_amount_usd':'发行数量(美元)',
            #     'ipo_price_hkd':'发行价(港元)',
            #     'ipo_amount_hkd':'发行数量(港元)',
            #     'ipo_price_cny':'发行价(人民币)',
            #     'ipo_amount_cny':'发行数量(人民币)',
            # },
            'instrument_info': 5, # 证券资料
            'company_info': 1, # 公司资料
            'ipo_info': 2, # IPO资料
            'index_adjust': 6, # 主要指数调整
            'over_weight_share': 4, # 超额配股权
            'controlled_companies': 3, # 参控股公司
        },
        # 分红送股
        'dividend':{
            'entry':'skef10_hk_gsds', # 公司大事
            'dividend_info': 1, # 分红送股
            # 配股/供股
            'rights_issue_info': 5, # 配股/供股
            # 扩股缩股/拆股合并
            'stock_split_info': 6, # 拆股合并
        },
        # 股本
        'capital':{ # 股本
            'entry':'skef10_hk_gbjg', # 股本结构
            'capital_structure': 1, # 股本结构, 历史股本变化
        },
        # 公告
        'notice':{
            'entry':'skef10_hk_gsds', # 公司大事
            'notice_info': 1, # 公告信息
        },
    },
    "us": {
        'profile':{
            'entry':'skef10_mg_gsgk', # 公司概况
            'instrument_info': 5, # 证券资料
            'company_info': 1, # 公司资料
            'ipo_info': 2, # IPO资料
            'index_adjust': 6, # 主要指数调整
            'over_weight_share': 4, # 超额配股权
            'controlled_companies': 3, # 参控股公司
        },
    },
}


# 默认http协议请求的header
DEFAULT_HTTP_REQUEST_HEADERS = {
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/145.0.0.0 Safari/537.36 Edg/145.0.0.0",
    "Accept": "text/plain, */*; q=0.01",
    "Accept-Language": "zh-CN,zh;q=0.9,en;q=0.8",
    #"Content-Type": "application/x-www-form-urlencoded; charset=UTF-8",  # POST 必填
    # Cookie 建议用 session.cookies 管理, 此处仅为演示
    # "Cookie": "Hm_lvt_5c4c948b141e4d66943a8430c3d600d0=...; ..."
}

def parse_currency(text: str=None) -> str:
    currency_raw = text or '港元'
    if currency_raw == '人民币':
        currency = Region.CN.currency
    elif currency_raw in ('港元','港币'):
        currency = Region.HK.currency
    elif currency_raw in ('美元','美金'):
        currency = Region.US.currency
    elif currency_raw in ('欧元','EUR'):
        currency = Region.EU.currency
    elif currency_raw in ('英镑'):
        currency = Region.UK.currency
    elif currency_raw in ('日元','JPY'):
        currency = Region.JP.currency
    elif currency_raw in ('新加坡元'):
        currency = Region.SG.currency
    else: # 默认港元
        currency = Region.HK.currency
    return currency

@dataclass
class DividendRecord:
    # --- 1. 基础配置 (Configuration) ---
    ex_date: str            # 除权日
    # --- 2. 无偿权益 (Free Distributions) ---
    # 现金分红 (Cash Dividend), 税前金额
    amount: float           # 每股金额
    currency: str           # 币种
    info_date: str          # 公告日期
    summary: str            # 分配方案说明
    pay_date: str           # 派息日
    cdate_begin: str        # 过户开始日
    cdate_end: str          # 过户截止日
    # 送红股 (Stock Dividend), 来源于未分配利润
    bonus_shares_per: float=0.0 # 送股比例
    # 转增股本 (Capitalization), 来源于资本公积
    capitalization_shares_per: float=0.0 # 转增股比例
    # --- 3. 有偿权益 (Rights Issue / Paid) ---
    # 股东需支付对价才能获得新股
    # 配股比例 (Rights Ratio), 如每10股配1股
    rights_shares: float=0    # 新股数量
    # 配股价格 (Rights Price), 每股认购价格
    rights_price: float=0.0   # 每股价格
    
def hk_parse_dividend_data(resp_json: dict, symbol: str=None, preferred_currency:str=Region.HK.currency) -> List[DividendRecord]:
    records = []
    for rs in resp_json.get('ResultSets', []):
        if rs.get('ResultSetKey') == 'table0':
            col_names = rs['ColName']
            for row in rs['Content']:
                ex_dividend_date = row[2] or ''
                amount_str = row[7] or ''
                amount=float(amount_str) if amount_str else 0.0
                
                # if symbol == '00008' and row[2]=='2020-09-03':
                #     amount+=0.0918
                scheme_info = row[1] or ''
                scheme = parse_scheme_info(date=ex_dividend_date,overview=scheme_info, preferred_currency=preferred_currency)
                scheme_count = len(scheme.get('dividend_components', []))
                if amount > 0 and scheme_count > 1:
                    fix_amount = float_round(amount/10, 4)
                    # 如果分红方案包含多个分红组件, 需要检测是否存在遗漏
                    new_amount = 0.0
                    amounts = []
                    for component in scheme['dividend_components']:
                        tmp_amount = component['amount'] or 0.0
                        tmp_amount = float_round(tmp_amount, 4)
                        if tmp_amount > 0 and preferred_currency == scheme.get('currency_word', preferred_currency):
                            amounts.append(tmp_amount)
                    if len(amounts) > 1 and amount in amounts:
                            new_amount = sum(amounts)
                            amount = new_amount
                            logger.warning(f"修正分红金额: {symbol} {ex_dividend_date} 从 {fix_amount} -> {new_amount}")
                amount = amount if amount else 0.0
                bonus_shares_per=0.0
                split_number = scheme.get('split_number', 0.0)
                if split_number > 0:
                    bonus_shares_per = split_number
                
                records.append(DividendRecord(
                    info_date=row[0],
                    summary=row[1],
                    ex_date=ex_dividend_date,
                    pay_date=row[3] or '',
                    cdate_begin=row[4] or '',
                    cdate_end=row[5] or '',
                    currency=parse_currency(row[6]),
                    amount=amount,
                    bonus_shares_per=bonus_shares_per
                ))
            break
    return records

def get_f10_hk_dividend(symbol: str) -> List[DividendRecord]:
    """
    获取港股分红送股信息
    """
    url = TDX_URL_HK_F10
    category = F10_CONFIG['hk']['dividend']
    url = TDX_URL_TQLEX + '?Entry=CWServ.' + category['entry']
    id = int(category['dividend_info'])
    params = {
        "Params": [str(id), symbol]
    }
    body = json.dumps(params, separators=(',', ':'))
    resp = requests.post(url, headers=DEFAULT_HTTP_REQUEST_HEADERS, data=body)
    resp.raise_for_status()
    resp_json = resp.json()
    if resp_json.get('ErrorCode') != 0:
        logger.error(f"请求失败: {resp_json.get('ErrorInfo')}")
        return []
    
    return hk_parse_dividend_data(resp_json, symbol)

@dataclass
class RightsIssue:
    """ 配股/供股 信息 """
    # 基本信息
    announcement_date: str      # 公告日期
    record_date: str            # 股权登记日
    ex_date: str                # 除权日
    
    # 配股方案
    summary: str                # 配股方案说明
    ratio: float                # 配股比例 (如"10 配 3")
    # ratio_base: float=0         # 配股基数 (如 10)
    # ratio_issue: float=0        # 配股数 (如 3)
    
    # 价格
    issue_price: float=0.0      # 配股价格
    currency: str=''            # 币种
    
    # # 缴款
    # payment_start: str=''       # 缴款起始日
    # payment_end: str=''         # 缴款截止日
    
    # # 其他
    # issue_code: str=''          # 配股代码
    # listing_date: str=''        # 配股上市日
    # total_shares: int=0         # 配股总数
    # amount_raised: float=0.0    # 募集资金总额

def hk_parse_rights_issue_data(resp_json: dict) -> List[RightsIssue]:
    records = []
    for rs in resp_json.get('ResultSets', []):
        if rs.get('ResultSetKey') == 'table0':
            col_names = rs['ColName']
            _ = col_names
            for row in rs['Content']:
                issue_price=float(row[3]) if row[3] else 0.0
                #issue_price=float_round(issue_price, 4)
                records.append(RightsIssue(
                    announcement_date=row[0],  # 公告日期
                    summary=row[1],            # 配股方案说明
                    record_date=row[7] or '',  # 股权登记日
                    ex_date=row[5] or '',      # 除权日
                    ratio=float(row[2] or '0'),# 配股比例
                    issue_price=issue_price,
                    currency=parse_currency(row[4]),
                    # payment_start=row[8] or '',
                    # payment_end=row[9] or '',
                    # issue_code=row[10] or '',
                    # listing_date=row[11] or '',
                    # total_shares=int(row[12]) if row[12] else 0,
                    # amount_raised=float(row[13]) if row[13] else 0.0
                ))
    return records

def get_f10_hk_rights_issue(symbol: str) -> List[RightsIssue]:
    """
    获取港股配股/供股信息
    """
    url = TDX_URL_HK_F10
    category = F10_CONFIG['hk']['dividend']
    url = TDX_URL_TQLEX + '?Entry=CWServ.' + category['entry']
    id = int(category['rights_issue_info'])
    params = {
        "Params": [str(id), symbol]
    }
    body = json.dumps(params, separators=(',', ':'))
    resp = requests.post(url, headers=DEFAULT_HTTP_REQUEST_HEADERS, data=body)
    resp.raise_for_status()
    resp_json = resp.json()
    if resp_json.get('ErrorCode') != 0:
        logger.error(f"请求失败: {resp_json.get('ErrorInfo')}")
        return []
    
    return hk_parse_rights_issue_data(resp_json)

@dataclass
class StockSplit:
    # 基本信息
    announcement_date: str      # 公告日期
    #effective_date: str         # 生效日期
    ex_date: str                # 除权日
    
    # 重组方案
    restructuring_type: str     # 重组方式
    summary: str                # 方案说明
    # 拆合类型
    consolidation_base: float=0.0 # 合并基数
    """合并基数"""
    split_number: float=0.0     # 每股拆细数目(股)
    """每股拆细数目"""
    # split_ratio: float=0.0      # 拆分合并比例
    # """拆分合并比例"""

def hk_parse_stock_split_data(resp_json: dict) -> List[StockSplit]:
    records = []
    for rs in resp_json.get('ResultSets', []):
        if rs.get('ResultSetKey') == 'table0':
            col_names = rs['ColName']
            _ = col_names
            for row in rs['Content']:
                consolidation_base_str = row[3] # 合并基数
                split_number_str = row[4] # 每股拆细数目(股)
                #print(f'consolidation_base={consolidation_base_str}, split_number={split_number_str}')
                if consolidation_base_str:
                    consolidation_base = float(consolidation_base_str)
                else:
                    consolidation_base = 0.0
                if split_number_str:
                    split_number = float(split_number_str)
                else:
                    split_number = 0.0
                if consolidation_base == 0 and split_number == 0:
                    continue
                records.append(StockSplit(
                    announcement_date=row[0], # 公告日期
                    #effective_date=row[1],     # 生效日期
                    ex_date=row[5] or '',      # 除权日
                    restructuring_type=row[1], # 重组方式
                    summary=row[2],            # 方案说明
                    consolidation_base=consolidation_base-1 if consolidation_base > 0 else 0.0,
                    split_number=split_number-1 if split_number > 0 else 0.0,
                ))
    return records

def get_f10_hk_stock_split(symbol: str) -> List[StockSplit]:
    """
    获取港股拆分/合并信息
    """
    url = TDX_URL_HK_F10
    category = F10_CONFIG['hk']['dividend']
    url = TDX_URL_TQLEX + '?Entry=CWServ.' + category['entry']
    id = int(category['stock_split_info'])
    params = {
        "Params": [str(id), symbol]
    }
    body = json.dumps(params, separators=(',', ':'))
    resp = requests.post(url, headers=DEFAULT_HTTP_REQUEST_HEADERS, data=body)
    resp.raise_for_status()
    resp_json = resp.json()
    if resp_json.get('ErrorCode') != 0:
        logger.error(f"请求失败: {resp_json.get('ErrorInfo')}")
        return []
    
    return hk_parse_stock_split_data(resp_json)

def get_ext_xdxr_info(inst: Instrument) -> List[XdxrInfo]:
    if inst.exchange.region != Region.HK:
        return []
    code = inst.market_ticker()
    dividends = get_f10_hk_dividend(code)
    rights = get_f10_hk_rights_issue(code)
    splits = get_f10_hk_stock_split(code)

    merged :Dict[str, XdxrInfo] = {}
    for row in dividends:
        key = row.ex_date
        #logger.debug(row)
        dividend_amount = row.amount
        divident_currency = row.currency
        bonus_shares_per = row.bonus_shares_per or 0.0
        if dividend_amount > 0 and divident_currency != inst.exchange.region.currency:
            fx = ExchangeRateCache(row.currency)
            rates = fx.get_rate(row.ex_date)
            try:
                rate = rates[divident_currency]
            except:
                rate = 1.0
            dividend_amount /= rate
        if key in merged:
            existing = merged[key]
            existing.FenHong += (dividend_amount*10.0)
            existing.SongZhuanGu = bonus_shares_per * 10.0
            merged[key] = existing
        else:
            newrec = XdxrInfo()
            newrec.Date = key
            newrec.Category = 1
            newrec.FenHong = dividend_amount*10.0
            newrec.SongZhuanGu = bonus_shares_per * 10.0
            merged[key] = newrec
            logger.debug(f'{key} {dividend_amount} {bonus_shares_per}')
    for row in rights:
        key = row.ex_date
        entitlement_ratio = row.ratio
        right_amount = row.issue_price
        right_currency = row.currency
        if right_amount > 0 and right_currency != inst.exchange.region.currency:
            fx = ExchangeRateCache(row.currency)
            rates = fx.get_rate(row.ex_date)
            try:
                rate = rates[right_currency]
            except:
                rate = 1.0
            right_amount /= rate
        if key in merged:
            existing = merged[key]
            existing.PeiGu += (entitlement_ratio*10.0)
            existing.PeiGuJia += right_amount
            merged[key] = existing
        else:
            newrec = XdxrInfo()
            newrec.Date = key
            newrec.Category = 1
            
            newrec.PeiGu = (entitlement_ratio*10.0)
            newrec.PeiGuJia = right_amount
            
            merged[key] = newrec
    for row in splits:
        key = row.ex_date
        SongZhuanGu = row.split_number
        SuoGu = row.consolidation_base
        if key in merged:
            existing = merged[key]
            existing.SongZhuanGu = (SongZhuanGu-1) * 10
            existing.SuoGu = (SuoGu -1) *10 
            merged[key] = existing
        else:
            newrec = XdxrInfo()
            newrec.Date = key
            newrec.Category = 1
            newrec.SongZhuanGu = (SongZhuanGu-1) * 10
            newrec.SuoGu = (SuoGu -1) *10 
            merged[key] = newrec
    
    # # produce sorted list from merged values
    # def sort_key(r):
    #     try:
    #         return datetime.strptime(r.get('ex_date'), "%Y-%m-%d")
    #     except Exception:
    #         return datetime.min
    # sorted_list = sorted(merged.values(), key=sort_key, reverse=False)
    sorted_list = sorted(merged.values(), key=lambda x: x.Date, reverse=False)
    return sorted_list
    

if __name__ == "__main__":
    import pandas as pd
    from ...instruments import get_instrument_info
    # code = '00700'
    # #code = '01027' # 合并案例, 亚洲策略科技, 01027.hk
    # data = get_f10_hk_dividend(code)
    # df = pd.DataFrame(data)
    # print(df)
    # data = get_f10_hk_rights_issue(code)
    # df = pd.DataFrame(data)
    # print(df)
    # data = get_f10_hk_stock_split(code)
    # df = pd.DataFrame(data)
    # print(df)
    symbol = '00077.hk'
    inst = get_instrument_info(symbol)
    print(inst)
    data = get_ext_xdxr_info(inst)
    df = pd.DataFrame(data)
    print(df)