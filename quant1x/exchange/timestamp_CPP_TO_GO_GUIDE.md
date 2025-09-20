# C++ Timestamp 类到 Go 语言的转换文档

本文档展示了如何将 C++ 的 `timestamp` 类转换为对应的 Go 语言实现。

## 概述

C++ 版本的 `timestamp` 类是一个本地时间戳类，以毫秒为单位存储时间。Go 版本通过 `TimestampCpp` 结构体提供了完全对应的功能。

## 类型对应关系

| C++ | Go | 说明 |
|-----|----|----|
| `timestamp` | `TimestampCpp` | 主要时间戳类型 |
| `int64_t ms_` | `ms int64` | 内部毫秒存储 |

## 构造函数对应关系

### C++ 构造函数
```cpp
// 默认构造函数
timestamp();

// 从毫秒数构造
timestamp(int64_t t);

// 从时间点构造
timestamp(const std::chrono::system_clock::time_point &tp);

// 从字符串构造
timestamp(const std::string &str);

// 从年月日时分秒毫秒构造
timestamp(int y, int m, int d, int hh = 0, int mm = 0, int ss = 0, int sss = 0);
```

### Go 对应函数
```go
// 零值构造
func ZeroTimestamp() TimestampCpp

// 从毫秒数构造
func NewTimestamp(ms int64) TimestampCpp

// 从time.Time构造
func NewTimestampFromTime(t time.Time) TimestampCpp

// 从字符串构造
func NewTimestampFromString(str string) (TimestampCpp, error)

// 从年月日时分秒毫秒构造
func NewTimestampFromDate(year, month, day, hour, minute, second, millisecond int) TimestampCpp
```

## 静态方法对应关系

| C++ 静态方法 | Go 函数 | 功能说明 |
|-------------|---------|---------|
| `timestamp::now()` | `NowTimestamp()` | 获取当前时间戳 |
| `timestamp::zero()` | `ZeroTimestamp()` | 获取零值时间戳 |
| `timestamp::parse(str)` | `ParseTimestamp(str)` | 解析日期时间字符串 |
| `timestamp::parse_time(str)` | `ParseTimeOnly(str)` | 仅解析时间字符串 |
| `timestamp::pre_market_time(y,m,d)` | `PreMarketTimestamp(y,m,d)` | 盘前时间戳 |
| `timestamp::midnight()` | `MidnightTimestamp()` | 当天零点时间戳 |

## 实例方法对应关系

| C++ 方法 | Go 方法 | 功能说明 |
|----------|---------|---------|
| `value()` | `Value()` | 获取毫秒数 |
| `start_of_day()` | `StartOfDay()` | 当天零点时间戳 |
| `today(h,m,s,ms)` | `Today(h,m,s,ms)` | 当天指定时间 |
| `since(h,m,s,ms)` | `Since(h,m,s,ms)` | 从零点开始的时间 |
| `offset(h,m,s,ms)` | `Offset(h,m,s,ms)` | 偏移时间 |
| `pre_market_time()` | `PreMarketTime()` | 当天盘前时间 |
| `floor()` | `Floor()` | 向下取整到分钟 |
| `ceil()` | `Ceil()` | 向上取整到分钟 |
| `extract()` | `Extract()` | 提取年月日 |
| `toString(layout)` | `ToString(layout...)` | 格式化为字符串 |
| `toStringAsTimeInSeconds(layout)` | `ToStringAsTimeInSeconds(layout...)` | 以秒为单位格式化 |
| `only_date()` | `OnlyDate()` | 仅返回日期 |
| `cache_date()` | `CacheDate()` | 缓存日期格式 |
| `only_time()` | `OnlyTime()` | 仅返回时间 |
| `yyyymmdd()` | `YYYYMMDD()` | 整型日期格式 |
| `empty()` | `IsEmpty()` | 是否为空 |
| `is_same_date(other)` | `IsSameDate(other)` | 是否同一天 |

## 比较运算符对应关系

| C++ 运算符 | Go 方法 | 功能说明 |
|-----------|---------|---------|
| `operator==` | `Equal(other)` | 等于比较 |
| `operator!=` | `NotEqual(other)` | 不等于比较 |
| `operator<` | `Less(other)` | 小于比较 |
| `operator>` | `Greater(other)` | 大于比较 |
| `operator<=` | `LessOrEqual(other)` | 小于等于比较 |
| `operator>=` | `GreaterOrEqual(other)` | 大于等于比较 |
| `operator<<` | `String()` | 字符串输出 |

## 使用示例对比

### C++ 示例
```cpp
#include <quant1x/exchange/timestamp.h>
using namespace exchange;

// 创建时间戳
timestamp ts1(2022, 1, 1, 15, 30, 45, 123);
timestamp ts2 = timestamp::now();
timestamp ts3 = timestamp::parse("2022-01-01 15:30:45");

// 时间操作
timestamp startDay = ts1.start_of_day();
timestamp today9AM = ts1.today(9, 0, 0, 0);
timestamp offset = ts1.offset(1, 30, 0, 0);

// 格式化
std::string dateStr = ts1.only_date();
std::string timeStr = ts1.only_time();
uint32_t yyyymmdd = ts1.yyyymmdd();

// 比较
if (ts1 < ts2) {
    std::cout << "ts1 is earlier" << std::endl;
}

// 输出
std::cout << ts1 << std::endl;
```

### Go 对应示例
```go
package main

import (
    "fmt"
    "gitee.com/quant1x/std/timestamp"
)

func main() {
    // 创建时间戳
    ts1 := timestamp.NewTimestampFromDate(2022, 1, 1, 15, 30, 45, 123)
    ts2 := timestamp.NowTimestamp()
    ts3, _ := timestamp.ParseTimestamp("2022-01-01 15:30:45")

    // 时间操作
    startDay := ts1.StartOfDay()
    today9AM := ts1.Today(9, 0, 0, 0)
    offset := ts1.Offset(1, 30, 0, 0)

    // 格式化
    dateStr := ts1.OnlyDate()
    timeStr := ts1.OnlyTime()
    yyyymmdd := ts1.YYYYMMDD()

    // 比较
    if ts1.Less(ts2) {
        fmt.Println("ts1 is earlier")
    }

    // 输出
    fmt.Println(ts1.String())
}
```

## 主要差异

1. **错误处理**: Go 版本的解析函数返回 `error`，而 C++ 版本可能抛出异常。
2. **可变参数**: Go 版本的格式化函数使用可变参数 `...string`，C++ 版本使用默认参数。
3. **命名约定**: Go 版本遵循 Go 的命名约定（首字母大写的公开方法）。
4. **类型安全**: Go 版本通过类型系统提供更好的类型安全。

## 性能考虑

- Go 版本保持了与 C++ 版本相同的核心算法
- 内存布局相似（都使用 int64 存储毫秒）
- 时间复杂度保持一致
- 提供了基准测试来验证性能

## 测试覆盖

Go 版本包含了完整的测试套件，覆盖：
- 基本功能测试
- 时间操作测试
- 格式化测试
- 比较操作测试
- 解析功能测试
- 边界情况测试
- 性能基准测试

这样确保了 Go 版本与 C++ 版本在功能上的完全对等。