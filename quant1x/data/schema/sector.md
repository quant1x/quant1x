# sector - 板块信息数据结构

## 依赖

无外部依赖。

## 数据结构

### Sector (Struct)

板块信息结构体。

| Rust 字段 | Python 字段 | 类型 | 说明 |
|---|---|---|---|
| name | name | String/str | 板块名称 |
| code | code | String/str | 板块代码 |
| sector_type | type | i32/int | 板块类型 |
| count | count | i32/int | 成分股数量 |
| block | block | String/str | 板块标识 |
| constituent_stocks | constituent_stocks | Vec\<String\>/List[str] | 成分股列表 |
