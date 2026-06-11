# dividend - 分红除权除息数据结构

## 依赖

无外部依赖（仅标准库 `std::collections::HashMap`）。

## 枚举定义

### MarketType (Enum)

市场类型。

| Rust 变体 | Python 值 | 说明 |
|---|---|---|
| AShare | "A 股" | A股 |
| HkShare | "港股" | 港股 |
| UsShare | "美股" | 美股 |
| UkShare | "英股" | 英股 |
| SgShare | "新加坡" | 新加坡 |
| Fund | "基金" | 基金 |
| Reits | "REITs" | REITs |
| Other | "其他" | 其他 |

### DividendType (Enum)

分红类型。

| Rust 变体 | Python 值 | 说明 |
|---|---|---|
| Cash | "现金分红" | 现金分红 |
| Special | "特别分红" | 特别分红 |
| Property | "实物分红" | 实物分红 |
| None_ | "无分红" | 无分红 |

### BonusType (Enum)

红股类型。

| Rust 变体 | Python 值 | 说明 |
|---|---|---|
| BonusIssue | "红股发行" | 红股发行 |
| StockDividend | "股票分红" | 股票分红 |
| Capitalization | "资本化发行" | 资本化发行 |
| None_ | "无红股" | 无红股 |

### ActionType (Enum)

公司行为类型。

| Rust 变体 | Python 值 | 说明 |
|---|---|---|
| Dividend | "分红" | 分红 |
| Bonus | "送红股" | 送红股 |
| Split | "拆股" | 拆股 |
| ReverseSplit | "缩股/合股" | 缩股/合股 |
| Consolidation | "股份合并" | 股份合并 |
| RightsIssue | "供股/配股" | 供股/配股 |
| Mixed | "混合方案" | 混合方案 |
| SpinOff | "分拆上市" | 分拆上市 |

## 数据结构

### DividendAdjustmentRecord (Struct)

除权除息记录。扁平化设计，明确区分 Dividend/Bonus/Split/Consolidation。

| Rust 字段 | Python 字段 | 类型 | 说明 |
|---|---|---|---|
| symbol | symbol | String/str | 股票代码 |
| market | market | MarketType | 市场类型 |
| action_type | action_type | ActionType | 行为类型 |
| announcement_date | announcement_date | Option\<String\>/Optional[str] | 公告日期 |
| record_date | record_date | Option\<String\>/Optional[str] | 股权登记日 |
| ex_date | ex_date | Option\<String\>/Optional[str] | 除权除息日 |
| payment_date | payment_date | Option\<String\>/Optional[str] | 派息/到账日 |
| dividend_amount | dividend_amount | Option\<f64\>/Optional[float] | 每股现金分红金额 |
| dividend_currency | dividend_currency | Option\<String\>/Optional[str] | 分红币种 |
| dividend_type | dividend_type | DividendType | 分红类型 |
| bonus_ratio | bonus_ratio | Option\<f64\>/Optional[float] | 红股比例 |
| bonus_type | bonus_type | BonusType | 红股类型 |
| split_ratio | split_ratio | Option\<f64\>/Optional[float] | 拆股比例 |
| rights_ratio | rights_ratio | Option\<f64\>/Optional[float] | 配股比例 |
| rights_price | rights_price | Option\<f64\>/Optional[float] | 配股价 |
| rights_currency | rights_currency | Option\<String\>/Optional[str] | 配股价币种 |
| consolidation_ratio | consolidation_ratio | Option\<f64\>/Optional[float] | 缩股比例 |
| consolidation_base | consolidation_base | Option\<i64\>/Optional[int] | 合并基数 |
| consolidation_target | consolidation_target | Option\<i64\>/Optional[int] | 合并目标 |
| raw_description | raw_description | String/str | 原始方案描述 |
| extra_info | extra_info | HashMap/Dict | 额外信息 |

方法：
- `new()` — 构造默认记录
- `has_cash_dividend()` — 是否有现金分红
- `has_bonus()` — 是否有送红股
- `has_split()` — 是否有拆股
- `has_consolidation()` — 是否有缩股/合并
- `has_rights_issue()` — 是否有供股/配股
- `get_consolidation_factor()` — 获取缩股因子
- `get_bonus_factor()` — 获取红股因子
- `get_split_factor()` — 获取拆股因子
- `get_adjustment_factor()` — 获取除权除息因子，返回 (price_factor, share_factor, cash_dividend)
- `get_adjustment_description()` — 获取除权除息描述文本

### DividendAdjustment (Struct)

分红除权除息数据处理中心。

| Rust 字段 | Python 字段 | 类型 | 说明 |
|---|---|---|---|
| records | records | Vec\<DividendAdjustmentRecord\>/List[DividendAdjustmentRecord] | 记录列表 |

方法：
- `add_record()` — 添加一条记录
- `add_records()` — 批量添加记录
- `get_dividend_records(symbol)` — 获取现金分红记录
- `get_bonus_records(symbol)` — 获取送红股记录
- `get_all_records(symbol)` — 获取所有除权除息记录
- `get_by_market(market)` — 按市场筛选
- `get_by_action_type(action_type)` — 按行为类型筛选
- `get_by_ex_date_range(start, end)` — 按日期范围筛选
- `calculate_ex_dividend_price(symbol, price, ex_date)` — 计算除息后理论价格
- `calculate_adjusted_shares(symbol, shares, ex_date)` — 计算除权后持股数量
- `get_total_dividend_income(symbol, shares, start, end, rate)` — 计算现金分红总收入
- `get_total_bonus_shares(symbol, shares, start, end)` — 计算送红股总数
