import re
from dataclasses import dataclass
from typing import Optional, List
from quant1x.log import logger
from quant1x.data.meta.forex import ExchangeRateCache
from quant1x.data.meta.region import Region

# 预编译正则表达式（与 xdxr_hkex.py 保持一致）
RATIO_PATTERN = re.compile(r'每\s*(\d+)\s*股')
SPECIAL_PATTERN = re.compile(r'特别')
BONUS_PATTERN = re.compile(r'每\s*(\d+)\s*股[^\n\r]*获发\s*(\d+)\s*股')
BONUS_SIMPLE_PATTERN = re.compile(r'(获发|派)\s*(\d+)\s*股')
MONEY_WITH_UNIT_PATTERN = re.compile(r'(港元|港币|美元|欧元|英镑|人民币)\s*(\d{1,3}(?:[,，]\d{3})*(?:\.\d+)?|\d+(?:\.\d+)?)\s*(分|仙)')
MONEY_PATTERN = re.compile(r'(\d{1,3}(?:[,，]\d{3})*(?:\.\d+)?|\d+(?:\.\d+)?)\s*(港元|港仙|港币|美元|美仙|欧元|英镑|人民币|元)')
SEPARATOR_PATTERN = re.compile(r'[\uff0c,\uff1b;]')


def parse_money(text):
    """
    从文本中提取金额和币种。
    返回 (amount, currency)，如果没有找到则返回 (None, None)
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
        amount_str = amount_str.replace(',', '').replace('，', '')

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

    # 如果没有匹配到"货币+数字+分"模式，使用原来的模式
    # 匹配数字 (整数或小数，支持千分位逗号) 后跟货币单位
    # 支持：港元、港仙、港币、美元、美仙、欧元、英镑、人民币/元
    # 注意顺序：长词优先以避免被短词（如"元"）误匹配
    matches = MONEY_PATTERN.findall(text)

    if matches:
        # 如果有多个匹配（例如既有股息又有特别股息），取第一个作为主股息
        amount_str, currency = matches[0]
        # 规范化金额字符串（去千分位分隔符）
        amount_str = amount_str.replace(',', '').replace('，', '')
        # 规范化货币词：'元' -> '人民币'
        if currency == '元':
            currency = '人民币'
        # 规范化：'港币' -> '港元'
        elif currency == '港币':
            currency = '港元'

        try:
            amount = float(amount_str)
        except ValueError:
            return amount_str, currency

        # 映射本地货币名称到 ISO 3-letter 代码，并处理港仙/美仙为小数表示
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

        # 如果原文是港仙或美仙（分），将数值除以100以得到对应货币
        if currency in ('港仙', '美仙'):
            try:
                amount = float(amount) / 100.0
            except Exception:
                pass

        return amount, currency_code

    return None, None


def parse_money_all(text):
    """
    从文本中提取所有金额和币种。
    返回列表 [(amount, currency_code, raw_currency_word, original_amount_str), ...]
    """
    if not text:
        return []

    results = []

    # 首先匹配模式: "港币23分"、"港币21.1仙" 或 "美元23分" 等
    # 模式: 货币名称 + 数字 + 分/仙
    context_matches = MONEY_WITH_UNIT_PATTERN.findall(text)

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

    for currency, amount_str, unit in context_matches:
        orig = amount_str
        amount_str = amount_str.replace(',', '').replace('，', '')
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

        currency_code = currency_map.get(currency, currency)
        results.append((amount, currency_code, currency + unit, orig))

    # 然后使用原来的模式匹配其他格式
    matches = MONEY_PATTERN.findall(text)

    for amount_str, currency_word in matches:
        orig = amount_str
        amount_str = amount_str.replace(',', '').replace('，', '')
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
        currency_code = currency_map.get(currency_word, currency_word)
        results.append((amount, currency_code, currency_word, orig))
    return results


def parse_scheme_schemes(overview):
    """
    拆分方案概述文本，识别有多少个子方案。

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

    # 如果无法拆分，返回原始文本
    return [overview]


def parse_single_scheme(scheme_text):
    """
    解析单个方案，抽象出分配方案的股数基数和金额描述两个部分。

    参数:
        scheme_text: 单个方案文本，例如 "末期股息每股4.5港元" 或 "每10股派息18.13港元"

    返回:
        dict: {
            'ratio_shares': int,  # 分配基数（每多少股）
            'amount_text': str,   # 纯金额描述文本（如 "4.5港元"）
            'has_special': bool,  # 是否特别股息
            'bonus': bool,        # 是否送股
            'bonus_desc': str     # 送股描述
        }
    """
    result = {
        'ratio_shares': 1,
        'amount_text': scheme_text,
        'has_special': False,
        'bonus': False,
        'bonus_desc': None
    }

    # 提取分配基数（每x股）
    match_ratio = RATIO_PATTERN.search(scheme_text)
    if match_ratio:
        try:
            result['ratio_shares'] = int(match_ratio.group(1))
        except ValueError:
            pass

    # 提取金额文本（数字+货币单位）
    # 优先匹配"货币+数字+分/仙"格式
    match = MONEY_WITH_UNIT_PATTERN.search(scheme_text)
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
        result['bonus_desc'] = f"每{m.group(1)}股获发{m.group(2)}股"
    else:
        m2 = BONUS_SIMPLE_PATTERN.search(scheme_text)
        if m2:
            result['bonus'] = True
            result['bonus_desc'] = m2.group(0)

    return result


def parse_scheme_info(date:str, overview, preferred_currency:str=Region.HK.currency):
    """
    解析方案概述文本的所有信息。

    参数:
        overview: 方案概述文本
        preferred_currency: 首选货币代码

    返回:
        dict: 包含所有解析字段的字典
    """
    # 拆分子方案（可能包含多个货币方案）
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

    # 收集所有方案的结构信息（特别股息、送股、股数基数等）
    has_special = any(ps['scheme']['has_special'] for ps in parsed_schemes)
    bonus = any(ps['scheme']['bonus'] for ps in parsed_schemes)
    bonus_desc_list = [ps['scheme']['bonus_desc'] for ps in parsed_schemes if ps['scheme']['bonus_desc']]
    bonus_desc = '; '.join(bonus_desc_list) if bonus_desc_list else None

    # 统一 ratio_shares（取最大公约数，通常是1）
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
        'dividend_currency': None
    }
    fx = ExchangeRateCache(preferred_currency)
    rates = fx.get_rate(date=date)
    # 收集所有金额组件（每个组件保留原始方案文本，并统一 ratio_shares）
    for i, ps in enumerate(parsed_schemes):
        if ps['amount'] is not None and ps['currency']:
            # 根据 ratio_shares 转换金额到每股
            scheme_ratio = ps['scheme']['ratio_shares']
            if scheme_ratio > 0 and scheme_ratio != ratio_shares:
                # 转换到统一基数：每 ratio_shares 股派息金额
                per_share_amount = ps['amount'] * ratio_shares / scheme_ratio
            else:
                per_share_amount = ps['amount']
            
            scheme_currency = ps['currency']
            if preferred_currency != scheme_currency:
                try:
                    rate = rates[scheme_currency]
                except:
                    logger.error(f"无法获取汇率: {scheme_currency}, 回退到1.0")
                    rate = 1.0
                per_share_amount /= rate
                # 转换为首选货币
                scheme_currency = preferred_currency
                
            result['dividend_components'].append({
                'amount': per_share_amount,
                'currency': scheme_currency,
                'currency_word': ps['currency'],
                'raw': str(ps['amount']),
                'source_text': schemes[i],  # 保留拆分后的方案文本
                'original_ratio': scheme_ratio  # 原始股数基数
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
    text = "中期股息每股 9.18 港仙，一千股派 108 股盈大地产（432）(相当于每股派息 0.1922 港元)"
    text = '年度股息每股普通股17.04港仙，可以股代息'
    text = '年度股息每股0.123港元，每10股派人民币1元'
    text = '第一中期股息每股派美元0.133元(可选择以股代息)'
    info = parse_scheme_info('1999-08-16', text)
    print(info)
