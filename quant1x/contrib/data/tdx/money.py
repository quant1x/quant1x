# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import re
from dataclasses import dataclass
from typing import Optional, List
from quant1x.log import logger
from quant1x.data.meta.forex import ExchangeRateCache
from quant1x.data.meta.region import Region

# 预编译正则表达式(与 xdxr_hkex.py 保持一致)
RATIO_PATTERN = re.compile(r'每\s*(\d+)\s*股')
SPECIAL_PATTERN = re.compile(r'特别')
BONUS_PATTERN = re.compile(r'每\s*(\d+)\s*股[^\n\r]*(?:送|派|获发|送转)\s*(\d+)\s*股')
BONUS_SIMPLE_PATTERN = re.compile(r'(获发|派)\s*(\d+)\s*股')
MONEY_WITH_UNIT_PATTERN = re.compile(r'(港元|港币|美元|欧元|英镑|人民币)\s*(\d{1,3}(?:[,, ]\d{3})*(?:\.\d+)?|\d+(?:\.\d+)?)\s*(分|仙)')
MONEY_PATTERN = re.compile(r'(\d{1,3}(?:[,, ]\d{3})*(?:\.\d+)?|\d+(?:\.\d+)?)\s*(港元|港仙|港币|美元|美仙|欧元|英镑|人民币|元)')
SEPARATOR_PATTERN = re.compile(r'[\uff0c,\uff1b;]')
PREFIX_CURRENCY_PATTERN = re.compile(r'(港元|港币|美元|欧元|英镑|人民币)\s*(\d{1,3}(?:[,, ]\d{3})*(?:\.\d+)?|\d+(?:\.\d+)?)')

# 货币映射表
CURRENCY_MAP = {
    '港元': 'HKD',
    '港仙': 'HKD',
    '港币': 'HKD',
    '人民币': 'CNY',
    '美元': 'USD',
    '美仙': 'USD',
    '欧元': 'EUR',
    '英镑': 'GBP',
}


def parse_money(text):
    """
    从文本中提取金额和币种. 
    返回 (amount, currency), 如果没有找到则返回 (None, None)
    """
    if not text:
        return None, None

    # 首先匹配模式: "港币23分" 或 "美元23分" 或 "港币21.1仙" 等
    # 模式: 货币名称 + 数字 + 分/仙
    match = MONEY_WITH_UNIT_PATTERN.search(text)

    if match:
        currency = match.group(1)
        amount_str = match.group(2)
        unit = match.group(3)  # 分或仙
        amount_str = amount_str.replace(',', '').replace(', ', '')

        # 规范化货币词
        if currency == '港币':
            currency = '港元'
        elif currency == '人民币':
            currency = '人民币'

        try:
            amount = float(amount_str)
        except ValueError:
            return amount_str, currency

        # 映射本地货币名称到 ISO 3-letter 代码
        currency_map = {
            '港元': 'HKD',
            '港仙': 'HKD',
            '港币': 'HKD',
            '人民币': 'CNY',
            '美元': 'USD',
            '美仙': 'USD',
            '欧元': 'EUR',
            '英镑': 'GBP',
        }

        currency_code = currency_map.get(currency, currency)

        # 分/仙需要除以100
        try:
            amount = amount / 100.0
        except Exception:
            pass

        return amount, currency_code

    # 如果没有匹配到"货币+数字+分"模式, 使用原来的模式
    # 匹配数字 (整数或小数, 支持千分位逗号) 后跟货币单位
    # 支持: 港元, 港仙, 港币, 美元, 美仙, 欧元, 英镑, 人民币/元
    # 注意顺序: 长词优先以避免被短词(如"元")误匹配

    # 先尝试匹配"前缀货币词+数字"的情况, 例如"美元0.133"(不管后面有没有单位词)
    # 这种情况下, 货币由前缀货币词决定, 忽略后面的单位词(如"元", "cents"等)
    prefix_match = PREFIX_CURRENCY_PATTERN.search(text)
    if prefix_match:
        currency = prefix_match.group(1)
        amount_str = prefix_match.group(2)
        amount_str = amount_str.replace(',', '').replace(', ', '')
        # 规范化货币词: '港币' -> '港元'
        if currency == '港币':
            currency = '港元'

        try:
            amount = float(amount_str)
        except ValueError:
            return amount_str, currency

        currency_code = CURRENCY_MAP.get(currency, currency)
        return amount, currency_code

    matches = MONEY_PATTERN.findall(text)

    if matches:
        # 如果有多个匹配(例如既有股息又有特别股息), 取第一个作为主股息
        amount_str, currency = matches[0]
        # 规范化金额字符串(去千分位分隔符)
        amount_str = amount_str.replace(',', '').replace(', ', '')
        # 规范化货币词: '元' -> '人民币'
        if currency == '元':
            currency = '人民币'
        # 规范化: '港币' -> '港元'
        elif currency == '港币':
            currency = '港元'

        try:
            amount = float(amount_str)
        except ValueError:
            return amount_str, currency

        # 映射本地货币名称到 ISO 3-letter 代码, 并处理港仙/美仙为小数表示
        currency_code = CURRENCY_MAP.get(currency, currency)

        # 如果原文是港仙或美仙(分), 将数值除以100以得到对应货币
        if currency in ('港仙', '美仙'):
            try:
                amount = float(amount) / 100.0
            except Exception:
                pass

        return amount, currency_code

    return None, None


def parse_money_all(text):
    """
    从文本中提取所有金额和币种. 
    返回列表 [(amount, currency_code, raw_currency_word, original_amount_str), ...]
    """
    if not text:
        return []

    results = []

    # 首先匹配模式: "港币23分", "港币21.1仙" 或 "美元23分" 等
    # 模式: 货币名称 + 数字 + 分/仙
    context_matches = MONEY_WITH_UNIT_PATTERN.findall(text)

    for currency, amount_str, unit in context_matches:
        orig = amount_str
        amount_str = amount_str.replace(',', '').replace(', ', '')
        # 规范化货币词
        if currency == '港币':
            currency = '港元'

        try:
            amount = float(amount_str)
        except Exception:
            amount = None

        # 分/仙需要除以100
        if isinstance(amount, (int, float)):
            try:
                amount = amount / 100.0
            except Exception:
                pass

        currency_code = CURRENCY_MAP.get(currency, currency)
        results.append((amount, currency_code, currency + unit, orig))

    # 先尝试匹配"前缀货币词+数字"的情况, 例如"美元0.133"(不管后面有没有单位词)
    prefix_matches = PREFIX_CURRENCY_PATTERN.findall(text)

    for currency_word, amount_str in prefix_matches:
        orig = amount_str
        amount_str = amount_str.replace(',', '').replace(', ', '')
        if currency_word == '港币':
            currency_word = '港元'
        try:
            amount = float(amount_str)
        except Exception:
            amount = None
        currency_code = CURRENCY_MAP.get(currency_word, currency_word)
        results.append((amount, currency_code, currency_word + '元', orig))

    # 然后使用原来的模式匹配其他格式
    matches = MONEY_PATTERN.findall(text)

    for amount_str, currency_word in matches:
        orig = amount_str
        amount_str = amount_str.replace(',', '').replace(', ', '')
        if currency_word == '元':
            currency_word = '人民币'
        elif currency_word == '港币':
            currency_word = '港元'
        try:
            amount = float(amount_str)
        except Exception:
            amount = None
        # 港仙/美仙转换为对应货币
        if currency_word in ('港仙', '美仙') and isinstance(amount, (int, float)):
            try:
                amount = float(amount) / 100.0
            except Exception:
                pass
        currency_code = CURRENCY_MAP.get(currency_word, currency_word)
        results.append((amount, currency_code, currency_word, orig))
    return results


def parse_scheme_schemes(overview):
    """
    拆分方案概述文本, 识别有多少个子方案. 

    参数:
        overview: 方案概述文本

    返回:
        list: 子方案文本列表
    """
    overview = overview.replace(' ', '').replace('\u3000', '')
    # 用正则一次性匹配所有分隔符拆分
    schemes = SEPARATOR_PATTERN.split(overview)
    schemes = [s.strip() for s in schemes if s.strip()]

    if len(schemes) > 1:
        return schemes

    # 如果无法拆分, 返回原始文本
    return [overview]


def parse_single_scheme(scheme_text):
    """
    解析单个方案, 抽象出分配方案的股数基数和金额描述两个部分. 

    参数:
        scheme_text: 单个方案文本, 例如 "末期股息每股4.5港元" 或 "每10股派息18.13港元"

    返回:
        dict: {
            'ratio_shares': int,  # 分配基数(每多少股)
            'amount_text': str,   # 纯金额描述文本(如 "4.5港元")
            'has_special': bool,  # 是否特别股息
            'bonus': bool,        # 是否送股
            'bonus_desc': str,    # 送股描述
            'split_number': float # 每股送股数(如 "每10股送1股" -> 0.1)
        }
    """
    result = {
        'ratio_shares': 1,
        'amount_text': scheme_text,
        'has_special': False,
        'bonus': False,
        'bonus_desc': None,
        'split_number': 0.0
    }

    # 提取分配基数(每x股)
    match_ratio = RATIO_PATTERN.search(scheme_text)
    if match_ratio:
        try:
            result['ratio_shares'] = int(match_ratio.group(1))
        except ValueError:
            pass

    # 提取金额文本(数字+货币单位)
    # 优先匹配"货币+数字+分/仙"格式
    match = MONEY_WITH_UNIT_PATTERN.search(scheme_text)
    if not match:
        # 匹配"前缀货币词+数字"格式, 如"美元0.133"(不管后面有没有单位词)
        match = PREFIX_CURRENCY_PATTERN.search(scheme_text)
    if not match:
        # 匹配"数字+货币"格式
        match = MONEY_PATTERN.search(scheme_text)

    if match:
        result['amount_text'] = match.group(0)

    # 标记特别股息
    if SPECIAL_PATTERN.search(scheme_text):
        result['has_special'] = True

    # 检测送股/派股信息
    m = BONUS_PATTERN.search(scheme_text)
    if m:
        result['bonus'] = True
        result['bonus_desc'] = f"每{m.group(1)}股送{m.group(2)}股"
        # split_number 是基于 ratio_shares 的送股数(不是每股送股数)
        try:
            ratio_shares = int(m.group(1))
            bonus_shares = int(m.group(2))
            result['split_number'] = bonus_shares
        except ValueError:
            pass
    else:
        m2 = BONUS_SIMPLE_PATTERN.search(scheme_text)
        if m2:
            result['bonus'] = True
            result['bonus_desc'] = m2.group(0)
            # 简单模式: 如"派1股", 假设基于 ratio_shares 送1股
            try:
                result['split_number'] = float(m2.group(2))
            except ValueError:
                pass

    return result


def parse_scheme_info(date:str, overview, preferred_currency:str=Region.HK.currency):
    """
    解析方案概述文本的所有信息. 

    参数:
        overview: 方案概述文本
        preferred_currency: 首选货币代码

    返回:
        dict: 包含所有解析字段的字典
    """
    # 拆分子方案(可能包含多个货币方案)
    schemes = parse_scheme_schemes(overview)

    # 解析所有子方案
    parsed_schemes = []
    for scheme_text in schemes:
        scheme = parse_single_scheme(scheme_text)
        dividend_amount, dividend_currency = parse_money(scheme['amount_text'])
        parsed_schemes.append({
            'scheme': scheme,
            'amount': dividend_amount,
            'currency': dividend_currency
        })

    # 收集所有方案的结构信息(特别股息, 送股, 股数基数等)
    has_special = any(ps['scheme']['has_special'] for ps in parsed_schemes)
    bonus = any(ps['scheme']['bonus'] for ps in parsed_schemes)
    bonus_desc_list = [ps['scheme']['bonus_desc'] for ps in parsed_schemes if ps['scheme']['bonus_desc']]
    bonus_desc = '; '.join(bonus_desc_list) if bonus_desc_list else None

    # 统一 ratio_shares(取最大公约数, 通常是1)
    ratio_shares_list = [ps['scheme']['ratio_shares'] for ps in parsed_schemes if ps['scheme']['ratio_shares'] > 0]
    ratio_shares = min(ratio_shares_list) if ratio_shares_list else 1

    result = {
        'overview': overview,  # 方案原文
        'schemes': schemes,    # 拆分后的子方案列表
        'has_special': has_special,
        'ratio_shares': ratio_shares,
        'bonus': bonus,
        'bonus_desc': bonus_desc,
        'dividend_components': [],
        'dividend_amount': None,
        'dividend_currency': None,
        'split_number': 0.0  # 每股送股总数(所有子方案的和)
    }
    fx = ExchangeRateCache(preferred_currency)
    rates = fx.get_rate(date=date)

    # 检查是否为"可选择货币"的互斥方案
    # 只有当"可选择"后面跟着货币时才是互斥方案(如"可选择货币0.777678港元")
    # 而"可选择以股代息"不是互斥的货币方案
    alternative_schemes_indices = []
    for i, scheme in enumerate(schemes):
        if '可选择' in scheme:
            # 先排除"可选择以股代息"等非货币选项
            if '以股代息' in scheme or '以股息' in scheme or '股份' in scheme:
                continue
            # 检查"可选择"后是否跟着"货币"或"派", 或者直接跟着货币词
            if re.search(r'可选择\s*(?:货币|派|(?:\d+(?:\.\d+)?\s*(?:港元|港币|美元|欧元|英镑|人民币)))', scheme):
                alternative_schemes_indices.append(i)

    # 如果存在互斥方案, 选择匹配首选货币的方案(优先主方案, 其次互斥方案)
    has_alternative = len(alternative_schemes_indices) > 0

    # 收集所有金额组件(每个组件保留原始方案文本, 并统一 ratio_shares)
    for i, ps in enumerate(parsed_schemes):
        # 如果存在互斥方案, 需要只选择一个
        skip_scheme = False
        if has_alternative:
            # 如果当前方案在互斥方案列表中
            if i in alternative_schemes_indices:
                # 检查是否匹配首选货币
                scheme_text = schemes[i]
                preferred_currency_names = [k for k, v in CURRENCY_MAP.items() if v == preferred_currency]

                matches_preferred = False
                for curr_name in preferred_currency_names:
                    if curr_name in scheme_text:
                        matches_preferred = True
                        break

                # 如果不匹配首选货币, 跳过这个方案
                if not matches_preferred:
                    skip_scheme = True
            else:
                # 当前方案不是互斥方案, 检查是否有互斥方案
                # 如果有互斥方案, 只使用主方案当且仅当主方案匹配首选货币
                if alternative_schemes_indices:
                    # 检查是否有互斥方案匹配首选货币
                    any_alternative_matches = False
                    for alt_idx in alternative_schemes_indices:
                        alt_scheme = schemes[alt_idx]
                        preferred_currency_names = [k for k, v in CURRENCY_MAP.items() if v == preferred_currency]
                        for curr_name in preferred_currency_names:
                            if curr_name in alt_scheme:
                                any_alternative_matches = True
                                break
                        if any_alternative_matches:
                            break

                    # 如果有互斥方案匹配首选货币, 跳过主方案(使用互斥方案)
                    if any_alternative_matches:
                        skip_scheme = True

        if skip_scheme:
            continue

        # 计算每股送股数(统一转换为每股送股数)
        split_number = ps['scheme']['split_number']
        scheme_ratio = ps['scheme']['ratio_shares']
        if scheme_ratio > 0:
            per_share_split_number = split_number / scheme_ratio
        else:
            per_share_split_number = 0.0
        result['split_number'] += per_share_split_number

        # 确定金额和货币
        if ps['amount'] is not None and ps['currency']:
            # 现金分红方案
            scheme_ratio = ps['scheme']['ratio_shares']
            if scheme_ratio > 0 and scheme_ratio != ratio_shares:
                # 转换到统一基数: 每 ratio_shares 股派息金额
                per_share_amount = ps['amount'] * ratio_shares / scheme_ratio
            else:
                per_share_amount = ps['amount']

            scheme_currency = ps['currency']
            if preferred_currency != scheme_currency:
                try:
                    rate = rates[scheme_currency] * (1-0.005)
                except:
                    logger.error(f"无法获取汇率: {scheme_currency}, 回退到1.0")
                    rate = 1.0
                logger.debug(f"{date}, 汇率: {scheme_currency} -> {preferred_currency}: {rate}")
                per_share_amount /= rate
                # 转换为首选货币
                scheme_currency = preferred_currency

            result['dividend_components'].append({
                'amount': per_share_amount,
                'currency': scheme_currency,
                'currency_word': ps['currency'],
                'raw': str(ps['amount']),
                'source_text': schemes[i],  # 保留拆分后的方案文本
                'original_ratio': scheme_ratio,  # 原始股数基数
                'split_number': split_number  # 该子方案的每股送股数
            })
        elif split_number > 0:
            # 送转股方案(无金额)
            result['dividend_components'].append({
                'amount': 0.0,
                'currency': preferred_currency,
                'currency_word': None,
                'raw': None,
                'source_text': schemes[i],  # 保留拆分后的方案文本
                'original_ratio': ps['scheme']['ratio_shares'],  # 原始股数基数
                'split_number': split_number  # 该子方案的每股送股数
            })

    # 累加所有组件的金额
    total_amount = 0.0
    for comp in result['dividend_components']:
        if comp['amount'] is not None:
            total_amount += comp['amount']

    if result['dividend_components']:
        result['dividend_amount'] = total_amount
        result['dividend_currency'] = result['dividend_components'][0]['currency']

    return result


if __name__ == '__main__':
    text = "中期股息每股 9.18 港仙, 一千股派 108 股盈大地产(432)(相当于每股派息 0.1922 港元)"
    text = '年度股息每股普通股17.04港仙, 可以股代息'
    text = '年度股息每股0.123港元, 每10股派人民币1元'
    text = '第一中期股息每股派美元0.133元(可选择以股代息)'
    text = '年度股息12港仙, 每10股送1股'
    info = parse_scheme_info('1999-08-02', text)
    print(info)
