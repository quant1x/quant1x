# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from enum import Enum
from dataclasses import dataclass, field
from typing import List, Optional, Dict, Any
from datetime import date

# ================= 枚举定义 =================

class MarketType(Enum):
    """市场类型"""
    A_SHARE = "A 股"
    HK_SHARE = "港股"
    US_SHARE = "美股"
    UK_SHARE = "英股"
    SG_SHARE = "新加坡"
    FUND = "基金"
    REITS = "REITs"
    OTHER = "其他"

class DividendType(Enum):
    """分红类型"""
    CASH = "现金分红"
    SPECIAL = "特别分红"
    PROPERTY = "实物分红"
    NONE = "无分红"

class BonusType(Enum):
    """红股类型"""
    BONUS_ISSUE = "红股发行"
    STOCK_DIVIDEND = "股票分红"
    CAPITALIZATION = "资本化发行"
    NONE = "无红股"

class ActionType(Enum):
    """公司行为类型"""
    DIVIDEND = "分红"
    BONUS = "送红股"
    SPLIT = "拆股"
    REVERSE_SPLIT = "缩股/合股"
    CONSOLIDATION = "股份合并"
    RIGHTS_ISSUE = "供股/配股"
    MIXED = "混合方案"
    SPIN_OFF = "分拆上市"

# ================= 核心数据模型（扁平化，含 Bonus）=================

@dataclass
class DividendAdjustmentRecord:
    """
    除权除息记录 - 扁平化设计
    
    明确区分：
    - Dividend（现金分红）
    - Bonus（送红股）
    - Split（拆股）
    - Consolidation（缩股/合并）
    
    金额与币种独立存储
    """
    # ===== 基础信息 =====
    symbol: str                           # 股票代码
    market: MarketType                    # 市场类型
    action_type: ActionType               # 行为类型
    
    # ===== 日期字段 =====
    announcement_date: Optional[str] = None   # 公告日期
    record_date: Optional[str] = None         # 股权登记日
    ex_date: Optional[str] = None             # 除权除息日（核心）
    payment_date: Optional[str] = None        # 派息/到账日
    
    # ===== Dividend 专用字段（现金分红）⭐ =====
    dividend_amount: Optional[float] = None   # 每股现金分红金额
    dividend_currency: Optional[str] = None   # 分红币种 (CNY/HKD/USD)
    dividend_type: DividendType = DividendType.NONE
    
    # ===== Bonus 专用字段（送红股）⭐ 新增 =====
    bonus_ratio: Optional[float] = None       # 红股比例 (如 10 送 3 -> 0.3)
    bonus_type: BonusType = BonusType.NONE    # 红股类型
    
    # ===== Split 专用字段（拆股）=====
    split_ratio: Optional[float] = None       # 拆股比例 (1 拆 5 -> 5.0)
    
    # ===== Rights Issue 专用字段（供股/配股）=====
    rights_ratio: Optional[float] = None      # 配股比例
    rights_price: Optional[float] = None      # 配股价
    rights_currency: Optional[str] = None     # 配股价币种
    
    # ===== Consolidation 专用字段（缩股/合并）=====
    consolidation_ratio: Optional[float] = None  # 缩股比例 (10 合 1 -> 0.1)
    consolidation_base: Optional[int] = None     # 合并基数 (10)
    consolidation_target: Optional[int] = None   # 合并目标 (1)
    
    # ===== 其他字段 =====
    raw_description: str = ""                   # 原始方案描述
    extra_info: Dict[str, Any] = field(default_factory=dict)
    
    # ===== 便捷方法 =====
    def has_cash_dividend(self) -> bool:
        """是否有现金分红"""
        return self.dividend_amount is not None and self.dividend_amount > 0
    
    def has_bonus(self) -> bool:
        """是否有送红股"""
        return self.bonus_ratio is not None and self.bonus_ratio > 0
    
    def has_split(self) -> bool:
        """是否有拆股"""
        return self.split_ratio is not None and self.split_ratio > 1
    
    def has_consolidation(self) -> bool:
        """是否有缩股/合并"""
        return (self.consolidation_ratio is not None and self.consolidation_ratio < 1) or \
               (self.consolidation_base is not None and self.consolidation_target is not None)
    
    def has_rights_issue(self) -> bool:
        """是否有供股/配股"""
        return self.rights_ratio is not None and self.rights_ratio > 0
    
    def get_consolidation_factor(self) -> Optional[float]:
        """获取缩股因子"""
        if self.consolidation_ratio:
            return self.consolidation_ratio
        if self.consolidation_base and self.consolidation_target:
            return self.consolidation_target / self.consolidation_base
        return None
    
    def get_bonus_factor(self) -> float:
        """获取红股因子 (1 + bonus_ratio)"""
        if self.has_bonus():
            return 1 + self.bonus_ratio
        return 1.0
    
    def get_split_factor(self) -> float:
        """获取拆股因子"""
        if self.has_split():
            return self.split_ratio
        return 1.0
    
    def get_adjustment_factor(self) -> Dict[str, float]:
        """
        获取除权除息因子（用于复权计算）
        返回：{price_factor, share_factor, cash_dividend}
        """
        factor = {
            "price_factor": 1.0,
            "share_factor": 1.0,
            "cash_dividend": 0.0
        }
        
        # 1. 现金分红
        if self.has_cash_dividend():
            factor["cash_dividend"] = self.dividend_amount
        
        # 2. Bonus 红股（股份扩张，价格下降）⭐
        if self.has_bonus():
            bonus_factor = self.get_bonus_factor()
            factor["price_factor"] /= bonus_factor
            factor["share_factor"] *= bonus_factor
        
        # 3. Split 拆股（股份扩张，价格下降）
        if self.has_split():
            split_factor = self.get_split_factor()
            factor["price_factor"] /= split_factor
            factor["share_factor"] *= split_factor
        
        # 4. Consolidation 缩股/合并（股份收缩，价格上升）
        if self.has_consolidation():
            cf = self.get_consolidation_factor()
            if cf and cf > 0:
                factor["price_factor"] /= cf
                factor["share_factor"] *= cf
        
        return factor
    
    def get_adjustment_description(self) -> str:
        """获取除权除息描述文本"""
        parts = []
        
        if self.has_cash_dividend():
            parts.append(f"派息{self.dividend_amount}{self.dividend_currency or ''}")
        
        if self.has_bonus():
            parts.append(f"送红股{self.bonus_ratio * 10:.1f}股/10 股")
        
        if self.has_split():
            parts.append(f"拆股 1 拆{self.split_ratio}")
        
        if self.has_consolidation():
            if self.consolidation_base and self.consolidation_target:
                parts.append(f"合并{self.consolidation_base}合{self.consolidation_target}")
            else:
                parts.append(f"缩股")
        
        if self.has_rights_issue():
            parts.append(f"供股{self.rights_ratio * 10:.1f}股/10 股")
        
        return " + ".join(parts) if parts else "无"


# ================= 核心处理类 ⭐ =================

class DividendAdjustment:
    """
    分红除权除息数据处理中心
    
    功能：
    1. 统一存储 A 股、港股、美股、英股等多市场数据
    2. 明确区分 Dividend（现金）、Bonus（红股）、Split（拆股）
    3. 金额与币种独立存储
    4. 支持复权计算
    """
    
    def __init__(self):
        self.records: List[DividendAdjustmentRecord] = []
    
    def add_record(self, record: DividendAdjustmentRecord):
        """添加一条除权除息记录"""
        self.records.append(record)
    
    def add_records(self, records: List[DividendAdjustmentRecord]):
        """批量添加记录"""
        self.records.extend(records)
    
    def get_dividend_records(self, symbol: str) -> List[DividendAdjustmentRecord]:
        """获取某标的的所有现金分红记录"""
        return [r for r in self.records if r.symbol == symbol and r.has_cash_dividend()]
    
    def get_bonus_records(self, symbol: str) -> List[DividendAdjustmentRecord]:
        """获取某标的的所有送红股记录 ⭐ 新增"""
        return [r for r in self.records if r.symbol == symbol and r.has_bonus()]
    
    def get_all_records(self, symbol: str) -> List[DividendAdjustmentRecord]:
        """获取某标的的所有除权除息记录"""
        return [r for r in self.records if r.symbol == symbol]
    
    def get_by_market(self, market: MarketType) -> List[DividendAdjustmentRecord]:
        """获取某市场的所有记录"""
        return [r for r in self.records if r.market == market]
    
    def get_by_action_type(self, action_type: ActionType) -> List[DividendAdjustmentRecord]:
        """获取某类型的公司行为记录"""
        return [r for r in self.records if r.action_type == action_type]
    
    def get_by_ex_date_range(self, start_date: str, end_date: str) -> List[DividendAdjustmentRecord]:
        """获取某时间段内的记录"""
        return [r for r in self.records if r.ex_date and start_date <= r.ex_date <= end_date]
    
    def calculate_ex_dividend_price(self, symbol: str, price: float, ex_date: str) -> float:
        """计算除息后的理论价格"""
        records = self.get_all_records(symbol)
        target = next((r for r in records if r.ex_date == ex_date), None)
        
        if not target:
            return price
        
        factor = target.get_adjustment_factor()
        adjusted = price - factor["cash_dividend"]
        adjusted /= factor["price_factor"] if factor["price_factor"] > 0 else 1
        
        return round(adjusted, 2)
    
    def calculate_adjusted_shares(self, symbol: str, shares: int, ex_date: str) -> int:
        """计算除权后的持股数量"""
        records = self.get_all_records(symbol)
        target = next((r for r in records if r.ex_date == ex_date), None)
        
        if not target:
            return shares
        
        factor = target.get_adjustment_factor()
        return int(shares * factor["share_factor"])
    
    def get_total_dividend_income(self, symbol: str, shares: int, 
                                   start_date: str = None, end_date: str = None,
                                   exchange_rate: float = 1.0) -> float:
        """计算某标的在时间段内的现金分红总收入"""
        records = self.get_dividend_records(symbol)
        total = 0.0
        
        for r in records:
            if start_date and r.ex_date and r.ex_date < start_date:
                continue
            if end_date and r.ex_date and r.ex_date > end_date:
                continue
            
            if r.has_cash_dividend():
                total += r.dividend_amount * shares * exchange_rate
        
        return round(total, 2)
    
    def get_total_bonus_shares(self, symbol: str, shares: int, 
                                start_date: str = None, end_date: str = None) -> int:
        """计算某标的在时间段内的送红股总数 ⭐ 新增"""
        records = self.get_bonus_records(symbol)
        total_bonus = 0
        
        for r in records:
            if start_date and r.ex_date and r.ex_date < start_date:
                continue
            if end_date and r.ex_date and r.ex_date > end_date:
                continue
            
            if r.has_bonus():
                total_bonus += int(shares * r.bonus_ratio)
        
        return total_bonus
    
    def export_to_dict(self) -> Dict[str, Any]:
        """导出为字典格式"""
        return {
            "total_count": len(self.records),
            "dividend_count": len([r for r in self.records if r.has_cash_dividend()]),
            "bonus_count": len([r for r in self.records if r.has_bonus()]),
            "split_count": len([r for r in self.records if r.has_split()]),
            "consolidation_count": len([r for r in self.records if r.has_consolidation()]),
            "markets": list(set(r.market.value for r in self.records)),
            "action_types": list(set(r.action_type.value for r in self.records)),
            "records": [
                {
                    "symbol": r.symbol,
                    "market": r.market.value,
                    "action_type": r.action_type.value,
                    "ex_date": r.ex_date,
                    "record_date": r.record_date,
                    "payment_date": r.payment_date,
                    "dividend_amount": r.dividend_amount,
                    "dividend_currency": r.dividend_currency,
                    "dividend_type": r.dividend_type.value,
                    "bonus_ratio": r.bonus_ratio,
                    "bonus_type": r.bonus_type.value,
                    "split_ratio": r.split_ratio,
                    "rights_ratio": r.rights_ratio,
                    "rights_price": r.rights_price,
                    "consolidation_base": r.consolidation_base,
                    "consolidation_target": r.consolidation_target,
                    "adjustment_description": r.get_adjustment_description(),
                    "raw_description": r.raw_description
                }
                for r in self.records
            ]
        }
    
    def export_to_csv(self, filepath: str):
        """导出为 CSV 文件"""
        import csv
        
        with open(filepath, 'w', newline='', encoding='utf-8-sig') as f:
            writer = csv.writer(f)
            writer.writerow([
                'Symbol', 'Market', 'ActionType', 'ExDate', 'RecordDate', 'PaymentDate',
                'DividendAmount', 'DividendCurrency', 'BonusRatio', 'BonusType',
                'SplitRatio', 'RightsRatio', 'RightsPrice', 
                'ConsolidationBase', 'ConsolidationTarget', 
                'AdjustmentDescription', 'RawDescription'
            ])
            
            for r in self.records:
                writer.writerow([
                    r.symbol,
                    r.market.value,
                    r.action_type.value,
                    r.ex_date,
                    r.record_date,
                    r.payment_date,
                    r.dividend_amount,
                    r.dividend_currency,
                    r.bonus_ratio,
                    r.bonus_type.value,
                    r.split_ratio,
                    r.rights_ratio,
                    r.rights_price,
                    r.consolidation_base,
                    r.consolidation_target,
                    r.get_adjustment_description(),
                    r.raw_description
                ])


# ================= 使用示例（含 Bonus 场景）⭐ =================

if __name__ == "__main__":
    import json
    
    # 初始化处理器
    dividend_processor = DividendAdjustment()
    
    # 1. 腾讯控股 (港股) 现金分红
    dividend_processor.add_record(DividendAdjustmentRecord(
        symbol="00700.HK",
        market=MarketType.HK_SHARE,
        action_type=ActionType.DIVIDEND,
        announcement_date="2025-03-19",
        record_date="2024-12-31",
        ex_date="2025-05-16",
        payment_date="2025-05-30",
        dividend_amount=4.5,
        dividend_currency="HKD",
        dividend_type=DividendType.CASH,
        raw_description="末期股息每股 4.5 港元"
    ))
    
    # 2. 某港股 送红股 (10 送 2) ⭐ 新增 Bonus 场景
    dividend_processor.add_record(DividendAdjustmentRecord(
        symbol="00005.HK",
        market=MarketType.HK_SHARE,
        action_type=ActionType.BONUS,
        announcement_date="2024-03-15",
        record_date="2024-05-20",
        ex_date="2024-05-21",
        payment_date="2024-06-10",
        bonus_ratio=0.2,  # 10 送 2
        bonus_type=BonusType.BONUS_ISSUE,
        raw_description="每 10 股送 2 股红股"
    ))
    
    # 3. 某港股 混合方案 (派息 + 送红股) ⭐ 新增
    dividend_processor.add_record(DividendAdjustmentRecord(
        symbol="00001.HK",
        market=MarketType.HK_SHARE,
        action_type=ActionType.MIXED,
        announcement_date="2024-03-20",
        record_date="2024-05-25",
        ex_date="2024-05-26",
        payment_date="2024-06-15",
        dividend_amount=1.5,
        dividend_currency="HKD",
        dividend_type=DividendType.CASH,
        bonus_ratio=0.1,  # 10 送 1
        bonus_type=BonusType.BONUS_ISSUE,
        raw_description="每股派息 1.5 港元 + 每 10 股送 1 股红股"
    ))
    
    # 4. 某港股 缩股 (10 合 1)
    dividend_processor.add_record(DividendAdjustmentRecord(
        symbol="01234.HK",
        market=MarketType.HK_SHARE,
        action_type=ActionType.REVERSE_SPLIT,
        ex_date="2024-07-11",
        consolidation_base=10,
        consolidation_target=1,
        raw_description="每 10 股合并为 1 股"
    ))
    
    # 5. 某 A 股 混合方案 (10 送 3 派 2 元)
    dividend_processor.add_record(DividendAdjustmentRecord(
        symbol="600000.SH",
        market=MarketType.A_SHARE,
        action_type=ActionType.MIXED,
        ex_date="2024-05-11",
        record_date="2024-05-10",
        payment_date="2024-05-20",
        dividend_amount=0.2,
        dividend_currency="CNY",
        dividend_type=DividendType.CASH,
        bonus_ratio=0.3,  # 10 送 3
        bonus_type=BonusType.STOCK_DIVIDEND,
        raw_description="10 送 3 派 2 元"
    ))
    
    # 6. 某美股 拆股 (1 拆 5) + 分红
    dividend_processor.add_record(DividendAdjustmentRecord(
        symbol="NVDA.US",
        market=MarketType.US_SHARE,
        action_type=ActionType.MIXED,
        ex_date="2024-06-07",
        dividend_amount=0.04,
        dividend_currency="USD",
        dividend_type=DividendType.CASH,
        split_ratio=5.0,
        raw_description="1-for-5 split + $0.04 dividend"
    ))
    
    # 7. 某港股 供股 (2 供 1) ⭐ 新增
    dividend_processor.add_record(DividendAdjustmentRecord(
        symbol="00002.HK",
        market=MarketType.HK_SHARE,
        action_type=ActionType.RIGHTS_ISSUE,
        ex_date="2024-08-01",
        rights_ratio=0.5,  # 2 供 1
        rights_price=10.0,
        rights_currency="HKD",
        raw_description="每 2 股供 1 股，供股价 10 港元"
    ))
    
    # 输出汇总
    print(json.dumps(dividend_processor.export_to_dict(), ensure_ascii=False, indent=2))
    
    # 测试除息价格计算
    print("\n=== 除息价格计算 ===")
    original_price = 400.0
    ex_price = dividend_processor.calculate_ex_dividend_price("00700.HK", original_price, "2025-05-16")
    print(f"腾讯控股 除息前：{original_price} HKD → 除息后：{ex_price} HKD")
    
    # 测试 Bonus 价格计算
    print("\n=== 送红股价格计算 ===")
    original_price = 100.0
    ex_price = dividend_processor.calculate_ex_dividend_price("00005.HK", original_price, "2024-05-21")
    print(f"送红股前：{original_price} HKD → 送红股后：{ex_price} HKD (10 送 2)")
    
    # 测试混合方案价格计算
    print("\n=== 混合方案价格计算 ===")
    original_price = 80.0
    ex_price = dividend_processor.calculate_ex_dividend_price("00001.HK", original_price, "2024-05-26")
    print(f"混合方案前：{original_price} HKD → 除权后：{ex_price} HKD (派息 1.5 + 10 送 1)")
    
    # 测试缩股价格计算
    print("\n=== 缩股价格计算 ===")
    original_price = 1.5
    ex_price = dividend_processor.calculate_ex_dividend_price("01234.HK", original_price, "2024-07-11")
    print(f"缩股前：{original_price} HKD → 缩股后：{ex_price} HKD (10 合 1)")
    
    # 测试分红收入计算
    print("\n=== 分红收入计算 ===")
    shares = 1000
    income = dividend_processor.get_total_dividend_income("00700.HK", shares)
    print(f"持有 {shares} 股腾讯控股，预计现金分红收入：{income} HKD")
    
    # 测试红股数量计算 ⭐ 新增
    print("\n=== 送红股数量计算 ===")
    shares = 1000
    bonus_shares = dividend_processor.get_total_bonus_shares("00005.HK", shares)
    print(f"持有 {shares} 股，预计获得红股：{bonus_shares} 股 (10 送 2)")
    
    # 测试除权描述
    print("\n=== 除权描述 ===")
    for r in dividend_processor.records:
        print(f"{r.symbol}: {r.get_adjustment_description()}")