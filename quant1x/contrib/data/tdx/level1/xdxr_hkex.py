import re
import json
import datetime

# 预编译正则表达式
PAREN_PATTERN = re.compile(r'\(([^)]*)\)')
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

def parse_table_rows(section_text):
    """
    解析 ASCII 表格文本，处理多行合并的情况，返回单元格列表的列表
    """
    lines = section_text.strip().split('\n')
    rows = []
    current_row = None
    
    for line in lines:
        # 跳过分割线 (包含 ─, ┬, ┴, ├, ┼, ┤, ┌, ┐, └, ┘)
        if any(char in line for char in ['─', '┬', '┴', '├', '┼', '┤', '┌', '┐', '└', '┘']):
            continue
        
        # 只处理包含数据行的符号 │
        if '│' not in line:
            continue
            
        # 分割单元格，去掉首尾空字符串
        cells = [c.strip() for c in line.split('│')]
        # 去掉首尾因为分割产生的空元素
        if cells and cells[0] == '': cells.pop(0)
        if cells and cells[-1] == '': cells.pop(-1)
        
        # 检查是否是续行 (第一列为空)
        # 在分红派息表中，第一列是公告日期。如果第一列为空，说明是上一行的续行
        if len(cells) > 1 and not cells[0]:
            if current_row:
                # 将第二列 (方案概述) 的内容追加到上一行
                # 注意：不同表格列数可能不同，这里假设续行主要是为了补充长文本
                # 找到非空的单元格追加到对应位置，通常是在第 2 列 (index 1)
                for i, cell in enumerate(cells):
                    if cell:
                        if i < len(current_row):
                            current_row[i] += "" + cell
                        else:
                            current_row.append(cell)
            continue
        
        # 新行
        if cells:
            # 只有当行里有实际内容时才保存 (避免空行)
            if any(c for c in cells):
                if current_row:
                    rows.append(current_row)
                current_row = cells
    
    if current_row:
        rows.append(current_row)
        
    return rows

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


def parse_scheme_info(overview, preferred_currency=None):
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

    # 使用第一个方案的结构信息（特别股息、送股、股数基数等）
    first_scheme = parsed_schemes[0]['scheme']

    result = {
        'preferred_text': schemes[0] if schemes[0] != overview else None,
        'parse_target': schemes[0],
        'has_special': first_scheme['has_special'],
        'ratio_shares': first_scheme['ratio_shares'],
        'bonus': first_scheme['bonus'],
        'bonus_desc': first_scheme['bonus_desc'],
        'dividend_components': [],
        'dividend_amount': None,
        'dividend_currency': None,
        'dividend_selection': None,
        'preferred_used': None
    }

    # 收集所有金额组件
    for ps in parsed_schemes:
        if ps['amount'] is not None and ps['currency']:
            result['dividend_components'].append({
                'amount': ps['amount'],
                'currency': ps['currency'],
                'currency_word': ps['currency'],
                'raw': str(ps['amount'])
            })

    # 如果有多个货币方案，根据 preferred_currency 选择
    if len(parsed_schemes) > 1:
        chosen = None
        preferred_upper = preferred_currency.upper() if isinstance(preferred_currency, str) else None

        if preferred_upper:
            matches = [ps for ps in parsed_schemes if ps['currency'] == preferred_upper]
            if matches:
                chosen = matches[-1]
                result['dividend_selection'] = 'preferred_currency_requested'
                result['preferred_used'] = preferred_upper

        if not chosen:
            chosen = parsed_schemes[-1]
            result['dividend_selection'] = 'last_scheme_preferred'
            result['preferred_used'] = None

        result['dividend_amount'] = chosen['amount']
        result['dividend_currency'] = chosen['currency']
    else:
        # 只有一个方案，直接使用
        result['dividend_amount'] = parsed_schemes[0]['amount']
        result['dividend_currency'] = parsed_schemes[0]['currency']
        result['dividend_selection'] = 'single_scheme'

    return result

def parse_date_safe(date_str):
    """尝试将 YYYY-MM-DD 格式字符串解析为 date 对象，失败返回 None"""
    if not date_str or date_str.strip() in ('---', ''):
        return None
    try:
        return datetime.datetime.strptime(date_str.strip(), "%Y-%m-%d").date()
    except Exception:
        # 有时日期可能是其他格式，尝试常见替代
        try:
            return datetime.datetime.strptime(date_str.strip(), "%Y/%m/%d").date()
        except Exception:
            return None


def parse_dividends(text, preferred_currency=None):
    """解析分红派息部分"""
    # 定位 section
    start_marker = "【1.分红派息】"
    end_marker = "【2.供股】"
    
    # 可能在文首有一行索引性的摘要（例如"本栏包括【1.分红派息】【2.供股】..."），
    # 因此优先选取第二个出现的章节标题作为实际内容起始点
    start_idx = text.find(start_marker)
    if start_idx == -1:
        return []
    # 如果同一文档中该标记出现多次，取后一处出现作为真实章节开始
    next_start = text.find(start_marker, start_idx + 1)
    if next_start != -1:
        start_idx = next_start

    end_idx = text.find(end_marker, start_idx)

    section_content = text[start_idx + len(start_marker):end_idx] if end_idx != -1 else text[start_idx + len(start_marker):]
    
    rows = parse_table_rows(section_content)
    results = []

    # 表头：公告日期，方案概述，截止日期，除净日，派付日，暂停过户起，暂停过户止
    # 索引：0, 1, 2, 3, 4, 5, 6

    for row in rows:
        if len(row) < 7:
            continue

        announce = row[0].strip()
        # 跳过表头行（如表格中重复出现"公告日期"等）
        if '公告日期' in announce or announce.startswith('公告'):
            continue
        overview = row[1]

        # 解析方案信息
        info = parse_scheme_info(overview, preferred_currency)

        # 计算单股分红金额
        dividend_amount_per_share = None
        if info['ratio_shares'] and isinstance(info['dividend_amount'], (int, float)):
            dividend_amount_per_share = info['dividend_amount'] / info['ratio_shares']
        record = {
            "category": "分红派息",
            "announce_date": announce,
            "scheme_overview": overview,
            "scheme_parse_source": info['parse_target'],
            "scheme_preferred_from_parentheses": True if info['preferred_text'] else False,
            "dividend_components": info['dividend_components'],
            "has_special_dividend": info['has_special'],
            "dividend_selection": info['dividend_selection'],
            "preferred_currency_requested": preferred_currency if preferred_currency else None,
            "preferred_currency_used": info['preferred_used'],
            "record_date": row[2].strip() if len(row) > 2 else None,
            "ex_date": row[3].strip() if len(row) > 3 else None,
            "payment_date": row[4].strip() if len(row) > 4 else None,
            "closure_start": row[5].strip() if len(row) > 5 else None,
            "closure_end": row[6].strip() if len(row) > 6 else None,
            "dividend_amount": info['dividend_amount'],
            "dividend_currency": info['dividend_currency'],
            "ratio_shares": info['ratio_shares'],
            "dividend_amount_per_share": dividend_amount_per_share,
            "bonus": info['bonus'],
            "bonus_description": info['bonus_desc']
        }

        # # 解析 announce_date 为 date 对象，便于排序
        # parsed = parse_date_safe(announce)
        # if parsed:
        #     record["announce_date_parsed"] = parsed.isoformat()

        results.append(record)

    return results

def parse_rights(text, preferred_currency=None):
    """解析供股部分"""
    start_marker = "【2.供股】"
    end_marker = "【3.拆分合并】"
    
    start_idx = text.find(start_marker)
    if start_idx == -1:
        return []
    next_start = text.find(start_marker, start_idx + 1)
    if next_start != -1:
        start_idx = next_start
    end_idx = text.find(end_marker, start_idx)
    if start_idx == -1:
        return []
    section_content = text[start_idx + len(start_marker):end_idx] if end_idx != -1 else text[start_idx + len(start_marker):]
    rows = parse_table_rows(section_content)
    #print(rows)
    results = []
    # 表头: 公告日期, 方案概述, 每股供股(股), 供股价格(元), 供股价格币种, 除净日, 派付日, 暂停过户起, 暂停过户止
    # 索引: 0, 1, 2, 3, 4, 5, 6, 7, 8
    for row in rows:
        if len(row) < 9:
            continue
        # 跳过可能的表头行
        if '公告日期' in row[0] or row[0].strip().startswith('公告'):
            continue
        price_, currency_= parse_money(row[1])
        if not currency_:
            price_, currency_ = parse_money(row[4])
        currency = currency_ if currency_ else ''
        record = {
            # 1. 基础信息
            "category": "供股", # 供股, rights issue
            "announce_date": row[0], # 公告日期
            "scheme_overview": row[1], # 方案概述
            # 2. 供股核心参数 (修正了严重的语义错误)
            "entitlement_ratio": row[2], # 每股供股(股)
            "subscription_price": row[3], # 供股价格(元)
            "price_currency": currency, # 供股价格币种
            # 3. 关键时间节点
            "ex_date": row[5], # 除净日
            "payment_date": row[6].strip() if len(row) > 6 else None, # 支付日
            "closure_start": row[7].strip() if len(row) > 7 else None, # 暂停过户起
            "closure_end": row[8].strip() if len(row) > 8 else None, # 暂停过户止
            
        }
        results.append(record)
        
    return results

def parse_splits(text, preferred_currency=None):
    """解析拆分合并部分"""
    start_marker = "【3.拆分合并】"
    # 结束标记可以是免责条款或文本结束
    end_marker = "〖免责条款〗"
    
    start_idx = text.find(start_marker)
    if start_idx == -1:
        return []
    next_start = text.find(start_marker, start_idx + 1)
    if next_start != -1:
        start_idx = next_start
    end_idx = text.find(end_marker, start_idx)
    if start_idx == -1:
        return []
    section_content = text[start_idx + len(start_marker):end_idx] if end_idx != -1 else text[start_idx + len(start_marker):]
    
    rows = parse_table_rows(section_content)
    results = []
    
    # 表头：公告日期，重组方式，方案概述，一股合并基数，每股拆细 (股), 除净日，换领股票起，换领股票止，变更说明
    # 索引：0, 1, 2, 3, 4, 5, 6, 7, 8
    
    for row in rows:
        if len(row) < 9:
            continue
        # 跳过可能的表头行
        if '公告日期' in row[0] or row[0].strip().startswith('公告'):
            continue
        record = {
            "category": "拆分合并", # 拆分合并
            "announce_date": row[0], # 公告日期
            "restructuring_type": row[1], # 重组方式
            "scheme_overview": row[2], # 方案概述
            "consolidation_base": row[3], # 一股合并基数
            "split_ratio": row[4], # 每股拆细 (股)
            "ex_date": row[5], # 除净日
            "cert_exchange_start": row[6], # 换领股票起
            "cert_exchange_end": row[7], # 换领股票止
            "change_description": row[8] # 变更说明
        }
        results.append(record)
        
    return results

def parse_text_to_list(text, preferred_currency=None):
    """统一返回一个记录列表（分红/送股/拆分合并等均包含），并按公告日期升序排序（最早在前）。
    支持传入字符串或字符串列表（会将列表以换行符连接）。"""
    if isinstance(text, list):
        normalized = []
        for item in text:
            if isinstance(item, str):
                normalized.append(item)
            elif isinstance(item, dict):
                # 常见字段名可能包含 'text' 或 'reply'
                if 'text' in item and isinstance(item['text'], str):
                    normalized.append(item['text'])
                elif 'reply' in item and isinstance(item['reply'], str):
                    normalized.append(item['reply'])
                else:
                    normalized.append(json.dumps(item, ensure_ascii=False))
            else:
                normalized.append(str(item))
        text = "\n".join(normalized)

    dividends = parse_dividends(text, preferred_currency=preferred_currency)
    rights = parse_rights(text, preferred_currency=preferred_currency)
    splits = parse_splits(text, preferred_currency=preferred_currency)

    # 合并所有记录
    combined = []
    combined.extend(dividends)
    combined.extend(rights)
    combined.extend(splits)

    # merge records sharing the same announce_date into one dict
    merged = {}
    for rec in combined:
        key = rec.get('ex_date')
        print(key)
        if key in merged:
            existing = merged[key]
            # maintain list of categories
            cats = existing.get('categories', [existing.get('category')])
            if rec.get('category') not in cats:
                cats.append(rec.get('category'))
            existing['categories'] = cats
            # merge other fields, preferring non-null/non-empty values
            for k, v in rec.items():
                if k in ('ex_date', 'category'):
                    continue
                if v is None or v == '' or v == '---':
                    continue
                if k not in existing or existing[k] is None or existing[k] == '':
                    existing[k] = v
                elif existing[k] != v and k not in ('categories'):
                    # if conflict, convert to list
                    if not isinstance(existing[k], list):
                        existing[k] = [existing[k]]
                    if v not in existing[k]:
                        existing[k].append(v)
        else:
            newrec = rec.copy()
            newrec['categories'] = [rec.get('category')]
            merged[key] = newrec

    # produce sorted list from merged values
    def sort_key(r):
        try:
            return datetime.datetime.strptime(r.get('ex_date'), "%Y-%m-%d")
        except Exception:
            return datetime.datetime.min
    sorted_list = sorted(merged.values(), key=sort_key, reverse=False)
    return sorted_list

def main():
    # 原始文本
    raw_text = """分红送股☆ ◇00700 腾讯控股 更新日期：2026-02-27◇ 通达信港股F10
★本栏包括【1.分红派息】【2.供股】【3.拆分合并】

【1.分红派息】
┌─────┬─────────────────────┬─────┬─────┬─────────┬─────┬─────┐
│公告日期  │方案概述                                  │截止日期  │除净日    │派付日            │暂停过户起│暂停过户止│
├─────┼─────────────────────┼─────┼─────┼─────────┼─────┼─────┤
│2025-03-19│末期股息每股4.5港元                       │2024-12-31│2025-05-16│现金：2025-05-30  │2025-05-20│2025-05-21│
│2024-03-20│末期股息每股3.4港元                       │2023-12-31│2024-05-17│现金：2024-05-31  │2024-05-21│2024-05-22│
│2023-03-22│末期股息每股2.4港元                       │2022-12-31│2023-05-19│现金：2023-06-05  │2023-05-23│2023-05-24│
│2022-11-16│每10股股份获发1股美团B类普通股(相当于每股 │---       │2023-01-05│---               │2023-01-09│2023-01-10│
│          │派息18.13港元)                            │          │          │                  │          │          │
│2022-03-23│末期股息每股160港仙                       │2021-12-31│2022-05-20│现金：2022-06-06  │2022-05-24│2022-05-25│
│2021-12-23│21股（00700）派1股京东集团-SW（09618）A类 │---       │2022-01-20│---               │2022-01-24│2022-01-25│
│          │普通股(相当于每股派息13.4港元)            │          │          │                  │          │          │
│2021-03-24│末期股息每股160港仙                       │2020-12-31│2021-05-24│现金：2021-06-07  │2021-05-26│2021-05-27│
│2020-03-18│末期股息每股1.20港元                      │2019-12-31│2020-05-15│现金：2020-05-29  │2020-05-19│2020-05-20│
│2019-03-21│末期股息每股1.00港元                      │2018-12-31│2019-05-17│现金：2019-05-31  │2019-05-21│2019-05-22│
│2018-03-21│末期股息每股0.88港元                      │2017-12-31│2018-05-18│现金：2018-06-01  │2018-05-23│2018-05-24│
│2017-03-22│末期股息每股0.61港元                      │2016-12-31│2017-05-19│现金：2017-06-02  │2017-05-23│2017-05-24│
│2016-03-17│年度股息每股47港仙                        │2015-12-31│2016-05-20│现金：2016-06-02  │2016-05-24│2016-05-25│
│2015-03-18│年度股息每股36港仙                        │2014-12-31│2015-05-15│现金：2015-05-29  │2015-05-19│2015-05-20│
│2014-03-19│年度股息1.2港元                           │2013-12-31│2014-05-15│现金：2014-05-29  │2014-05-19│2014-05-20│
│2013-03-20│年度股息1港元                             │2012-12-31│2013-05-20│现金：2013-05-30  │2013-05-22│2013-05-23│
│2012-03-14│年度股息0.75港元                          │2011-12-31│2012-05-18│现金：2012-05-30  │2012-05-22│2012-05-23│
│2011-03-16│年度股息0.55港元                          │2010-12-31│2011-05-03│现金：2011-05-25  │2011-05-05│2011-05-11│
│2010-03-17│年度股息0.4港元                           │2009-12-31│2010-05-05│现金：2010-05-26  │2010-05-07│2010-05-12│
│2009-03-18│年度股息0.25港元，特别股息0.1港元         │2008-12-31│2009-05-06│现金：2009-05-27  │2009-05-08│2009-05-13│
│2008-03-19│年度股息0.16港元                          │2007-12-31│2008-05-06│现金：2008-05-28  │2008-05-08│2008-05-14│
│2007-03-21│末期股息0.12港元                          │2006-12-31│2007-05-09│现金：2007-05-30  │2007-05-11│2007-05-16│
│2006-03-22│末期股息8港仙                             │2005-12-31│2006-05-15│现金：2006-06-07  │2006-05-17│2006-05-24│
│2005-03-17│末期股息7港仙                             │2004-12-31│2005-04-19│现金：2005-05-17  │2005-04-21│2005-04-27│
└─────┴─────────────────────┴─────┴─────┴─────────┴─────┴─────┘

【2.供股】 暂无数据

【3.拆分合并】
┌─────┬────┬──────────┬──────┬──────┬─────┬─────┬─────┬───────┐
│公告日期  │重组方式│方案概述            │一股合并基数│每股拆细(股)│除净日    │换领股票起│换领股票止│变 更说明      │
├─────┼────┼──────────┼──────┼──────┼─────┼─────┼─────┼───────┤
│2014-03-19│拆股    │每1股拆5股          │         ---│      5.0000│2014-05-15│---       │---       │---           │
└─────┴────┴──────────┴──────┴──────┴─────┴─────┴─────┴───────┘

〖免责条款〗
 1、本公司力求但不保证提供的任何信息的真实性、准确性、完整性及原创性等，投资者使
 用前请自行予以核实，如有错漏请以上市公司信息披露为准，本公司不对因上述信息全部
 或部分内容而引致的盈亏承担任何责任。
 2、本公司无法保证该项服务能满足用户的要求，也不担保服务不会受中断，对服务的及时
 性、安全性以及出错发生都不作担保。
 3、本公司提供的任何信息仅供投资者参考，不作为投资决策的依据，本公司不对投资者依
 据上述信息进行投资决策所产生的收益和损失承担任何责任。投资有风险，应谨慎至上。
"""

    # 执行解析并输出统一结构的列表（按公告日期降序）
    # 如果结果为空，输出一些调试信息以便定位问题
    unified = parse_text_to_list(raw_text)
    if not unified:
        print('DEBUG: unified list is empty')
        print('DEBUG: contains start marker?', '【1.分红派息】' in raw_text)
        # 打印开始附近的文本片段
        si = raw_text.find('【1.分红派息】')
        if si != -1:
            print('DEBUG SNIPPET:', raw_text[si:si+200])
            # 进一步逐行检查表格内容和管道字符
            start_marker = "【1.分红派息】"
            end_marker = "【2.供股】"
            sidx = raw_text.find(start_marker)
            eidx = raw_text.find(end_marker)
            if sidx != -1:
                section = raw_text[sidx + len(start_marker): eidx if eidx != -1 else None]
                lines = section.split('\n')
                print('DEBUG: section line count', len(lines))
                for i, L in enumerate(lines[:30]):
                    print(i, repr(L), 'HAS_PIPE:', ('│' in L))
    print(json.dumps(unified, ensure_ascii=False, indent=2))

if __name__ == "__main__":
    text = "中期股息每股 9.18 港仙，一千股派 108 股盈大地产（432）(相当于每股派息 0.1922 港元)"
    text = '第三次中期股息每股0.1美元，可选择货币0.777722港元或0.075079英镑'
    xx = parse_money_all(text)
    print(xx)
    #main()