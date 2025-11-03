# Changelog
All notable changes to this project will be documented in this file.

## [Unreleased]

## [0.7.1] - 2025-11-03
### Changed
- rust代码下一个版本号0.7.0
- 修正证券列表, 已支持北交所
- rust代码下一个版本0.7.1
- 屏蔽测试utf-8异常的观测代码, 暂时保留后面可能还会用到
- 优化c++字符集转码
- 调整证券名称的字节数为16, 后续的4个字节不属于证券名称, 视为未知/保留字段
- c++新增minizip库, 从服务器下载行业及zhb.zip, 解压获得tdxzs*.cfg
- 修复go和c++版本的板块数据差异, python补全板块代码的sh前缀
- 调整mimlloc编译辅助实现文件的路径到std
- 删除废弃的代码
- 删除废弃的代码
- go代码网络与c++对齐
- cmake安装脚本增加minizip
- 统一ETF的level1基准价格单位的处理
- rust实现板块每日自动更新

## [0.7.0] - 2025-10-29
### Changed
- 更新go依賴庫engine的版本號到1.12.2
- 取消对javascript库的支持, 改为c++实现的解码
- 优化level1证券列表接口, 支持北交所
- update changelog

## [0.6.160] - 2025-10-27
### Changed
- 修改cmake编译gtest测试用例存在失败的问题
- 修复5minK线前复权金额错误的bug
- 修复增加分钟K线数据存在日期和记录不对齐的bug
- 对齐rust与c++的量前复权
- update changelog

## [0.6.159] - 2025-10-27
### Changed
- 优化部分F10数据的代码
- update changelog

## [0.6.158] - 2025-10-26
### Changed
- 删除废弃的临时文件
- 修订应用程序名
- 优化f10缓存过期机制
- 调整测试代码
- 修复git tag脚本
- 修复c++ Testing模板
- 修复vscode 单元测试执行其它exe的问题
- 修改除权除息文档文件名
- 删除废弃的除权除息代码
- 调整vscode配置
- 剔除废弃的代码
- 修复std::error_code未使用告警
- 调整部分代码
- 调整测试代码
- 调整c++ catch2调试配置
- 删除扩展终端选项
- windows下调整为gcc的cppdbg
- 成交量复权与通达信算法对齐
- update changelog

## [0.6.157] - 2025-10-20
### Changed
- 优化.env的检索方法
- update changelog

## [0.6.156] - 2025-10-20
### Changed
- 新增vscode python配置
- 修改加载,env配置
- 优化加载.env
- update changelog

## [0.6.155] - 2025-10-20
### Changed
- 修复quant1x引入的bug
- update changelog

## [0.6.154] - 2025-10-20
### Changed
- 修订config描述
- update changelog

## [0.6.153] - 2025-10-20
### Changed
- 删除废弃的代码，补全函数注释
- 补充注释
- 文件改名前需要关闭临时tmp文件
- 调整readme文件名，采用功能模块名为前缀，方便维护
- 优化.env主导的工作区逻辑
- update changelog

## [0.6.152] - 2025-10-20
### Changed
- rust下一个版本0.6.15
- 优化csv写入, 修复可能null的bug
- 格式化代码，风格保持一致
- 删除废弃的代码
- 优化c++版本的分笔成交数据的更新流程, 修改csv读写方法，去掉反射的部分，作为adapter更新，不适用带有返回列表的函数
- 修订日志
- update changelog

## [0.6.151] - 2025-10-19
### Changed
- 交易日历增加盘前边界测试
- update changelog

## [0.6.150] - 2025-10-19
### Changed
- rust下一个版本0.6.13
- rust代码注释转中文
- update changelog

## [0.6.149] - 2025-10-19
### Changed
- 转英文注释为中文
- update changelog

## [0.6.148] - 2025-10-19
### Changed
- 将英文注释改成中文
- update changelog

## [0.6.147] - 2025-10-19
### Changed
- 修订网络超时
- 调整测试的证券代码
- 细化kline数据为空时的日志
- update changelog

## [0.6.146] - 2025-10-19
### Changed
- rust版本新增反序列化错判枚举
- rust协议接口统一反序列化失败或异常返回错误枚举
- update changelog

## [0.6.145] - 2025-10-19
### Changed
- 进程结束刷新日志改为info级别
- update changelog

## [0.6.144] - 2025-10-19
### Changed
- 优化加固rust连接池
- update changelog

## [0.6.143] - 2025-10-19
### Changed
- rust版本去掉日志文件前缀quant1x
- c++ level1协议处理模板函数增加quant1x::error
- 删除废弃的日志sink代码
- update changelog

## [0.6.142] - 2025-10-19
### Changed
- rust版本按级别拆分成不同的日志文件
- 删除git子模块
- 格式化代码
- 合并rust版本的协议命令字
- 去掉mod.rs中的函数，将mod作为纯粹的模块聚合功能
- tcp连接池增加重试功能
- 优化协议编解码
- update changelog

## [0.6.141] - 2025-10-18
### Changed
- 转换部分代码中全角符号为半角
- 转换部分全角字符为半角
- 新增按日志级别拆分日志
- 调整按日轮转
- 日志文件惰性生成
- update changelog

## [0.6.140] - 2025-10-12
### Changed
- 删除废弃的代码
- update changelog

## [0.6.139] - 2025-10-11
### Changed
- 删除废弃的测试文本文件
- update changelog

## [0.6.138] - 2025-10-10
### Changed
- 新增tail工具, 可安装
- update changelog

## [0.6.137] - 2025-10-10
### Changed
- 新增隐形价差的计算及单元测试
- update changelog

## [0.6.136] - 2025-10-10
### Changed
- 删除废弃的代码
- 优化tcp连接池，修复endpoint活跃数的bug，release时不应该归还endpoint，减少活跃数
- update changelog

## [0.6.135] - 2025-10-06
### Changed
- 删除废弃的注释
- update changelog

## [0.6.134] - 2025-10-06
### Changed
- 格式化c++代码
- 格式化并修正部分注释
- update changelog

## [0.6.133] - 2025-10-05
### Changed
- rust下一个版本0.6.12
- 前十大流通股东结构字段调整, 关于数量的字段改为int64_t
- 修订注释
- update changelog

## [0.6.132] - 2025-10-05
### Changed
- !1 backtest: implement FIFO round-trip stats, add stats helper and compat…
* backtest: implement FIFO round-trip stats, add stats helper and compat…
* 新增0号演示性策略(C++)
* 调整错误码的命名空间
- 调整backtest代码结构
- update changelog

## [0.6.131] - 2025-10-04
### Changed
- 调整测试yaml配置文件解析功能, msvc编译不通过
- 修复msvc编译yaml解析工具失败的问题, examples引用yaml-cpp方式不对
- update changelog

## [0.6.130] - 2025-10-04
### Changed
- 屏蔽进度条的锁, 进度条本身就是线程安全
- 消除未使用变量的告警
- update changelog

## [0.6.129] - 2025-10-03
### Changed
- c++默认关闭mimalloc
- update changelog

## [0.6.128] - 2025-10-03
### Changed
- 优化yaml配置文件解析方式
- 调整编译选项，gcc暂时还是使用-O2编译
- 消除msvc ULONG类型转换告警的问题
- 修复quant1x依赖项不能获取属性的问题
- 屏蔽重复的C++编译选项
- 新增默认开启mimalloc
- update changelog

## [0.6.127] - 2025-10-03
### Changed
- 调整ringbuffer路径
- update changelog

## [0.6.126] - 2025-10-03
### Changed
- 调整ringbuffer基准测试的引用路径
- 删除github action设置
- 提供基础版本的配置文件，说明各版本的配置文件路径
- 新增json到struct映射的c++模板工具
- 调整encoding测试用例源文件名，json新增自动反射编解码功能
- update changelog

## [0.6.125] - 2025-10-03
### Changed
- 删除废弃的代码
- update changelog

## [0.6.124] - 2025-10-03
### Changed
- 修订依赖库列表布局
- update changelog

## [0.6.123] - 2025-10-03
### Changed
- 修正ringbuffer路径
- 删除废弃的代码日志
- 调整numpy和numba的版本号，匹配numpy2.3.0
- update changelog

## [0.6.122] - 2025-10-02
### Changed
- 修订python类self拼写的问题
- update changelog

## [0.6.121] - 2025-10-02
### Changed
- 修订用于python加载本地缓存数据的QUANT1X_WORK关键字的机制
- update changelog

## [0.6.120] - 2025-10-01
### Changed
- 修复头文件路径
- 补充关键注释
- rust下一个版本0.6.11
- update changelog

## [0.6.119] - 2025-10-01
### Changed
- 将RingBuffer归于runtime
- update changelog

## [0.6.118] - 2025-10-01
### Changed
- 新增mio网络服务端和客户端的demo
- 新增rust cron测试代码
- 新增rust 筹码分析代码
- 新增rust hook测试代码
- 迁移早期的rust的engine代码
- 新增早期rust stock代码
- 新增rust早期的q1x代码
- 补充q1x rust代码
- 新增rust MuZero代码
- 新增rust cnn代码
- 新增chips rust代码
- update changelog

## [0.6.117] - 2025-10-01
### Changed
- 修复头文件扩展名的问题
- 调整测试工具的路径
- 明确k线协议中参数I其实是frequency频率，或者step步长，运行在指定k线类型的前提下按
frequency聚合k线
- runtime：添加基于 tokio 的 AsyncScheduler 与 core 全局绑定；清理 scheduler.rs 中警告
- 新增异步调度器模块
- update changelog

## [0.6.116] - 2025-10-01
### Changed
- 删除废弃的代码
- 修改函数签名, 入参data不符合日期的定义
- 调整capnp的测试缓存文件路径, 这个路径需要按照实际调整，这里仅仅是为了去掉q2x的关键字
- c++原文档合并到项目的readme中, 不在独立的展示c++的文档
- 头文件扩展名修改为h
- 将主应用程序名修改为q1x，q1x可以作为quant1x的缩写, 但是q2x就不合适了，没有2x这样语义
- update changelog

## [0.6.115] - 2025-10-01
### Changed
- 删除废弃的js测试工具
- update changelog

## [0.6.114] - 2025-10-01
### Changed
- 测试工具转移到项目根路径下tools
- 演示类代码转移到exemples
- rust下一个版本号0.6.10
- update changelog

## [0.6.113] - 2025-10-01
### Changed
- 缓存一个版本
- 删除mod备份
- 优化RingBuffer：
1. 改成使用 std::byte storage[]（从 aligned_storage 改为 byte-array），并使用 std::launder，这影响了如何对内存读写与类型别名进行优化与内联（不同编译器处理此类写法的 codegen 有差异）。
2. 添加了异常安全 rollback（在构造失败时回滚 enqueue_pos_/slot.seq），并在函数上标注了 noexcept(...) 条件，这可能让编译器为异常路径插入额外栈/原子操作或限制某些优化（尤其是 GCC 在当前版本上对条件 noexcept + 异常捕获/回滚的交互优化可能更保守）。
3. backoff_spin 更激进/可变，这有利于减少 busy-wait，但会改变时序与测量（不大可能直接造成 GCC 性能降这么多，但会影响热点在运行时的竞争特征）。
- update changelog

## [0.6.112] - 2025-09-30
### Changed
- 修复测试告警的问题
- 调整代码格式
- 删除临时文件
- 删除废弃的测试工具
- 新增ring buffer go版本已经rust实现的“Vyukov bounded MPMC queue”
- update changelog

## [0.6.111] - 2025-09-30
### Changed
- 修复网络数据可能存在包不完整的情况，修复分钟级别K线的开始日期没矫正导致的除权除息以及level1协议参数造成逻辑上的越界问题
- update changelog

## [0.6.110] - 2025-09-30
### Changed
- 修正rust版本要求，上下文一致1.90+
- update changelog

## [0.6.109] - 2025-09-30
### Changed
- rust下一个版本0.6.8
- update changelog

## [0.6.108] - 2025-09-30
### Changed
- 更新项目文档, 增加rust库下载数,
- rust版本确定为1.90+
- update changelog

## [0.6.107] - 2025-09-30
### Changed
- 删除不完善的timestamp文档
- 新增log4rs的yaml格式配置文件
- 新增分钟级别k线, 默认关闭, 打开的方式是quant1x.yaml中配置data.cache.kline, 按照pandas的freq的规则设置key，value为布尔值true
- update changelog

## [0.6.106] - 2025-09-30
### Changed
- rust下一个版本0.6.6
- 内存管理使用mimalloc
- rust代码格式化
- update changelog

## [0.6.105] - 2025-09-29
### Changed
- 更新rust配置项
- 新增rust版本的分笔成交数据
- update changelog

## [0.6.104] - 2025-09-29
### Changed
- 修复只有windows平台才依赖的组件
- update changelog

## [0.6.103] - 2025-09-29
### Changed
- 更新rust版本号到0.6.2
- update changelog

## [0.6.102] - 2025-09-29
### Changed
- 增加quant1x证书
- update changelog

## [0.6.101] - 2025-09-29
### Changed
- 调整文档, 明确目前只支持level1行情数据
- 更新rust版本号为0.6.1
- update changelog

## [0.6.100] - 2025-09-29
### Changed
- rust/c++的首页设置在github,仓库在gitee
- update changelog

## [0.6.99] - 2025-09-29
### Changed
- 调整main.rs路径与c++保持一致
- 调整rust package的包含路径
- update changelog

## [0.6.98] - 2025-09-29
### Changed
- 修复kline数据处理没有遵循”每页最新优先“的bug
- update changelog

## [0.6.97] - 2025-09-28
### Changed
- rust补全证券代码
- update changelog

## [0.6.96] - 2025-09-28
### Changed
- rust工具包quant1x发布0.2.0版本
- update changelog

## [0.6.95] - 2025-09-28
### Changed
- 补充部分rust level1协议
- 去掉行尾多余的分号
- 文件不存在时，时间戳返回0
- http协议时间时UTC，需要转成本地时间
- 删除rust废弃的代码
- 新增rust版本的level1协议的实现
- update changelog

## [0.6.94] - 2025-09-27
### Changed
- 新增rust版本二进制小端的编解码
- update changelog

## [0.6.93] - 2025-09-27
### Changed
- 修复doctest
- chore: prepare crate for publish (narrow include)
- 优化二进制小端编解码器
- 新增rust tcp连接池功能
- update changelog

## [0.6.92] - 2025-09-27
### Changed
- 修复包路径错误
- 新增rust版本的js解码器
- update changelog

## [0.6.91] - 2025-09-25
### Changed
- 调整level1头文件包含写法
- update changelog

## [0.6.90] - 2025-09-24
### Changed
- 新增FP Growth rust版本
- update changelog

## [0.6.89] - 2025-09-24
### Changed
- 修复tag标签展示样式
- update changelog

## [0.6.88] - 2025-09-24
### Changed
- 调整项目文件增加tag的展示
- update changelog

## [0.6.87] - 2025-09-24
### Changed
- 优化二进制文件转c++头文件的工具
- update changelog

## [0.6.86] - 2025-09-24
### Changed
- 调整FP Growth到learn
- update changelog

## [0.6.85] - 2025-09-24
### Changed
- 补全未实现的成员函数及文档
- update changelog

## [0.6.84] - 2025-09-24
### Changed
- 新增vscode cmake编译配置
- 新增FP Growth c++版本
- update changelog

## [0.6.83] - 2025-09-24
### Changed
- 删除废弃的代码，quant1x不收录实验性代码
- update changelog

## [0.6.82] - 2025-09-24
### Changed
- 新增FP Growth的go语言实现版本
- update changelog

## [0.6.81] - 2025-09-24
### Changed
- 新增贝叶斯分类算法
- 新增神经网络
- 新增simd实验性代码
- 新增板块实验性代码
- 新增sar实验性代码
- 新增go加载动态库的实验性代码
- 新增Garch模型实验性代码
- 新增筹码实验性代码
- 新增筹码分布实验性代码
- update changelog

## [0.6.80] - 2025-09-23
### Changed
- 修订tcp连接池的注释及文档
- update changelog

## [0.6.79] - 2025-09-23
### Changed
- 补充AI对tcp连接池实现方案的评估文档
- update changelog

## [0.6.78] - 2025-09-23
### Changed
- utf8 to gbk
- merge 0.6.x
- update changelog

## [0.6.77] - 2025-09-23
### Changed
- 补充部分技术文档
- 修复plantuml语法上的告警
- update changelog

## [0.6.76] - 2025-09-23
### Changed
- 新增factors go package概述
- 调整config c++实现代码的路径
- update changelog

## [0.6.75] - 2025-09-23
### Changed
- 新增检查当前版本是否在pypi中已经存在
- update changelog

## [0.6.74] - 2025-09-23
### Changed
- 优化shell脚本
- update changelog

## [0.6.73] - 2025-09-23
### Changed
- 调整bat发布脚本
- update changelog

## [0.6.72] - 2025-09-23
### Changed
- 修复else不能识别的问题
- update changelog

## [0.6.71] - 2025-09-23
### Changed
- 新增windows ps1的pypi发布脚本
- update changelog

## [0.6.70] - 2025-09-23
### Changed
- quant1x go实现只是提供新扩展的功能，主体功能都在engine中
- 补全go版本应用入口
- update changelog

## [0.6.69] - 2025-09-22
### Changed
- 调整go应用入口main
- update changelog

## [0.6.68] - 2025-09-22
### Changed
- 计算优化归于标准库组件
- update changelog

## [0.6.67] - 2025-09-22
### Changed
- 更新go依赖库engine版本到1.11.16
- update changelog

## [0.6.66] - 2025-09-22
### Changed
- 整理插件模块源文件
- update changelog

## [0.6.65] - 2025-09-22
### Changed
- 调整测试代码
- 修正头文件引用
- update changelog

## [0.6.64] - 2025-09-22
### Changed
- 调整yaml解码功能归于encoding包
- update changelog

## [0.6.63] - 2025-09-21
### Changed
- 调整tdd协议测试源文件名
- 删除python的行情数据实现
- 更新magic_enum版本
- 调度器归于runtime
- update changelog

## [0.6.62] - 2025-09-21
### Changed
- 更新xtensor版本到0.27.1
- update changelog

## [0.6.61] - 2025-09-20
### Changed
- 删除对Javascript支持的内容
- update changelog

## [0.6.60] - 2025-09-20
### Changed
- 修订关于rust开发环境的说明
- update changelog

## [0.6.59] - 2025-09-20
### Changed
- 新增git仓库过滤rust临时文件
- 新增rust项目初始设定
- 优化cpu亲和性，新增numa节点感知功能
- 调整README文档
- 字符串操作新增字符串蛇形与驼峰的转换功能
- 新增numa测试代码
- 新增go版本的timestamp
- update changelog

## [0.6.58] - 2025-09-20
### Changed
- 代码对齐
- update changelog

## [0.6.57] - 2025-09-20
### Changed
- 优化yaml序列化
- update changelog

## [0.6.56] - 2025-09-20
### Changed
- 改q1x命名空间为quant1x
- update changelog

## [0.6.55] - 2025-09-19
### Changed
- 修复日历js原算法的测试数据(√)
- update changelog

## [0.6.54] - 2025-09-19
### Changed
- 修复日历的c++/go的算法(√)
- update changelog

## [0.6.53] - 2025-09-18
### Changed
- 修复msvc编译boost.pfr+yaml-cpp失败的问题
- 恢复xsimd版本13.2.0，最新的代码gcc/clang编译通过, msvc会出现异常
- 修复boost.pfr模板编译的错误告警
- 绘图功能库
- 打开mimalloc库的检测
- 调整catch2用法, 增加catch_discover_tests的调用，但是vs2022并不能自动识别
- update changelog

## [0.6.52] - 2025-09-18
### Changed
- 新增第三方http库cpp-httplib，版本0.26.0
- 新增cpp-httplib测试代码，实验失败
- update changelog

## [0.6.51] - 2025-09-18
### Changed
- 新增go版本实验性质的交易日历解码工具
- 调整代码
- 实验结果, 可能是解码的问题, 只过滤双休日, 对于法定的节假日处理的比较薄弱，应该不是x的函数的问题。
- 统一交易日历编解码的源文件名
- update changelog

## [0.6.50] - 2025-09-17
### Changed
- 修订波峰检测的测试代码
- update changelog

## [0.6.49] - 2025-09-17
### Changed
- 修订过滤规则的测试代码
- update changelog

## [0.6.48] - 2025-09-17
### Changed
- 调整部分测试代码
- update changelog

## [0.6.47] - 2025-09-17
### Changed
- 修行性能优化指南的
- 补充后续可能的优化点的实验性代码
- update changelog

## [0.6.46] - 2025-09-17
### Changed
- 修复clang编译失败的问题
- update changelog

## [0.6.45] - 2025-09-16
### Changed
- 更新依赖库版本
- update changelog

## [0.6.44] - 2025-09-16
### Changed
- 整理level1测试代码
- update changelog

## [0.6.43] - 2025-09-16
### Changed
- 调整level1下的encoding为helpers, 防止与顶层编解码encoding冲突
- update changelog

## [0.6.42] - 2025-09-16
### Changed
- 新增为msvc准备的vcpkg的配置文件
- update changelog

## [0.6.41] - 2025-09-16
### Changed
- 将go版本的learn包迁移的quant1x顶层
- update changelog

## [0.6.40] - 2025-09-16
### Changed
- 更新依赖库版本
- 调整网络部分组件头文件宏前缀
- update changelog

## [0.6.39] - 2025-09-15
### Changed
- 修复全天交易时段分钟数综合大于240的bug
- update changelog

## [0.6.38] - 2025-09-15
### Changed
- 修复尾盘集合竞价判断不准确的bug
- update changelog

## [0.6.37] - 2025-09-15
### Changed
- 新增CPU性能优化文档
- 调整目录结构
- update changelog

## [0.6.36] - 2025-09-15
### Changed
- 新增获取cpu信息的函数
- 各数据下载的并发数可以从配置文件加载
- update changelog

## [0.6.35] - 2025-09-15
### Changed
- 新增获取物理CPU数量的函数
- update changelog

## [0.6.34] - 2025-09-15
### Changed
- 调整分笔成交数据的增量更新
- 更新数据线程数设定为5个
- update changelog

## [0.6.33] - 2025-09-14
### Changed
- 更新依赖库版本
- update changelog

## [0.6.32] - 2025-09-14
### Changed
- 调整交易日历实验性代码, 检测结果只有开始的日期是对的，还是存在问题.
- 新增go版本机器学习-聚类工具
- update changelog

## [0.6.31] - 2025-09-13
### Changed
- 删除go pool功能, 需要重新规划package
- 删除废弃的代码
- update changelog

## [0.6.30] - 2025-09-13
### Changed
- 修正头文件宏与目录名保持一致
- update changelog

## [0.6.29] - 2025-09-13
### Changed
- 删除中文注释
- update changelog

## [0.6.28] - 2025-09-13
### Changed
- 修订k线和分笔成交数据的字段为小写的蛇形格式，兼容go版本的数据结构
- update changelog

## [0.6.27] - 2025-09-13
### Changed
- 新增可用环境变量QUAN1X_WORK来指定配置文件的目录名是quant1x还是新版本的q2x
- update changelog

## [0.6.26] - 2025-09-13
### Changed
- 修订函数注释
- 删除废弃的cista编解码库的测试代码, 弃用cista
- 优化yaml配置自动反序列化
- 进度条indicators使用别名mpb，防止与技术指标冲突
- 原dataframe目录改名pandas
- 修改日线源文件名, 去掉复数s, 与其它类型k线命名保持一致
- 原dataframe目录改名pandas
- 新增分钟级别的K线
- 优化分钟级别K线开关, 默认关闭
- update changelog

## [0.6.25] - 2025-09-12
### Changed
- 更新依赖库boost.pfr版本
- update changelog

## [0.6.24] - 2025-09-12
### Changed
- 修复data和fmt新版本问题
- update changelog

## [0.6.23] - 2025-09-12
### Changed
- 更新date库
- 更新fmt库
- update changelog

## [0.6.22] - 2025-09-09
### Changed
- 调整base内部引用包的路径, 取消相对路径的用法
- update changelog

## [0.6.21] - 2025-09-08
### Changed
- 去掉不必要的设置注释
- update changelog

## [0.6.20] - 2025-09-08
### Changed
- bat文件设置GBK编码和回车换行
- update changelog

## [0.6.19] - 2025-09-08
### Changed
- 修正头文件归类不清晰的问题
- update changelog

## [0.6.18] - 2025-09-08
### Changed
- 更新依赖库boost.pfr版本(d9fde1f2a0ab92e7db204c49c1612536cb296dad)
- update changelog

## [0.6.17] - 2025-09-07
### Changed
- 修复头文件兼容问题
- update changelog

## [0.6.16] - 2025-09-07
### Changed
- 修订msvc的版本号
- update changelog

## [0.6.15] - 2025-09-07
### Changed
- 补全原q2x的测试代码，可能存在兼容问题，先保留
- update changelog

## [0.6.14] - 2025-09-07
### Changed
- Merge branch '0.6.x'
- update changelog

## [0.6.13] - 2025-09-07
### Changed
- 剔除绘图的库
- update changelog

## [0.6.12] - 2025-09-07
### Changed
- 剔除对cista库的依赖
- update changelog

## [0.6.11] - 2025-09-07
### Changed
- 安装头文件排除__pycache__目录
- update changelog

## [0.6.10] - 2025-09-07
### Changed
- 调整测试代码
- update changelog

## [0.6.9] - 2025-09-07
### Changed
- 新增安装应用q2x的配置
- update changelog

## [0.6.8] - 2025-09-06
### Changed
- 修订安装quant1x库的头文件路径
- 修订安装quant1x库的头文件路径
- update changelog

## [0.6.7] - 2025-09-06
### Changed
- 调整静态库后缀
- update changelog

## [0.6.6] - 2025-09-06
### Changed
- 修订setup.py, 删除废弃的rst格式历史信息文档
- update changelog

## [0.6.5] - 2025-09-04
### Changed
- 调整测试代码
- update changelog

## [0.6.4] - 2025-09-04
### Changed
- 实验PyProject.toml配置模式
- 新增机器学习的聚类模块
- 删除废弃的尝试go1.25新特征go-import meta
- 优化k-means++代码
- 调整测试代码
- update changelog

## [0.6.3] - 2025-09-04
### Changed
- 更新c++第三方库版本到1.36.0
- update changelog

## [0.6.2] - 2025-09-04
### Changed
- git仓库忽略cmake构建的临时目录
- cmake变量名前缀, 统一quant1x_
- update changelog

## [0.6.1] - 2025-09-02
### Changed
- 更新c++第三方库xsimd版本到13.2.0(eb3bacbe1012bf79a1ad68f2aec53b3e051bc14d)
- update changelog
- 修订README文件, 开发语言支持python/go/c++。rust视情况而定, 部分实现可以参考q1x-rust。后续q1x为前缀的项目会归档后放弃维护
- 更新c++第三方库版本到1.36.0

## [0.6.0] - 2025-09-02
### Changed
- 新增go模块
- 修订git仓库需要忽略的文件类型或目录
- 新增c++的实现代码
- 新增clangd配置
- 新增c++第三方库argparse
- 新增c++第三方库xtensor
- 新增c++第三方库asio
- 新增c++第三方库backward-cpp
- 新增c++第三方库backward-cpp
- 新增c++第三方库cista
- 新增c++第三方库spdlog
- 新增c++第三方库boost.pfr
- 新增c++第三方库protobuf
- 新增c++第三方库bs::thread-pool
- 新增c++第三方库mio
- 新增c++第三方库csv2
- 新增c++第三方库date,fmt
- 新增c++第三方库indicators
- 新增c++第三方库magic_enum
- 新增c++第三方库croncpp
- 新增c++第三方库nlohmann json
- 新增c++第三方库inja
- 新增c++第三方库robin-map
- 新增二进制协议定义文件
- git仓库忽略.cache目录
- 新增二进制文件转c++代码的python脚本
- 新增c++README.md
- 补全c++实现的其它代码
- 新增go项目模块文件
- update changelog

## [0.5.9] - 2025-08-31
### Changed
- 更新依赖库numpy版本
- update changelog

## [0.5.8] - 2025-08-31
### Changed
- 更新依赖库版本
- update changelog

## [0.5.7] - 2025-08-31
### Changed
- 修改相对路径的问题
- update changelog

## [0.5.6] - 2025-08-31
### Changed
- 调整发布脚本中引用q1x-base为quant1x
- update changelog

## [0.5.5] - 2025-08-31
### Changed
- 新增.gitattributes, 为bat设置单独的回车换行
- update changelog

## [0.5.4] - 2025-08-31
### Changed
- Merge branch '0.5.x' of https://gitee.com/quant1x/quant1x into 0.5.x
- update changelog

## [0.5.3] - 2025-08-31
### Changed
- 发布脚本设置可执行属性
- 删除废弃的代码
- 复制q1x-base代码
- update changelog

## [0.5.2] - 2025-08-21
### Changed
- 优化发布pypi脚本
- update changelog

## [0.5.1] - 2025-08-21
### Changed
- 优化setup.py
- update changelog

## [0.5.0] - 2025-08-21
### Changed
- 调整git忽略条目

## [0.1.13] - 2025-08-21
### Changed
- 从原89k项目迁移到本项目, 旨在研究量化的可能
- update changelog

## [0.1.12] - 2025-08-21
### Changed
- 删除全部的子模块
- update changelog

## [0.1.11] - 2025-08-21
### Changed
- update changelog
- 更新python支持版本到3.12.x, go版本到1.25.x
- Merge branch 'master' of https://gitee.com/quant1x/quant1x
- 更新版本
- update changelog

## [0.1.10] - 2023-09-28
### Changed
- 链接 go版本量化引擎(data)

## [0.1.9] - 2023-09-27
### Changed
- 链接 go版本量化引擎(data)

## [0.1.8] - 2023-09-27
### Changed
- 新增pip换国内源的说明
- 调整README

## [0.1.7] - 2023-09-21
### Changed
- 更新 模块版本

## [0.1.6] - 2023-09-21
### Changed
- 链接 python版本通达信网络数据工具库
- 链接 python版本通达信网络数据工具库
- 链接 python版本通达信网络数据工具库

## [0.1.5] - 2023-09-21
### Changed
- 链接 python版本通达信网络数据工具库

## [0.1.4] - 2023-09-21
### Changed
- 链接 python版本公式指标

## [0.1.3] - 2023-09-21
### Changed
- 调整文档说明中版本号的描述

## [0.1.2] - 2023-09-21
### Changed
- 调整文档说明中版本号的描述

## [0.1.1] - 2023-09-21
### Changed
- 增加README

## [0.1.0] - 2023-09-21
### Changed
- Initial commit
- 链接 量化交易需要的工具库
- 链接 迅投win64工具库
- 链接 go版本的通达信数据网络工具库
- 链接 python win64版本的简易交易客户端


[Unreleased]: https://gitee.com/quant1x/quant1x.git/compare/v0.7.1...HEAD
[0.7.1]: https://gitee.com/quant1x/quant1x.git/compare/v0.7.0...v0.7.1
[0.7.0]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.160...v0.7.0
[0.6.160]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.159...v0.6.160
[0.6.159]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.158...v0.6.159
[0.6.158]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.157...v0.6.158
[0.6.157]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.156...v0.6.157
[0.6.156]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.155...v0.6.156
[0.6.155]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.154...v0.6.155
[0.6.154]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.153...v0.6.154
[0.6.153]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.152...v0.6.153
[0.6.152]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.151...v0.6.152
[0.6.151]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.150...v0.6.151
[0.6.150]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.149...v0.6.150
[0.6.149]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.148...v0.6.149
[0.6.148]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.147...v0.6.148
[0.6.147]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.146...v0.6.147
[0.6.146]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.145...v0.6.146
[0.6.145]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.144...v0.6.145
[0.6.144]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.143...v0.6.144
[0.6.143]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.142...v0.6.143
[0.6.142]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.141...v0.6.142
[0.6.141]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.140...v0.6.141
[0.6.140]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.139...v0.6.140
[0.6.139]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.138...v0.6.139
[0.6.138]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.137...v0.6.138
[0.6.137]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.136...v0.6.137
[0.6.136]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.135...v0.6.136
[0.6.135]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.134...v0.6.135
[0.6.134]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.133...v0.6.134
[0.6.133]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.132...v0.6.133
[0.6.132]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.131...v0.6.132
[0.6.131]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.130...v0.6.131
[0.6.130]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.129...v0.6.130
[0.6.129]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.128...v0.6.129
[0.6.128]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.127...v0.6.128
[0.6.127]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.126...v0.6.127
[0.6.126]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.125...v0.6.126
[0.6.125]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.124...v0.6.125
[0.6.124]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.123...v0.6.124
[0.6.123]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.122...v0.6.123
[0.6.122]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.121...v0.6.122
[0.6.121]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.120...v0.6.121
[0.6.120]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.119...v0.6.120
[0.6.119]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.118...v0.6.119
[0.6.118]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.117...v0.6.118
[0.6.117]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.116...v0.6.117
[0.6.116]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.115...v0.6.116
[0.6.115]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.114...v0.6.115
[0.6.114]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.113...v0.6.114
[0.6.113]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.112...v0.6.113
[0.6.112]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.111...v0.6.112
[0.6.111]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.110...v0.6.111
[0.6.110]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.109...v0.6.110
[0.6.109]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.108...v0.6.109
[0.6.108]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.107...v0.6.108
[0.6.107]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.106...v0.6.107
[0.6.106]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.105...v0.6.106
[0.6.105]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.104...v0.6.105
[0.6.104]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.103...v0.6.104
[0.6.103]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.102...v0.6.103
[0.6.102]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.101...v0.6.102
[0.6.101]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.100...v0.6.101
[0.6.100]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.99...v0.6.100
[0.6.99]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.98...v0.6.99
[0.6.98]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.97...v0.6.98
[0.6.97]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.96...v0.6.97
[0.6.96]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.95...v0.6.96
[0.6.95]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.94...v0.6.95
[0.6.94]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.93...v0.6.94
[0.6.93]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.92...v0.6.93
[0.6.92]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.91...v0.6.92
[0.6.91]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.90...v0.6.91
[0.6.90]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.89...v0.6.90
[0.6.89]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.88...v0.6.89
[0.6.88]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.87...v0.6.88
[0.6.87]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.86...v0.6.87
[0.6.86]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.85...v0.6.86
[0.6.85]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.84...v0.6.85
[0.6.84]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.83...v0.6.84
[0.6.83]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.82...v0.6.83
[0.6.82]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.81...v0.6.82
[0.6.81]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.80...v0.6.81
[0.6.80]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.79...v0.6.80
[0.6.79]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.78...v0.6.79
[0.6.78]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.77...v0.6.78
[0.6.77]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.76...v0.6.77
[0.6.76]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.75...v0.6.76
[0.6.75]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.74...v0.6.75
[0.6.74]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.73...v0.6.74
[0.6.73]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.72...v0.6.73
[0.6.72]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.71...v0.6.72
[0.6.71]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.70...v0.6.71
[0.6.70]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.69...v0.6.70
[0.6.69]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.68...v0.6.69
[0.6.68]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.67...v0.6.68
[0.6.67]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.66...v0.6.67
[0.6.66]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.65...v0.6.66
[0.6.65]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.64...v0.6.65
[0.6.64]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.63...v0.6.64
[0.6.63]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.62...v0.6.63
[0.6.62]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.61...v0.6.62
[0.6.61]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.60...v0.6.61
[0.6.60]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.59...v0.6.60
[0.6.59]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.58...v0.6.59
[0.6.58]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.57...v0.6.58
[0.6.57]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.56...v0.6.57
[0.6.56]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.55...v0.6.56
[0.6.55]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.54...v0.6.55
[0.6.54]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.53...v0.6.54
[0.6.53]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.52...v0.6.53
[0.6.52]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.51...v0.6.52
[0.6.51]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.50...v0.6.51
[0.6.50]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.49...v0.6.50
[0.6.49]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.48...v0.6.49
[0.6.48]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.47...v0.6.48
[0.6.47]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.46...v0.6.47
[0.6.46]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.45...v0.6.46
[0.6.45]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.44...v0.6.45
[0.6.44]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.43...v0.6.44
[0.6.43]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.42...v0.6.43
[0.6.42]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.41...v0.6.42
[0.6.41]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.40...v0.6.41
[0.6.40]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.39...v0.6.40
[0.6.39]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.38...v0.6.39
[0.6.38]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.37...v0.6.38
[0.6.37]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.36...v0.6.37
[0.6.36]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.35...v0.6.36
[0.6.35]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.34...v0.6.35
[0.6.34]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.33...v0.6.34
[0.6.33]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.32...v0.6.33
[0.6.32]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.31...v0.6.32
[0.6.31]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.30...v0.6.31
[0.6.30]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.29...v0.6.30
[0.6.29]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.28...v0.6.29
[0.6.28]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.27...v0.6.28
[0.6.27]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.26...v0.6.27
[0.6.26]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.25...v0.6.26
[0.6.25]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.24...v0.6.25
[0.6.24]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.23...v0.6.24
[0.6.23]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.22...v0.6.23
[0.6.22]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.21...v0.6.22
[0.6.21]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.20...v0.6.21
[0.6.20]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.19...v0.6.20
[0.6.19]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.18...v0.6.19
[0.6.18]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.17...v0.6.18
[0.6.17]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.16...v0.6.17
[0.6.16]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.15...v0.6.16
[0.6.15]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.14...v0.6.15
[0.6.14]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.13...v0.6.14
[0.6.13]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.12...v0.6.13
[0.6.12]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.11...v0.6.12
[0.6.11]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.10...v0.6.11
[0.6.10]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.9...v0.6.10
[0.6.9]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.8...v0.6.9
[0.6.8]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.7...v0.6.8
[0.6.7]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.6...v0.6.7
[0.6.6]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.5...v0.6.6
[0.6.5]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.4...v0.6.5
[0.6.4]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.3...v0.6.4
[0.6.3]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.2...v0.6.3
[0.6.2]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.1...v0.6.2
[0.6.1]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.0...v0.6.1
[0.6.0]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.9...v0.6.0
[0.5.9]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.8...v0.5.9
[0.5.8]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.7...v0.5.8
[0.5.7]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.6...v0.5.7
[0.5.6]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.5...v0.5.6
[0.5.5]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.4...v0.5.5
[0.5.4]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.3...v0.5.4
[0.5.3]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.2...v0.5.3
[0.5.2]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.1...v0.5.2
[0.5.1]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.0...v0.5.1
[0.5.0]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.13...v0.5.0
[0.1.13]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.12...v0.1.13
[0.1.12]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.11...v0.1.12
[0.1.11]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.10...v0.1.11
[0.1.10]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.9...v0.1.10
[0.1.9]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.8...v0.1.9
[0.1.8]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.7...v0.1.8
[0.1.7]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.6...v0.1.7
[0.1.6]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.5...v0.1.6
[0.1.5]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.4...v0.1.5
[0.1.4]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.3...v0.1.4
[0.1.3]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.2...v0.1.3
[0.1.2]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.1...v0.1.2
[0.1.1]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.0...v0.1.1

[0.1.0]: https://gitee.com/quant1x/quant1x.git/releases/tag/v0.1.0
