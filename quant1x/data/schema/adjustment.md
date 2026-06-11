# adjustment - 除权除息数据结构

## 依赖

- `quant1x/data/meta::Exchange` - 交易所枚举
- `quant1x/data/meta::Timestamp` - 时间戳

## 数据结构

### XdxrCategory (Enum)

除权除息类型枚举。

| 值 | Rust 变体 | Python 名称 | 含义 |
|---|---|---|---|
| 1 | ExDividend | EX_DIVIDEND | 除权除息 |
| 2 | BonusSharesListing | BONUS_SHARES_LISTING | 送股上市 |
| 3 | RestrictedSharesListing | RESTRICTED_SHARES_LISTING | 非流通股上市 |
| 4 | UnspecifiedCapitalAdjustment | UNSPECIFIED_CAPITAL_ADJUSTMENT | 未知股本变动 |
| 5 | GeneralCapitalAdjustment | GENERAL_CAPITAL_ADJUSTMENT | 股本变化 |
| 6 | NewShareIssuance | NEW_SHARE_ISSUANCE | 增发新股 |
| 7 | ShareRepurchase | SHARE_REPURCHASE | 股份回购 |
| 8 | NewSharesListing | NEW_SHARES_LISTING | 增发新股上市 |
| 9 | TransferredRightsSharesListing | TRANSFERRED_RIGHTS_SHARES_LISTING | 转配股上市 |
| 10 | ConvertibleBondListing | CONVERTIBLE_BOND_LISTING | 可转债上市 |
| 11 | StockSplitOrReverseSplit | STOCK_SPLIT_OR_REVERSE_SPLIT | 拆股或合股 |
| 12 | RestrictedSharesConsolidation | RESTRICTED_SHARES_CONSOLIDATION | 非流通股缩股 |
| 13 | IssueCallWarrants | ISSUE_CALL_WARRANTS | 送认购权证 |
| 14 | IssuePutWarrants | ISSUE_PUT_WARRANTS | 送认沽权证 |

方法：
- `to_string(category: i32) -> String` — 类型编号转中文字符串

### XdxrInfo (Struct)

除权除息信息。

| Rust 字段 | Python 字段 | 类型 | 说明 |
|---|---|---|---|
| date | Date | String/str | 日期 YYYY-MM-DD |
| category | Category | i32/int | 类型编号 |
| name | Name | String/str | 类型名称 |
| fen_hong | FenHong | f64/float | 分红(元) |
| dividend_currency | dividend_currency | String/str | 分红币种 |
| pei_gu_jia | PeiGuJia | f64/float | 配股价(元) |
| rights_currency | rights_currency | String/str | 配股价币种 |
| song_zhuan_gu | SongZhuanGu | f64/float | 送转股(股) |
| pei_gu | PeiGu | f64/float | 配股(股) |
| suo_gu | SuoGu | f64/float | 缩股(股) |
| qian_liu_tong | QianLiuTong | f64/float | 除权前流通股(万股) |
| hou_liu_tong | HouLiuTong | f64/float | 除权后流通股(万股) |
| qian_zong_gu_ben | QianZongGuBen | f64/float | 除权前总股本(万股) |
| hou_zong_gu_ben | HouZongGuBen | f64/float | 除权后总股本(万股) |
| fen_shu | FenShu | f64/float | 权证份数 |
| xing_quan_jia | XingQuanJia | f64/float | 行权价格(元) |

方法：
- `is_adjust()` — 是否需要复权
- `adjust_factor()` — 计算除权因子 (m, a)
- `compute_monetary_adjustment()` — 货币调整金额
- `compute_share_adjustment_ratio()` — 股本调整比率
- `is_capital_change()` — 是否为股本变动

### XdxrEntry (Struct)

除权除息条目。

| Rust 字段 | Python 字段 | 类型 | 说明 |
|---|---|---|---|
| exchange | exchange | Exchange | 交易所 |
| ticker | ticker | String/str | 证券代码 |
| count | count | i32/int | 记录数 |
| list | list | Vec\<XdxrInfo\>/List[XdxrInfo] | 除权除息记录列表 |

### CumulativeAdjustment (Struct)

复权数据结构体。

| Rust 字段 | Python 字段 | 类型 | 说明 |
|---|---|---|---|
| timestamp | timestamp | Timestamp | 复权日期 |
| m | m | f64/float | 乘性因子 |
| a | a | f64/float | 加性因子 |
| monetary_adjustment | monetary_adjustment | f64/float | 货币调整 |
| share_adjustment_ratio | share_adjustment_ratio | f64/float | 股本调整比率 |
| no | no | i32/int | 复权调整序号 |

方法：
- `new()` — 构造函数
- `to_string()` — 格式化输出
- `apply(price)` — 复权
- `inverse(adjusted_price)` — 还权
