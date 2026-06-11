# bar - K线数据结构

## 依赖

无外部依赖。

## 数据结构

### Bar (Struct)

K线数据结构体。

| Rust 字段 | Python 字段 | 类型 | 说明 |
|---|---|---|---|
| date | date | String/str | 日期 YYYY-MM-DD |
| open | open | f64/float | 开盘价 |
| close | close | f64/float | 收盘价 |
| high | high | f64/float | 最高价 |
| low | low | f64/float | 最低价 |
| volume | volume | f64/float | 成交量 |
| amount | amount | f64/float | 成交额 |
| up | up | i32/int | 上涨家数（仅指数） |
| down | down | i32/int | 下跌家数（仅指数） |
| timestamp | timestamp | String/str | 时间戳 YYYY-MM-DD HH:MM:SS |
| adjustment_count | adjustment_count | i32/int | 复权次数 |

方法：
- `headers()` — K线数据CSV头部
