# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from quant1x.std.numeric import NumberRange
from .rule import CodeRule
from ..exchange import Exchange
from ..instrument import InstrumentType

# HKEX 香港交易所规则
hkex_rules = [
    # 指数
    CodeRule(Exchange.HKEX, "HSI", InstrumentType.INDEX, "恒生指数", "香港交易所"),
    CodeRule(Exchange.HKEX, "HSCEI", InstrumentType.INDEX, "国企指数", "香港交易所"),
    CodeRule(Exchange.HKEX, "HSCCI", InstrumentType.INDEX, "红筹指数", "香港交易所"),
    CodeRule(Exchange.HKEX, "HSTECH", InstrumentType.INDEX, "恒生科技指数", "香港交易所"),
    
    # 00001-09999, 主板及GEM上市证券
    CodeRule(Exchange.HKEX, NumberRange("00001", "02799"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("02800", "02849"), InstrumentType.FUND, "交易所买卖基金", ""),
    CodeRule(Exchange.HKEX, NumberRange("02850", "02899"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("02900", "02999"), InstrumentType.TEMPORARY_STOCK, "主板临时柜台", ""),
    CodeRule(Exchange.HKEX, NumberRange("03000", "03199"), InstrumentType.FUND, "交易所买卖基金", ""),
    CodeRule(Exchange.HKEX, NumberRange("03200", "03399"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("03400", "03499"), InstrumentType.FUND, "交易所买卖基金", ""),
    CodeRule(Exchange.HKEX, NumberRange("03500", "03599"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("03600", "03999"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("04000", "04199"), InstrumentType.BOND, "外汇基金债券", "香港金融管理局"),
    CodeRule(Exchange.HKEX, NumberRange("04200", "04299"), InstrumentType.BOND, "政府债券", "香港特别行政区"),
    CodeRule(Exchange.HKEX, NumberRange("04300", "04329"), InstrumentType.BOND, "债券证券", "仅售予专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("04330", "04399"), InstrumentType.OTHER, "NASDQA-AMEX实验计划", ""),
    CodeRule(Exchange.HKEX, NumberRange("04400", "04599"), InstrumentType.BOND, "债券证券", "仅售予专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("04600", "04699"), InstrumentType.STOCK, "优先股", "仅售予专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("04700", "04799"), InstrumentType.BOND, "债务证券", "售予公众"),
    CodeRule(Exchange.HKEX, NumberRange("04800", "04999"), InstrumentType.WARRANT, "权证", "SPAC"),
    CodeRule(Exchange.HKEX, NumberRange("05000", "06029"), InstrumentType.BOND, "债券证券", "仅售予专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("06030", "06199"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("06200", "06299"), InstrumentType.OTHER, "香港预讬证券", "香港預託證券"),
    CodeRule(Exchange.HKEX, NumberRange("06300", "06399"), InstrumentType.OTHER, "证券/预讬证券", "被美国联邦证券法界定为受限制(RS)证券"),
    CodeRule(Exchange.HKEX, NumberRange("06400", "06599"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("06600", "06749"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("06750", "06799"), InstrumentType.BOND, "财政部债券", "中华人民共和国"),
    CodeRule(Exchange.HKEX, NumberRange("06800", "06999"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("07000", "07199"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("07200", "07399"), InstrumentType.OTHER, "杠杆及反向产品", ""),
    CodeRule(Exchange.HKEX, NumberRange("07400", "07499"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("07500", "07599"), InstrumentType.OTHER, "杠杆及反向产品", ""),
    CodeRule(Exchange.HKEX, NumberRange("07600", "07699"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("07700", "07799"), InstrumentType.OTHER, "杠杆及反向产品", ""),
    CodeRule(Exchange.HKEX, NumberRange("07800", "07999"), InstrumentType.OTHER, "股份", "SPAC"),
    CodeRule(Exchange.HKEX, NumberRange("08000", "08550"), InstrumentType.GEM_MARKET, "GEM证券", ""),
    CodeRule(Exchange.HKEX, NumberRange("08551", "08600"), InstrumentType.TEMPORARY_STOCK, "GEM临时柜台", ""),
    CodeRule(Exchange.HKEX, NumberRange("08601", "08999"), InstrumentType.GEM_MARKET, "GEM证券", ""),
    CodeRule(Exchange.HKEX, NumberRange("09000", "09199"), InstrumentType.FUND, "交易所买卖基金", "美元"),
    CodeRule(Exchange.HKEX, NumberRange("09200", "09399"), InstrumentType.OTHER, "杠杆及反向产品", "美元"),
    CodeRule(Exchange.HKEX, NumberRange("09400", "09499"), InstrumentType.FUND, "交易所买卖基金", "美元"),
    CodeRule(Exchange.HKEX, NumberRange("09500", "09599"), InstrumentType.OTHER, "杠杆及反向产品", "美元"),
    CodeRule(Exchange.HKEX, NumberRange("09600", "09699"), InstrumentType.STOCK, "主板", ""),
    CodeRule(Exchange.HKEX, NumberRange("09700", "09799"), InstrumentType.OTHER, "杠杆及反向产品", "美元"),
    CodeRule(Exchange.HKEX, NumberRange("09800", "09849"), InstrumentType.FUND, "交易所买卖基金", "美元"),
    CodeRule(Exchange.HKEX, NumberRange("09850", "09999"), InstrumentType.STOCK, "主板", ""),
    
    # 10000-29999, 衍生权证
    CodeRule(Exchange.HKEX, NumberRange("10000", "10899"), InstrumentType.WARRANT, "衍生权证", "相关资产在香港以外地区上市的衍生权证、一篮子权证及非标准型权证"),
    CodeRule(Exchange.HKEX, NumberRange("10900", "10999"), InstrumentType.WARRANT, "衍生权证", "相关资产在香港以外地区上市的衍生权证(以美元买卖)"),
    CodeRule(Exchange.HKEX, NumberRange("11000", "11999"), InstrumentType.WARRANT, "衍生权证", "相关资产在香港以外地区上市的衍生权证、一篮子权证及非标准型权证"),
    CodeRule(Exchange.HKEX, NumberRange("12000", "29999"), InstrumentType.WARRANT, "衍生权证", ""),
    
    # 30000-39999, 供沪深股通使用
    CodeRule(Exchange.HKEX, NumberRange("30000", "39999"), InstrumentType.OTHER, "沪深股通", ""),
    
    # 40000-40999, 仅售于专业投资者的债务证券
    CodeRule(Exchange.HKEX, NumberRange("40000", "40999"), InstrumentType.BOND, "债务证券", "仅售于专业投资者"),
    # 41000-46999, 供日后使用
    CodeRule(Exchange.HKEX, NumberRange("41000", "46999"), InstrumentType.OTHER, "供日后使用", "保留"),
    # 47000-48999, 供日后使用
    CodeRule(Exchange.HKEX, NumberRange("47000", "48999"), InstrumentType.OTHER, "界内证", "保留"),
    # 49000-49499, 供日后使用
    CodeRule(Exchange.HKEX, NumberRange("49000", "49499"), InstrumentType.OTHER, "供日后使用", "保留"),
    # 49500-69999, 牛熊证, callable bull and bear contract
    CodeRule(Exchange.HKEX, NumberRange("49500", "49999"), InstrumentType.OPTION, "牛熊证", "相关资产在香港以外地区上市"),
    CodeRule(Exchange.HKEX, NumberRange("50000", "69999"), InstrumentType.OPTION, "牛熊证", ""),  
    # 70000-79999, 供沪深股通使用
    CodeRule(Exchange.HKEX, NumberRange("70000", "79999"), InstrumentType.OTHER, "沪深股通", ""),
    # 80000-89999, 以人民币买卖的产品
    CodeRule(Exchange.HKEX, NumberRange("80000", "82799"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("82800", "82849"), InstrumentType.FUND, "交易所买卖基金", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("82850", "82899"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("82900", "82999"), InstrumentType.TEMPORARY_STOCK, "主板临时柜台", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("83000", "83199"), InstrumentType.FUND, "交易所买卖基金", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("83200", "83399"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("83400", "83499"), InstrumentType.FUND, "交易所买卖基金", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("83500", "83599"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("83600", "83999"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("84000", "84299"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("84300", "84329"), InstrumentType.BOND, "债券证券", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("84330", "84399"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("84400", "84599"), InstrumentType.BOND, "债务证券", "仅售于专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("84600", "84699"), InstrumentType.STOCK, "优先股", "仅售于专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("84700", "84999"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("85000", "85743"), InstrumentType.BOND, "债务证券", "仅售于专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("85744", "85900"), InstrumentType.BOND, "债务证券", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("85901", "86029"), InstrumentType.BOND, "债务证券", "仅售于专业投资者"),
    CodeRule(Exchange.HKEX, NumberRange("86030", "86199"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("86200", "86299"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("86600", "86799"), InstrumentType.OTHER, "中华人民共和国财政部债券/主板证券", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("86800", "86999"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87000", "87099"), InstrumentType.FUND, "房地产投资信托基金及交易所买卖基金以外的单位信托/互惠基金", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87100", "87199"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("87200", "87399"), InstrumentType.OTHER, "杠杆及反向产品", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87400", "87499"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87500", "87599"), InstrumentType.OTHER, "杠杆及反向产品", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87600", "87699"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87700", "87799"), InstrumentType.OTHER, "杠杆及反向产品", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("87800", "88999"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("89000", "89099"), InstrumentType.BOND, "中华人民共和国财政部债券", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("89100", "89199"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("89200", "89599"), InstrumentType.WARRANT, "衍生权证", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("89600", "89699"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    CodeRule(Exchange.HKEX, NumberRange("89700", "89849"), InstrumentType.OTHER, "供日后使用", "保留"),
    CodeRule(Exchange.HKEX, NumberRange("89850", "89999"), InstrumentType.STOCK, "主板", "以人民币买卖"),
    
    # 90000-99999, 供沪深股通使用
    CodeRule(Exchange.HKEX, "9", InstrumentType.OTHER, "沪深股通", ""),
]

# 港股交易最小变动单位（最小价位表）
# 定义价格区间和对应的最小变动价位
price_ranges = [
    (0.01, 0.25),      # 区间1: 0.01 至 0.25
    (0.25, 0.50),      # 区间2: 0.25 至 0.50
    (0.50, 10.00),     # 区间3: 0.50 至 10.00
    (10.00, 20.00),    # 区间4: 10.00 至 20.00
    (20.00, 100.00),   # 区间5: 20.00 至 100.00
    (100.00, 200.00),  # 区间6: 100.00 至 200.00
    (200.00, 500.00),  # 区间7: 200.00 至 500.00
    (500.00, 1000.00), # 区间8: 500.00 至 1,000.00
    (1000.00, 2000.00),# 区间9: 1,000.00 至 2,000.00
    (2000.00, 5000.00),# 区间10: 2,000.00 至 5,000.00
    (5000.00, 9995.00) # 区间11: 5,000.00 至 9,995.00
]

min_price_changes = [
    0.001,  # 对应区间1
    0.005,  # 对应区间2
    0.010,  # 对应区间3
    0.020,  # 对应区间4
    0.050,  # 对应区间5
    0.100,  # 对应区间6
    0.200,  # 对应区间7
    0.500,  # 对应区间8
    1.000,  # 对应区间9
    2.000,  # 对应区间10
    5.000   # 对应区间11
]

# 创建字典映射
price_info = dict(zip(price_ranges, min_price_changes))

# 函数：根据价格获取最小变动单位
def get_min_price_change(price):
    """
    根据给定的股价，返回对应的最小变动价位
    
    参数:
        price (float): 股票价格
        
    返回:
        float: 最小变动价位
    """
    for (low, high), min_change in price_info.items():
        if low <= price < high:
            return min_change
    # 如果价格超出范围，返回None
    return None

# 函数：格式化价格到最小变动单位的倍数
def round_to_tick(price):
    """
    将价格四舍五入到最小变动单位的整数倍
    
    参数:
        price (float): 原始价格
        
    返回:
        float: 调整后的价格
    """
    min_change = get_min_price_change(price)
    if min_change is None:
        return None
    
    # 计算最接近的最小变动单位倍数
    ticks = round(price / min_change)
    return ticks * min_change

# 示例使用
if __name__ == "__main__":
    # 测试不同价格的变动单位
    test_prices = [0.10, 0.30, 5.00, 15.00, 50.00, 150.00, 300.00, 750.00, 1500.00, 3000.00, 8000.00]
    
    print("港股价格变动单位查询示例:")
    print("=" * 50)
    
    for price in test_prices:
        min_change = get_min_price_change(price)
        rounded_price = round_to_tick(price)
        
        if min_change is not None:
            print(f"股价: HK${price:8.2f} | "
                  f"最小变动单位: HK${min_change:6.3f} | "
                  f"建议报价: HK${rounded_price:8.3f}")
        else:
            print(f"股价: HK${price:8.2f} | 价格超出有效范围")

    # 显示所有价格区间
    print("\n" + "=" * 50)
    print("完整的港股最小变动价位表:")
    print("-" * 50)
    print("股价范围(港元)     最小变动价位(港元)")
    print("-" * 50)
    
    for (low, high), min_change in price_info.items():
        print(f"{low:6.2f} - {high:9.2f}     {min_change:8.3f}")
        
