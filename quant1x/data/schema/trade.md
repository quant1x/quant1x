# trade - 逐笔交易数据结构

## 依赖

无外部依赖（仅标准库 `std::collections::HashMap`）。

## 枚举定义

### Direction (Enum)

交易方向。

| 值 | Rust 变体 | Python 名称 | 说明 |
|---|---|---|---|
| 0 | Buy | BUY | 主动买入 |
| 1 | Sell | SELL | 主动卖出 |
| 2 | Neutral | NEUTRAL | 中性盘 |

## 数据结构

### Transaction (Struct)

逐笔交易数据结构体。

| Rust 字段 | Python 字段 | 类型 | 说明 |
|---|---|---|---|
| time | time | String/str | 时间 |
| price | price | f64/float | 价格 |
| volume | volume | i64/int | 成交量 |
| num | num | i64/int | 成交笔数 |
| amount | amount | f64/float | 成交额 |
| direction | direction | i32/int | 交易方向 |

方法：
- `headers()` — 逐笔交易数据CSV头部
- `to_map()` — 转为扁平字典
