# Quant1X Technical Indicator Library (C++)

这是一个高性能、泛型、支持变周期的技术指标库，支持以下指标：

| 指标 | 描述 |
|------|------|
| MA<T> | 简单移动平均 |
| EMA<T> | 指数移动平均 |
| SMA<T> | 通达信风格 SMA |
| RSI<T> | 相对强弱指数 |
| MACD<T> | 异同移动平均线 |

## 1. 主要特性

- ✅ 支持固定和变量周期
- ✅ 返回长度一致的序列
- ✅ 前若干个点自动返回默认值（NaN 或 50）
- ✅ 支持 block 式 apply 方法
- ✅ 高性能 O(1) 更新逻辑
- ✅ gtest 单元测试覆盖所有指标
- ✅ 支持绘图模块（matplotlib-cpp）
- ✅ 支持 pybind11 接口（可选）

## 2. 移动平均算法对比说明

### 2.1 算法差异对比表

| 算法类型         | 公式/计算逻辑                                   | 初始值处理          | 输出示例（输入 [1, 2, 3, 4]）       | 应用场景                     |
|------------------|-----------------------------------------------|---------------------|------------------------------------|----------------------------|
| 国际标准 SMA     | SMA_t = (CLOSE_t + ... + CLOSE_{t-N+1}) / N   | 前N-1项为NaN        | [NaN, NaN, NaN, 2.5] (N=4)        | 传统均线策略（MA5/MA20）    |
| 通达信 SMA       | SMA_t = [M*CLOSE_t + (N-M)*SMA_{t-1}] / N     | SMA_0 = CLOSE_0     | [1.0, 1.25, 1.6875, 2.265625] (N=4,M=1) | 国内股票软件               |
| 国际标准 EMA     | EMA_t = α*CLOSE_t + (1-α)*EMA_{t-1}, α=2/(N+1)| EMA_0 = CLOSE_0/SMA | [1.0, 1.333, 2.111, 2.740] (N=4)  | 高频交易/趋势跟踪          |

### 2.2 核心差异说明

### 2.3 通达信 SMA 特性
- **递归加权**：新数据权重=M/N，历史数据权重=(N-M)/N
- **平滑效果**：比EMA更强调历史数据连续性
- **国内专用**：与国际标准SMA完全不同

## 3. 快速开始

### 3.1 formula

***通达信指标函数实现***

- [通达信指标公式函数大全](https://www.chanluns.com/tdxfun/)

| 级别 | 函数名           | 功能                                                 | 示例                                | 固定参数 | 序列参数 |
|:---|:--------------|:---------------------------------------------------|:----------------------------------|:-----|:-----|
| 0  | ABS           | 绝对值                                                | ABS(X)                            | [√]  | [√]  |
| 0  | REF           | 引用N周期前的值                                           | REF(CLOSE, 5)                     | [√]  | [√]  |
| 0  | IF            | 逻辑判断                                               | IF(CLOSE>10,1,0)                  | [√]  | [√]  |
| 0  | IFF           | 逻辑判断                                               | IFF(CLOSE>10,1,2)                 | [√]  | [√]  |
| 0  | IFN           | 逻辑判断                                               | IFN(CLOSE>10,1,2)                 | [√]  | [√]  |
| 0  | HHV           | 计算N周期内最高                                           | HHV(HIGH,5)                       | [√]  | [√]  |
| 0  | HHVBARS       | 求N周期内S最高值到当前周期数, 返回序列                              | HHVBARS(HIGH,5)                   | [√]  | [√]  |
| 0  | LLV           | 计算N周期内最低                                           | LLV(HLOW,5)                       | [√]  | [√]  |
| 0  | LLVBARS       | 求N周期内S最低值到当前周期数, 返回序列                              | LLVBARS(HLOW,5)                   | [√]  | [√]  |
| 0  | SQRT          | 计算S的平方根                                            | SQRT(CLOSE)                       | [√]  | [√]  |
| 0  | MAX           | 计算AB最大值                                            | MAX(CLOSE,HIGH)                   | [√]  | [√]  |
| 0  | MIN           | 计算AB最小值                                            | MIN(CLOSE,HIGH)                   | [√]  | [√]  |
| 0  | MA            | 计算N周期的移动平均值, 简称均线                                  | MA(CLOSE,5)                       | [√]  | [√]  |
| 0  | DMA           | S序列的动态移动平均, A作为平滑因子                                | DMA(CLOSE,5)                      | [√]  | [√]  |
| 0  | EMA           | S序列N周期的指数移动平均, α=/(1+com)                          | EMA(CLOSE,5)                      | [√]  | [√]  |
| 0  | SMA           | 计算N周期的简单移动平均值                                      | SMA(CLOSE,5, 1)                   | [√]  | [√]  |
| 0  | WMA           | S序列的N周期的加权移动平均值                                    | WMA(CLOSE,5)                      | [√]  | [√]  |
| 0  | STD           | 计算N周期内的标准差                                         | STD(CLOSE,20)                     | [√]  | [√]  |
| 0  | SUM           | 求总和, 如果N=0则从第一个有效值开始                               | SUM(CLOSE,5)                      | [√]  | [√]  |
| 0  | CONST         | 返回序列S最后的值组成常量序列                                    | CONST(CLOSE)                      | [√]  | [ ]  |
| 0  | AVEDEV        | 平均绝对差,序列与其平均值的绝对差的平均值                              | AVEDEV(CLOSE,5)                   | [√]  | [√]  |
| 0  | SLOPE         | S序列N周期回线性回归斜率                                      | SLOPE(CLOSE,5)                    | [√]  | [√]  |
| 0  | FORCAST       | S序列N周期回线性回归后的预测值                                   | FORCAST(CLOSE,5)                  | [√]  | [√]  |
| 0  | LAST          | 从前A日到前B日一直满足S条件,要求A>B & A>0 & B>=0                 | LAST(CLOSE>REF(CLOSE,1),LOW,HIGH) | [√]  | [√]  |
| 1  | COUNT         | COUNT(CLOSE>O,N),最近N天满足S的天数True的天数                 | COUNT(CLOSE>LOW,5)                | [√]  | [√]  |
| 1  | EVERY         | EVERY(CLOSE>O,5),最近N天是否都是True                      | EVERY(CLOSE>LOW,5)                | [X]  | [X]  |
| 1  | EXIST         | EXIST(CLOSE>O,5),最近N天是否都是True                      | EXIST(CLOSE>LOW,5)                | [X]  | [X]  |
| 1  | FILTER        | FILTER函数，S满足条件后，将其后N周期内的数据置为0                      | FILTER(CLOSE>LOW,5)               | [√]  | [√]  |
| 1  | BARSLAST      | 上一次条件成立到当前的周期数                                     | BARSLAST(X)                       | [√]  | [√]  |
| 1  | BARSLASTCOUNT | 统计连续满足S条件的周期数                                      | BARSLASTCOUNT(X)                  | [√]  | [ ]  |
| 1  | BARSLASTS     | 倒数第N次X条件成立到当前的周期数                                  | BARSLASTS(X, N)                   | [√]  | [X]  |
| 1  | BARSSINCEN    | N周期内第一次S条件成立到现在的周期数                                | BARSSINCEN(S,N)                   | [√]  | [√]  |
| 1  | CROSS         | 判断向上金叉穿越,两个序列互换就是判断向下死叉穿越                          | CROSS(MA(C,5),MA(C,10))           | [√]  | [ ]  |
| 1  | LONGCROSS     | 两条线维持一定周期后交叉,S1在N周期内都小于S2,本周期从S1下方向上穿过S2时返回1,否则返回0 | LONGCROSS(MA(C,5),MA(C,10),5)     | [X]  | [X]  |
| 1  | VALUEWHEN     | 当S条件成立时,取X的当前值,否则取VALUEWHEN的上个成立时的X值               | VALUEWHEN(S,X)                    | [X]  | [X]  |
| 1  | BETWEEN       | S处于A和B之间时为真。 包括 A<S<B 或 A>S>B                      | BETWEEN(S,A,B)                    | [X]  | [X]  |
| 1  | TOPRANGE      | TOPRANGE(HIGH)表示当前最高价是近多少周期内最高价的最大值                | TOPRANGE(HIGH)                    | [X]  | [X]  |
| 1  | LOWRANGE      | LOWRANGE(LOW)表示当前最低价是近多少周期内最低价的最小值                 | LOWRANGE(HIGH)                    | [X]  | [X]  |
