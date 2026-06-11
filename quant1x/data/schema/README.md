# Schema - 核心数据结构定义

本模块提供标准化数据结构，用于统一表示市场原始与衍生数据，为数据抽象层，只管定义。

## 模块索引

| 模块 | 说明 | 文档 | Python | Rust | Go | C++ |
|---|---|---|---|---|---|---|
| adjustment | 除权除息数据结构 | [adjustment.md](adjustment.md) | ✅ | ✅ | ❌ | ❌ |
| bar | K线数据结构 | [bar.md](bar.md) | ✅ | ✅ | ❌ | ❌ |
| company | 公司信息数据结构 | [company.md](company.md) | ✅ | ✅ | ❌ | ❌ |
| dividend | 分红除权除息数据结构 | [dividend.md](dividend.md) | ✅ | ✅ | ❌ | ❌ |
| sector | 板块信息数据结构 | [sector.md](sector.md) | ✅ | ✅ | ❌ | ❌ |
| trade | 逐笔交易数据结构 | [trade.md](trade.md) | ✅ | ✅ | ❌ | ❌ |

## 导出类型

| 类型 | 来源模块 |
|---|---|
| XdxrCategory, XdxrInfo, XdxrEntry, CumulativeAdjustment | adjustment |
| Bar | bar |
| CompanyInfoChunk | company |
| MarketType, DividendType, BonusType, ActionType | dividend |
| DividendAdjustmentRecord, DividendAdjustment | dividend |
| Sector | sector |
| Direction, Transaction | trade |
